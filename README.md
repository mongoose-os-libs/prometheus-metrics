# prometheus-metrics

A Mongoose OS Prometheus Metrics library.

## Introduction

[Prometheus](https://prometheus.io) is an open-source systems monitoring and
alerting toolkit originally built at SoundCloud. Since its inception in 2012,
many companies and organizations have adopted Prometheus, and the project has
a very active developer and user community. It is now a standalone open source
project and maintained independently of any company.

[Mongoose OS](https://mongoose-os.com) is a purpose-built secure Operating
System for commercial connected devices. It focuses on stable and secure
functioning of multiple connected devices in production and post-sale stages.
Key features include secure communication (TLS), over-the-air updates (OTA)
and remote device management. These features are usually missing from SDK and
their correct implementation would be a complex and resource consuming task.
Neglecting them may result in compromised device security and negative brand
perception of your products.

### Structure

`prometheus-metrics` is a library component that can be added to the app's
`mos.yml` file without any configuration needed out of the box, and it pulls
in the `http-server` module. The library opens a `/metrics` endpoint which
exposes the operating system and library vitalsigns to Prometheus. 

By adding the library to the build manifest in `mos.yml`, a compiler define
`MGOS_HAVE_PROMETHEUS_METRICS` is set, which other libraries can use to
create metrics and update them. This is _non intrusive_ because if the
library is not used, no additional code is compiled in Mongoose OS and its
libraries.

### Implementation

#### Base MGOS Metrics

All Mongoose vitals (memory, WiFi/Ethernet, CPU, scheduling) are exposed
using the `mgos_` prefix.

```
# HELP mgos_build Build info
# TYPE mgos_build gauge
mgos_build{app="empty",id="20171121-164823/???",version="1.1.04"} 1
# HELP mgos_platform Platform information
# TYPE mgos_platform gauge
mgos_platform{arch="esp32",mac="240AC4106560",idf="v1.0-2815-g50a73c1"} 1
# HELP mgos_uptime Uptime in seconds
# TYPE mgos_uptime counter
mgos_uptime 1888
# HELP mgos_heap_size System memory size
# TYPE mgos_heap_size gauge
mgos_heap_size 295076
```

#### Platform Specific Metrics

Platform specific vitals are exposed using the `$platform_` prefix, for
example `esp32_` for ESP32 and ESP-IDF metrics.

```
# HELP esp32_chip_info ESP32 Chip Information
# TYPE esp32_chip_info gauge
esp32_chip_info{model=0,cores=2,revision=1,features=32} 1
# HELP esp32_num_tasks ESP32 FreeRTOS task count
# TYPE esp32_num_tasks gauge
esp32_num_tasks 9
```

#### Library Specific Metrics

Exposing library metrics is a cooperation between the library owner and
this library. 

##### Library implementation

Library owners gate the code that creates, updates and exposes the metrics
by the define `MGOS_HAVE_PROMETHEUS_METRICS`. Metrics should be defined as
static variables and getters provided. Taking `mqtt` as an example:

```
#if MGOS_HAVE_PROMETHEUS_METRICS
static uint32_t metrics_mqtt_sent_topics = 0;
static uint32_t metrics_mqtt_sent_topics_bytes = 0;
static uint32_t metrics_mqtt_received_topics = 0;
static uint32_t metrics_mqtt_received_topics_bytes = 0;

uint32_t mgos_mqtt_get_metrics_mqtt_sent_topics (void) { return metrics_mqtt_sent_topics; }
uint32_t mgos_mqtt_get_metrics_mqtt_sent_topics_bytes (void) { return metrics_mqtt_sent_topics_bytes; }
uint32_t mgos_mqtt_get_metrics_mqtt_received_topics (void) { return metrics_mqtt_received_topics; }
uint32_t mgos_mqtt_get_metrics_mqtt_received_topics_bytes (void) { return metrics_mqtt_received_topics_bytes; }
#endif // MGOS_HAVE_PROMETHEUS_METRICS
```

As mentioned above, if the `prometheus-metrics` library is not included in
the app's `mos.yml` manifest, no code will be compiled which makes the addition
_non intrusive_.

##### Prometheus Metrics implementation

After the library specific metrics are exposed, they can be picked up in
`prometheus-metrics` by writing a static method for the library, prefixed
by `metrics_` taking the `mg_connection` to output to. This code is guarded
by `MGOS_HAVE_*` statements, so that the `prometheus-metrics` library
will compile in only libraries that are used. Staying with the `mqtt`
example:

```
#if MGOS_HAVE_MQTT
#include "mgos_mqtt.h"
#endif // MGOS_HAVE_MQTT

...

#if MGOS_HAVE_MQTT
static void metrics_mqtt(struct mg_connection *nc) {
  mg_printf(nc, "# HELP mgos_mqtt_sent_topics MQTT topics sent\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_sent_topics counter\r\n");
  mg_printf(nc, "mgos_mqtt_sent_topics %u\r\n", mgos_mqtt_get_metrics_mqtt_sent_topics());
  mg_printf(nc, "# HELP mgos_mqtt_sent_topics_bytes Total bytes sent in MQTT topics\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_sent_topics_bytes counter\r\n");
  mg_printf(nc, "mgos_mqtt_sent_topics_bytes %u\r\n", mgos_mqtt_get_metrics_mqtt_sent_topics_bytes());
  mg_printf(nc, "# HELP mgos_mqtt_received_topics MQTT topics sent\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_received_topics counter\r\n");
  mg_printf(nc, "mgos_mqtt_received_topics %u\r\n", mgos_mqtt_get_metrics_mqtt_received_topics());
  mg_printf(nc, "# HELP mgos_mqtt_received_topics_bytes Total bytes received in MQTT topics\r\n");
  mg_printf(nc, "# TYPE mgos_mqtt_received_topics_bytes counter\r\n");
  mg_printf(nc, "mgos_mqtt_received_topics_bytes %u\r\n", mgos_mqtt_get_metrics_mqtt_received_topics_bytes());
}
#endif // MGOS_HAVE_MQTT
```

and calling that from `metrics_handle()`.

This mechanism provides bilateral _non intrusion_:
*   Libraries will not be intruded by `prometheus-metrics` if it is not
    included (based on guards on `MGOS_HAVE_PROMETHEUS_METRICS` statements).
*   `prometheus-metrics` is will only pick up metrics from libraries that
    are used (based on guards on `MGOS_HAVE_*` statements).


#### Application Specific Metrics

Users are able to add their own metrics by installing a handler function.

```
#include "mgos_prometheus_metrics.h"

static void prometheus_metrics_fn(struct mg_connection *nc, void *fn_arg) {
  mg_printf(nc, "# Hello World\r\n");
  (void) fn_arg;
}

enum mgos_app_init_result mgos_app_init(void) {
  mgos_prometheus_metrics_set_handler(prometheus_metrics_fn, NULL);
  return MGOS_APP_INIT_SUCCESS;
}
```
