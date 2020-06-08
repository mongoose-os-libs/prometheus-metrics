/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos_prometheus_metrics.h"
#include "mgos_http_server.h"
#include "mgos_rpc.h"
#ifdef MGOS_HAVE_WIFI
#include "mgos_wifi.h"
#endif
#include "cache.h"
#include "mgos_config.h"
#include "mgos_ro_vars.h"
#include "mgos_time.h"

static struct cache *metrics_cache = NULL;

/* Platform specific extensions, see esp32/src/ for example */
void metrics_platform(struct mg_connection *nc);

struct metrics_handler {
  mgos_prometheus_metrics_fn_t handler;
  void *user_data;
  SLIST_ENTRY(metrics_handler) entries;
};

SLIST_HEAD(metrics_handlers, metrics_handler) s_metrics_handlers;

static void call_metrics_handlers(struct mg_connection *nc) {
  struct metrics_handler *mh;

  SLIST_FOREACH(mh, &s_metrics_handlers, entries) {
    mh->handler(nc, mh->user_data);
  }
}

void mgos_prometheus_metrics_add_handler(mgos_prometheus_metrics_fn_t handler,
                                         void *user_data) {
  struct metrics_handler *mh =
      (struct metrics_handler *) calloc(1, sizeof(*mh));

  mh->handler = handler;
  mh->user_data = user_data;
  SLIST_INSERT_HEAD(&s_metrics_handlers, mh, entries);
}

void mgos_prometheus_metrics_printf(struct mg_connection *nc,
                                    enum mgos_prometheus_metrics_type_t type,
                                    const char *name, const char *descr,
                                    const char *fmt, ...) {
  char chunk[500];
  size_t chunklen = 0;
  va_list ap;

  chunk[0] = '\0';

  /* Prometheus parser will only allow a metric to have one TYPE and HELP line,
   * which means that if applicatinos call mgos_prometheus_metrics_printf()
   * multiple times on the same name, the parser borks. Therefore, we have to
   * keep a cache of the names we've already output. This is implemented in
   * cache.[ch]
   */
  if (!cache_haskey(metrics_cache, name)) {
    snprintf(chunk, sizeof(chunk), "# HELP %s %s\n# TYPE %s %s\n%s%s", name,
             descr, name, type == COUNTER ? "counter" : "gauge", name,
             fmt[0] == '{' ? "" : " ");
    cache_addkey(metrics_cache, name);
  } else {
    snprintf(chunk, sizeof(chunk), "%s%s", name, fmt[0] == '{' ? "" : " ");
  }
  va_start(ap, fmt);
  vsnprintf(chunk + strlen(chunk), sizeof(chunk) - strlen(chunk), fmt, ap);
  va_end(ap);
  strncat(chunk, "\n", sizeof(chunk));
  chunklen = strlen(chunk);
  LOG(LL_DEBUG, ("Chunk '%s' with length %d", chunk, (int) chunklen));
  mg_printf(nc, "%X\r\n%s\r\n", (int) chunklen, chunk);
  return;
}

static void metrics_mgos(struct mg_connection *nc) {
  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_build", "Build info",
                                 "{app=\"%s\",id=\"%s\",version=\"%s\"} 1",
                                 MGOS_APP, mgos_sys_ro_vars_get_fw_id(),
                                 mgos_sys_ro_vars_get_fw_version());

  mgos_prometheus_metrics_printf(
      nc, GAUGE, "mgos_platform", "Platform info", "{arch=\"%s\",mac=\"%s\"} 1",
      mgos_sys_ro_vars_get_arch(), mgos_sys_ro_vars_get_mac_address());

  mgos_prometheus_metrics_printf(nc, COUNTER, "mgos_uptime",
                                 "Uptime in seconds", "%lu",
                                 (unsigned long) mgos_uptime());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_heap_size",
                                 "System memory size", "%u",
                                 mgos_get_heap_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_free_heap_size",
                                 "System free memory", "%u",
                                 mgos_get_free_heap_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_min_free_heap_size",
                                 "System minimal watermark of free memory",
                                 "%u", mgos_get_min_free_heap_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_fs_size", "Filesystem size",
                                 "%u", mgos_get_fs_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_free_fs_size",
                                 "Filesystem free space", "%u",
                                 mgos_get_free_fs_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_cpu_freq",
                                 "CPU Frequency in Hz", "%u",
                                 mgos_get_cpu_freq());

#ifdef MGOS_HAVE_WIFI
  mgos_prometheus_metrics_printf(nc, GAUGE, "wifi_rssi", "WiFi RSSI", "%d",
                                 mgos_wifi_sta_get_rssi());
#endif
}

void mgos_prometheus_metrics_send_chunks(struct mg_connection *nc) {
  // Create a string based cache for metric names
  metrics_cache = cache_create();

  metrics_mgos(nc);
  metrics_platform(nc);
  call_metrics_handlers(nc);
  mg_printf(nc, "0\r\n\r\n");

  cache_destroy(&metrics_cache);
}

static void metrics_handle(struct mg_connection *nc, int ev, void *ev_data,
                           void *user_data) {
  if (ev != MG_EV_HTTP_REQUEST) {
    return;
  }

  mg_printf(nc, "HTTP/1.1 200 OK\r\n");
  mg_printf(nc, "Server: Mongoose/" MG_VERSION "\r\n");
  mg_printf(nc, "Content-Type: text/plain\r\n");
  mg_printf(nc, "Content-Encoding: chunked\r\n");
  mg_printf(nc, "Transfer-Encoding: chunked\r\n");
  mg_printf(nc, "Connection: close\r\n");
  mg_printf(nc, "\r\n");

  mgos_prometheus_metrics_send_chunks(nc);
  nc->flags |= MG_F_SEND_AND_CLOSE;

  (void) ev_data;
  (void) user_data;
}

static void metric_rpc_cb(struct mg_rpc_request_info *ri, void *cb_arg,
                   struct mg_rpc_frame_info *fi, struct mg_str args) {
  
  // populate metric data
  mg_rpc_send_responsef(ri, "");

  (void) cb_arg;
  (void) fi;
}

bool mgos_prometheus_metrics_init(void) {
  if (mgos_sys_config_get_prometheus_server_enable()) {
    mgos_register_http_endpoint(mgos_sys_config_get_prometheus_server_uri(),
                                metrics_handle, NULL);
  }

  if (mgos_sys_config_get_prometheus_rpc_enable()){
    mg_rpc_add_handler(mgos_rpc_get_global(), "Prometheus.Metric", "{}", metric_rpc_cb, NULL);
  }

  return true;
}
