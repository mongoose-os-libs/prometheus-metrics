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

#include "mgos.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Metric types, represented as a float (or alternatively a uint32_t)
 *
 * GAUGE  : This is an absolute number.
 * COUNTER: This is a monotonic increasing number.
 */
enum mgos_prometheus_metrics_type_t {
  GAUGE   = 0,
  COUNTER = 1,
};

/* Output a formatted metric tuple to the network connection. For example:
 *
 * uint32_t my_counter=1234;
 * mgos_prometheus_metrics_printf(nc, COUNTER, "number_of_requests", "My Description",
 *                                "%u", my_counter);
 *
 * will output:
 * # TYPE number_of_requests counter
 * # HELP number_of_requests My Description
 * number_of_requests 1234
 *
 */
void mgos_prometheus_metrics_printf(struct mg_connection *nc,
                                    enum mgos_prometheus_metrics_type_t type,
                                    const char *name, const char *descr,
                                    const char *fmt, ...);

/* Prototype of a function which is to be called on each prometheus pull/push.
 */
typedef void (*mgos_prometheus_metrics_fn_t)(struct mg_connection *nc, void *user_data);

/* Registers a function handler, which will be called each time Prometheus
 * scrapes our HTTP /metrics endpoint. Libraries and application code can
 * register any number of handlers, which will be called one after another.
 * Example:
 *
 * #include "mgos_prometheus_metrics.h"
 * uint32_t my_counter=0;
 *
 * static void prometheus_metrics_fn(struct mg_connection *nc, void *user_data) {
 *   mgos_prometheus_metrics_printf(nc, COUNTER,
 *     "my_counter", "Total things counted",
 *     "%u", my_counter);
 *   (void) user_data;
 * }
 *
 * enum mgos_app_init_result mgos_app_init(void) {
 *   mgos_prometheus_metrics_add_handler(prometheus_metrics_fn, NULL);
 *   return MGOS_APP_INIT_SUCCESS;
 * }
 *
 */
void mgos_prometheus_metrics_add_handler(mgos_prometheus_metrics_fn_t handler, void *user_data);

/* Perform an HTTP POST request against the Prometheus Pushgateway specified in
 * the flag prometheus.pushgateway in mos.yml, using 'job' and 'instance'.
 * The job argument is mandatory, but instance can be passed NULL.
 */
void mgos_prometheus_metrics_push(const char *job, const char *instance);

#ifdef __cplusplus
}
#endif
