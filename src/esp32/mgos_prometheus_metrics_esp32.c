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

#include <FreeRTOS.h>
#include <esp_system.h>
#include <freertos/task.h>
#include "mgos_prometheus_metrics.h"

// The typo below is correct, IDF SDK returns temperature in Fahrenheit
// This is an undocumented feature -- symbol is defined in
// esp-idf/components/esp32/lib/libphy.a(phy_chip_v7_cal.o)
extern uint8_t temprature_sens_read();

void metrics_platform(struct mg_connection *nc) {
  esp_chip_info_t ci;

  esp_chip_info(&ci);
  mgos_prometheus_metrics_printf(
      nc, GAUGE, "esp32_chip_info", "ESP32 Chip Information",
      "{model=\"%d\",cores=\"%d\",revision=\"%d\",features=\"0x%x\",sdk=\"%s\"}"
      " 1",
      ci.model, ci.cores, ci.revision, ci.features, system_get_sdk_version());

  mgos_prometheus_metrics_printf(nc, GAUGE, "esp32_num_tasks",
                                 "ESP32 FreeRTOS task count", "%d",
                                 uxTaskGetNumberOfTasks());

  mgos_prometheus_metrics_printf(
      nc, GAUGE, "esp32_temperature", "ESP32 Internal Temperature in Celcius",
      "%.1f", ((float) temprature_sens_read() - 32) / 1.8);
}
