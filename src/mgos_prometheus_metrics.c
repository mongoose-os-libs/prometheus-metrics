/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos_prometheus_metrics.h"
#include "mgos_http_server.h"
#include "mgos_config.h"

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

void mgos_prometheus_metrics_add_handler(mgos_prometheus_metrics_fn_t handler, void *user_data) {
  struct metrics_handler *mh = (struct metrics_handler *) calloc(1, sizeof(*mh));
  mh->handler = handler;
  mh->user_data = user_data;
  SLIST_INSERT_HEAD(&s_metrics_handlers, mh, entries);
}

void mgos_prometheus_metrics_printf(struct mg_connection *nc, enum mgos_prometheus_metrics_type_t type, const char *name, const char *descr, const char *fmt, ...) {
  va_list ap;
  mg_printf(nc, "# HELP %s %s\n", name, descr);
  switch (type) {
    case(COUNTER):
      mg_printf(nc, "# TYPE %s counter\n", name);
      break;
    default:
      mg_printf(nc, "# TYPE %s gauge\n", name);
  }
  mg_printf(nc, "%s%s", name, fmt[0]=='{' ? "" : " ");
  va_start(ap, fmt);
  mg_vprintf(nc, fmt, ap);
  va_end(ap);
  mg_printf(nc, "\n");
  return;
}

static void metrics_mgos(struct mg_connection *nc) {
  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_build", "Build info", 
    "{app=\"%s\",id=\"%s\",version=\"%s\"} 1", MGOS_APP, mgos_sys_ro_vars_get_fw_id(), mgos_sys_ro_vars_get_fw_version());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_platform", "Platform info", 
    "{arch=\"%s\",mac=\"%s\"} 1", mgos_sys_ro_vars_get_arch(), mgos_sys_ro_vars_get_mac_address());

  mgos_prometheus_metrics_printf(nc, COUNTER, "mgos_uptime", "Uptime in seconds", 
    "%lu", (unsigned long) mgos_uptime());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_heap_size", "System memory size", 
    "%u", mgos_get_heap_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_free_heap_size", "System free memory", 
    "%u", mgos_get_free_heap_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_min_free_heap_size", "System minimal watermark of free memory", 
    "%u", mgos_get_min_free_heap_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_fs_size", "Filesystem size", 
    "%u", mgos_get_fs_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_free_fs_size", "Filesystem free space", 
    "%u", mgos_get_free_fs_size());

  mgos_prometheus_metrics_printf(nc, GAUGE, "mgos_cpu_freq", "CPU Frequency in Hz", 
    "%u", mgos_get_cpu_freq());
}

static void metrics_handle(struct mg_connection *nc, int ev, void *ev_data, void *user_data) {

  if (ev != MG_EV_HTTP_REQUEST)
    return;

  mg_printf(nc, "HTTP/1.0 200 OK\r\n");
  mg_printf(nc, "Server: Mongoose/"MG_VERSION"\r\n");
  mg_printf(nc, "Content-Type: text/plain\r\n");
  mg_printf(nc, "Connection: close\r\n");
  mg_printf(nc, "\r\n");

  metrics_mgos(nc);
  metrics_platform(nc);

  call_metrics_handlers(nc);

  nc->flags |= MG_F_SEND_AND_CLOSE;

  (void) ev_data;
  (void) user_data;
}

bool mgos_prometheus_metrics_init(void) {
  if (mgos_sys_config_get_prometheus_server_enable())
    mgos_register_http_endpoint(mgos_sys_config_get_prometheus_server_uri(), metrics_handle, NULL);

  return true;
}
