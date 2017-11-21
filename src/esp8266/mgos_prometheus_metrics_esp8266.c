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

  mg_printf(nc, "# HELP wifi_rssi WiFi RSSI\r\n");
  mg_printf(nc, "# TYPE wifi_rssi gauge\r\n");
  mg_printf(nc, "wifi_rssi %d\r\n", rssi);
}
#endif // MGOS_HAVE_WIFI

void metrics_platform(struct mg_connection *nc) {
#if MGOS_HAVE_WIFI
  metrics_wifi(nc);
#endif // MGOS_HAVE_WIFI  

  mg_printf(nc, "# HELP esp8266_chip_info ESP8266 Chip Information\r\n");
  mg_printf(nc, "# TYPE esp8266_chip_info gauge\r\n");
  mg_printf(nc, "esp8266_chip_info{sdk=\"%s\",cpu_freq=%u} 1\r\n", system_get_sdk_version(), system_get_cpu_freq());
}
