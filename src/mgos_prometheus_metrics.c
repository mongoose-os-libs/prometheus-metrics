/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos_prometheus_metrics.h"
#include "mgos_http_server.h"
#include "mgos_config.h"

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

static void metrics_mgos(struct mg_connection *nc) {
  mg_printf(nc, "# HELP mgos_build Build info\r\n");
  mg_printf(nc, "# TYPE mgos_build gauge\r\n");
  mg_printf(nc, "mgos_build{app=\"%s\",id=\"%s\",version=\"%s\"} 1\r\n", MGOS_APP, mgos_sys_ro_vars_get_fw_id(), mgos_sys_ro_vars_get_fw_version());
  mg_printf(nc, "# HELP mgos_platform Platform information\r\n");
  mg_printf(nc, "# TYPE mgos_platform gauge\r\n");
  mg_printf(nc, "mgos_platform{arch=\"%s\",mac=\"%s\"} 1\r\n", mgos_sys_ro_vars_get_arch(), mgos_sys_ro_vars_get_mac_address());
  mg_printf(nc, "# HELP mgos_uptime Uptime in seconds\r\n");
  mg_printf(nc, "# TYPE mgos_uptime counter\r\n");
  mg_printf(nc, "mgos_uptime %lu\r\n", (unsigned long) mgos_uptime());

  mg_printf(nc, "# HELP mgos_heap_size System memory size\r\n");
  mg_printf(nc, "# TYPE mgos_heap_size gauge\r\n");
  mg_printf(nc, "mgos_heap_size %u\r\n", mgos_get_heap_size());
  mg_printf(nc, "# HELP mgos_free_heap_size System free memory\r\n");
  mg_printf(nc, "# TYPE mgos_free_heap_size gauge\r\n");
  mg_printf(nc, "mgos_free_heap_size %u\r\n", mgos_get_free_heap_size());
  mg_printf(nc, "# HELP mgos_min_free_heap_size System minimal watermark of free memory\r\n");
  mg_printf(nc, "# TYPE mgos_min_free_heap_size gauge\r\n");
  mg_printf(nc, "mgos_min_free_heap_size %u\r\n", mgos_get_min_free_heap_size());
/*
  mg_printf(nc, "# HELP mgos_fs_memory_usage filesystem memory usage\r\n");
  mg_printf(nc, "# TYPE mgos_fs_memory_usage gauge\r\n");
  mg_printf(nc, "mgos_fs_memory_usage %u\r\n", mgos_get_fs_memory_usage());
*/
  mg_printf(nc, "# HELP mgos_fs_size filesystem size\r\n");
  mg_printf(nc, "# TYPE mgos_fs_size gauge\r\n");
  mg_printf(nc, "mgos_fs_size %u\r\n", mgos_get_fs_size());
  mg_printf(nc, "# HELP mgos_free_fs_size filesystem free space\r\n");
  mg_printf(nc, "# TYPE mgos_free_fs_size gauge\r\n");
  mg_printf(nc, "mgos_free_fs_size %u\r\n", mgos_get_free_fs_size());
  mg_printf(nc, "# HELP mgos_cpu_freq CPU frequency in Hz\r\n");
  mg_printf(nc, "# TYPE mgos_cpu_freq gauge\r\n");
  mg_printf(nc, "mgos_cpu_freq %u\r\n", mgos_get_cpu_freq());
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
