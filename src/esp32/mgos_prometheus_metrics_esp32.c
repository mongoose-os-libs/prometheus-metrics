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

  mgos_prometheus_metrics_printf(nc, GAUGE, "wifi_rssi", "WiFi RSSI", "%d", rssi);
}
#endif // MGOS_HAVE_WIFI

void metrics_platform(struct mg_connection *nc) {
  esp_chip_info_t ci;

  esp_chip_info(&ci);
  mgos_prometheus_metrics_printf(nc, GAUGE, "esp32_chip_info", "ESP32 Chip Information",
    "{model=%d,cores=%d,revision=%d,features=%x,sdk=\"%s\"} 1", ci.model, ci.cores, ci.revision, ci.features, system_get_sdk_version());

  mgos_prometheus_metrics_printf(nc, GAUGE, "esp32_num_tasks", "ESP32 FreeRTOS task count",
    "%d", uxTaskGetNumberOfTasks());

#if MGOS_HAVE_WIFI
  metrics_wifi(nc);
#endif // MGOS_HAVE_WIFI
}
