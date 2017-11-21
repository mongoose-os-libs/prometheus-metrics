/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos_prometheus_metrics.h"
#include "mgos_http_server.h"
#include "mgos_config.h"
#include <esp_wifi.h>
#include <esp_system.h>
#include <freertos/task.h>
#ifdef MGOS_HAVE_MQTT
#include "mgos_mqtt.h"
#endif // MGOS_HAVE_MQTT

#ifdef MGOS_HAVE_WIFI
static void metrics_wifi(struct mg_connection *nc) {
  wifi_ap_record_t info;

  if(0 != esp_wifi_sta_get_ap_info(&info))
    return;

  mg_printf(nc, "# HELP wifi_rssi WiFi RSSI\r\n");
  mg_printf(nc, "# TYPE wifi_rssi gauge\r\n");
  mg_printf(nc, "wifi_rssi %d\r\n", info.rssi);

  return;
}

#endif // MGOS_HAVE_WIFI

#ifdef MGOS_HAVE_MQTT
static void metrics_mqtt(struct mg_connection *nc) {
  mg_printf(nc, "# HELP mgos_mqtt_sent_topics MQTT topics sent\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_sent_topics counter\r\n");
  mg_printf(nc, "mgos_mqtt_sent_topics %u\r\n", mgos_mqtt_get_metrics_mqtt_sent_topics());
  mg_printf(nc, "# HELP mgos_mqtt_sent_topics_bytes Total bytes sent in MQTT topics\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_sent_topics_bytes counter\r\n");
  mg_printf(nc, "mgos_mqtt_sent_topics_bytes %u\r\n", mgos_mqtt_get_metrics_mqtt_sent_topics_bytes());
  mg_printf(nc, "# HELP mgos_mqtt_received_topics MQTT topics sent\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_received_topics counter\r\n");
  mg_printf(nc, "mgos_mqtt_received_topics %u\r\n", mgos_mqtt_get_metrics_mqtt_received_topics());
  mg_printf(nc, "# HELP mgos_mqtt_received_topics_bytes Total bytes received in MQTT topics\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_received_topics_bytes counter\r\n");
  mg_printf(nc, "mgos_mqtt_received_topics_bytes %u\r\n", mgos_mqtt_get_metrics_mqtt_received_topics_bytes());
}

#endif // MGOS_HAVE_MQTT

static void metrics_esp32(struct mg_connection *nc) {
  esp_chip_info_t ci;

  esp_chip_info(&ci);
  mg_printf(nc, "# HELP esp32_chip_info ESP32 Chip Information\r\n");
  mg_printf(nc, "# TYPE esp32_chip_info gauge\r\n");
  mg_printf(nc, "esp32_chip_info{model=%d,cores=%d,revision=%d,features=%x} 1\r\n", ci.model, ci.cores, ci.revision, ci.features );

  mg_printf(nc, "# HELP esp32_num_tasks ESP32 FreeRTOS task count\r\n");
  mg_printf(nc, "# TYPE esp32_num_tasks gauge\r\n");
  mg_printf(nc, "esp32_num_tasks %d\r\n", uxTaskGetNumberOfTasks());
}

static void metrics_mgos(struct mg_connection *nc) {
  mg_printf(nc, "# HELP mgos_build Build info\r\n");
  mg_printf(nc, "# TYPE mgos_build gauge\r\n");
  mg_printf(nc, "mgos_build{app=\"%s\",id=\"%s\",version=\"%s\"} 1\r\n", MGOS_APP, mgos_sys_ro_vars_get_fw_id(), mgos_sys_ro_vars_get_fw_version());
  mg_printf(nc, "# HELP mgos_platform Platform information\r\n");
  mg_printf(nc, "# TYPE mgos_platform gauge\r\n");
  mg_printf(nc, "mgos_platform{arch=\"%s\",mac=\"%s\",idf=\"%s\"} 1\r\n", mgos_sys_ro_vars_get_arch(), mgos_sys_ro_vars_get_mac_address(), esp_get_idf_version());
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

  metrics_esp32(nc);
  metrics_mgos(nc);

#ifdef MGOS_HAVE_MQTT
  metrics_mqtt(nc);
#endif // MGOS_HAVE_MQTT

#ifdef MGOS_HAVE_WIFI
  metrics_wifi(nc);
#endif // MGOS_HAVE_WIFI

  nc->flags |= MG_F_SEND_AND_CLOSE;

  (void) ev_data;
  (void) user_data;
}

bool mgos_prometheus_metrics_init(void) {
  if (mgos_sys_config_get_prometheus_server_enable())
    mgos_register_http_endpoint(mgos_sys_config_get_prometheus_server_uri(), metrics_handle, NULL);

  return true;
}
