/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos_prometheus_metrics.h"
#include "user_interface.h"

#ifdef MGOS_HAVE_WIFI
static void metrics_wifi(struct mg_connection *nc) {
  sint8 rssi;
  rssi = wifi_station_get_rssi();

  mgos_prometheus_metrics_printf(nc, GAUGE, "wifi_rssi", "WiFi RSSI", "%d", rssi);
}
#endif // MGOS_HAVE_WIFI

void metrics_platform(struct mg_connection *nc) {
  mgos_prometheus_metrics_printf(nc, GAUGE, "esp8266_chip_info", "ESP8266 Chip Information",
    "{sdk=\"%s\",cpu_freq=\"%u\"} 1", system_get_sdk_version(), system_get_cpu_freq());

#ifdef MGOS_HAVE_WIFI
  metrics_wifi(nc);
#endif // MGOS_HAVE_WIFI  
}
