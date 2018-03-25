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
