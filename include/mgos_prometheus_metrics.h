/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos.h"

/* Metric types, represented as a float (or alternatively a uint32_t)
 *
 * GAUGE  : This is an absolute number.
 * COUNTER: This is a monotonic increasing number.
 */
enum mgos_prometheus_metrics_type_t {
  GAUGE = 0,
  COUNTER = 1,
};

/* Prototype of a function which is to be called on each prometheus pull/push.
 */
typedef void (*mgos_prometheus_metrics_fn_t)(struct mg_connection *nc, void *user_data);

/* Registers a function handler, which will be called each time Prometheus
 * scrapes our HTTP /metrics endpoint. Libraries and application code can
 * register any number of handlers, which will be called one after another.
 */
void mgos_prometheus_metrics_add_handler(mgos_prometheus_metrics_fn_t handler, void *user_data);

/* Output a formatted metric tuple to the network connection. For example:
 *
 * uint32_t my_counter=1234;
 * mgos_prometheus_metrics_printf(nc, COUNTER, "number_of_requests", "My Desciption",
 *                                "%lu", my_counter);
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
