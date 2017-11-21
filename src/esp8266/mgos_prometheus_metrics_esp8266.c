/*
 * Copyright (c) 2017 Pim van Pelt <pim@ipng.nl>
 * All rights reserved
 */

#include "mgos_prometheus_metrics.h"

#ifdef MGOS_HAVE_WIFI
static void metrics_wifi(struct mg_connection *nc) {
  (void) nc;
}
#endif // MGOS_HAVE_WIFI

void metrics_platform(struct mg_connection *nc) {
#if MGOS_HAVE_WIFI
  metrics_wifi(nc);
#endif // MGOS_HAVE_WIFI  
}
