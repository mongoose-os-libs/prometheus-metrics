/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos.h"

enum mgos_prometheus_metrics_type_t {
  GAUGE = 0,
  COUNTER = 1,
};

typedef void (*mgos_prometheus_metrics_fn_t)(struct mg_connection *nc, void *user_data);
void mgos_prometheus_metrics_add_handler(mgos_prometheus_metrics_fn_t handler, void *user_data);
void mgos_prometheus_metrics_printf(struct mg_connection *nc, enum mgos_prometheus_metrics_type_t type, const char *name, const char *descr, const char *fmt, ...);

/* Internal prototypes, for use by MGOS libraries */

/* Platform specific extensions, see esp32/src/ for example */
void metrics_platform(struct mg_connection *nc);
