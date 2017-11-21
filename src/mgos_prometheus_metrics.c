/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos_prometheus_metrics.h"
#include "mgos_http_server.h"
#include "mgos_config.h"
#ifdef MGOS_HAVE_MQTT
#include "mgos_mqtt.h"
#endif // MGOS_HAVE_MQTT


static mgos_prometheus_metrics_fn_t s_prometheus_metrics_fn = NULL;
static void *s_prometheus_metrics_fn_arg = NULL;

void mgos_prometheus_metrics_set_handler(mgos_prometheus_metrics_fn_t fn, void *fn_arg) {
  s_prometheus_metrics_fn = fn;
  s_prometheus_metrics_fn_arg = fn_arg;
}

#ifdef MGOS_HAVE_MQTT
static void metrics_mqtt(struct mg_connection *nc) {
  mg_printf(nc, "# HELP mgos_mqtt_sent_topics_count MQTT topics sent\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_sent_topics_count counter\r\n");
  mg_printf(nc, "mgos_mqtt_sent_topics_count %u\r\n", mgos_mqtt_get_metrics_mqtt_sent_topics_count());
  mg_printf(nc, "# HELP mgos_mqtt_sent_topics_bytes_total Total bytes sent in MQTT topics\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_sent_topics_bytes_total counter\r\n");
  mg_printf(nc, "mgos_mqtt_sent_topics_bytes_total %u\r\n", mgos_mqtt_get_metrics_mqtt_sent_topics_bytes_total());
  mg_printf(nc, "# HELP mgos_mqtt_received_topics_count MQTT topics sent\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_received_topics_count counter\r\n");
  mg_printf(nc, "mgos_mqtt_received_topics_count %u\r\n", mgos_mqtt_get_metrics_mqtt_received_topics_count());
  mg_printf(nc, "# HELP mgos_mqtt_received_topics_bytes_total Total bytes received in MQTT topics\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_received_topics_bytes_total counter\r\n");
  mg_printf(nc, "mgos_mqtt_received_topics_bytes_total %u\r\n", mgos_mqtt_get_metrics_mqtt_received_topics_bytes_total());
}
#endif // MGOS_HAVE_MQTT

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

#ifdef MGOS_HAVE_MQTT
  metrics_mqtt(nc);
#endif // MGOS_HAVE_MQTT

  if (s_prometheus_metrics_fn != NULL)
    s_prometheus_metrics_fn(nc, s_prometheus_metrics_fn_arg);

  nc->flags |= MG_F_SEND_AND_CLOSE;

  (void) ev_data;
  (void) user_data;
}

bool mgos_prometheus_metrics_init(void) {
  if (mgos_sys_config_get_prometheus_server_enable())
    mgos_register_http_endpoint(mgos_sys_config_get_prometheus_server_uri(), metrics_handle, NULL);

  return true;
}
