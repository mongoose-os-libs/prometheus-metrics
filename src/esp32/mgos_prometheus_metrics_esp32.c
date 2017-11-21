/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos_prometheus_metrics.h"
#include <esp_wifi.h>
#include <esp_system.h>
#include <freertos/task.h>

#if MGOS_HAVE_WIFI
static void metrics_wifi(struct mg_connection *nc) {
  wifi_ap_record_t info;
  int rssi;

  if(0 != esp_wifi_sta_get_ap_info(&info))
    return;
  else
    rssi = info.rssi;

  mg_printf(nc, "# HELP wifi_rssi WiFi RSSI\r\n");
  mg_printf(nc, "# TYPE wifi_rssi gauge\r\n");
  mg_printf(nc, "wifi_rssi %d\r\n", rssi);

  return;
}
#endif // MGOS_HAVE_WIFI

void metrics_platform(struct mg_connection *nc) {
  esp_chip_info_t ci;

  esp_chip_info(&ci);
  mg_printf(nc, "# HELP esp32_chip_info ESP32 Chip Information\r\n");
  mg_printf(nc, "# TYPE esp32_chip_info gauge\r\n");
  mg_printf(nc, "esp32_chip_info{model=%d,cores=%d,revision=%d,features=%x,sdk=\"%s\"} 1\r\n", ci.model, ci.cores, ci.revision, ci.features, system_get_sdk_version());

  mg_printf(nc, "# HELP esp32_num_tasks ESP32 FreeRTOS task count\r\n");
  mg_printf(nc, "# TYPE esp32_num_tasks gauge\r\n");
  mg_printf(nc, "esp32_num_tasks %d\r\n", uxTaskGetNumberOfTasks());

#if MGOS_HAVE_WIFI
  metrics_wifi(nc);
#endif // MGOS_HAVE_WIFI
}
