/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos.h"

typedef void (*mgos_prometheus_metrics_fn_t)(struct mg_connection *nc, void *fn_arg);
void mgos_prometheus_metrics_set_handler(mgos_prometheus_metrics_fn_t fn, void *fn_arg);

/* Platform specific extensions, see esp32/src/ for example
 */
void metrics_platform(struct mg_connection *nc);
