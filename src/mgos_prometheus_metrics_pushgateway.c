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
#include <strings.h>

static char *s_job      = NULL;
static char *s_instance = NULL;

void mgos_prometheus_metrics_send_chunks(struct mg_connection *nc);

static char *mgos_prometheus_post_uri() {
  static char s_uri[100];

  if (s_instance) {
    snprintf(s_uri, sizeof(s_uri), "/metrics/job/%s/instance/%s", s_job, s_instance);
  } else{
    snprintf(s_uri, sizeof(s_uri), "/metrics/job/%s", s_job);
  }

  return s_uri;
}

static void mgos_prometheus_post_begin(struct mg_connection *nc) {
  mg_printf(nc, "POST %s HTTP/1.1\r\n", mgos_prometheus_post_uri());
  mg_printf(nc, "Host: %s\r\n", mgos_sys_config_get_prometheus_pushgateway());
  mg_printf(nc, "User-Agent: Mongoose/"MG_VERSION "\r\n");
  mg_printf(nc, "Transfer-Encoding: chunked\r\n");
  mg_printf(nc, "Content-Encoding: chunked\r\n");
  mg_printf(nc, "Content-Type: text/plain\r\n");
  mg_printf(nc, "Connection: close\r\n");
  mg_printf(nc, "\r\n");
}

static void mgos_prometheus_metrics_post_finish(struct mg_connection *nc) {
  struct mbuf *io;

  if (!nc) {
    return;
  }
  io = &nc->recv_mbuf;
  if (!io) {
    LOG(LL_INFO, ("No receive mbuf"));
    return;
  }

  if (0 == strncasecmp(io->buf, "HTTP/1.1 202", 12)) {
    LOG(LL_INFO, ("Prometheus POST to http://%s%s successful", mgos_sys_config_get_prometheus_pushgateway(), mgos_prometheus_post_uri()));
    return;
  }

  LOG(LL_ERROR, ("POST failed, response follows: %.*s", (int)io->len, io->buf));
}

static void mgos_prometheus_metrics_post_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data) {
  switch (ev) {
  case MG_EV_CONNECT: {
    int connect_status = *(int *)ev_data;
    if (connect_status == 0) {
//        LOG(LL_DEBUG, ("connect() success"));
      mgos_prometheus_post_begin(nc);
      mgos_prometheus_metrics_send_chunks(nc);
    } else {
      LOG(LL_ERROR, ("connect() error: %s", strerror(connect_status)));
    }
    break;
  }

  case MG_EV_POLL:
//      LOG(LL_DEBUG, ("poll received"));
    break;

  case MG_EV_RECV:
//      LOG(LL_DEBUG, ("data received"));
    mgos_prometheus_metrics_post_finish(nc);
    nc->flags |= MG_F_CLOSE_IMMEDIATELY;
    break;

  case MG_EV_SEND:
//      LOG(LL_DEBUG, ("data sent"));
    break;

  case MG_EV_CLOSE:
//      LOG(LL_DEBUG, ("connection closed"));
    break;
  }
  (void)user_data;
}

void mgos_prometheus_metrics_push(const char *job, const char *instance) {
  if (!job) {
    LOG(LL_ERROR, ("Need to set 'job' argument in order to POST to pushgateway at %s", mgos_sys_config_get_prometheus_pushgateway()));
  }
  if (s_job) {
    free(s_job);
  }
  s_job = strdup(job);

  if (s_instance) {
    free(s_instance);
  }
  if (instance) {
    s_instance = strdup(instance);
  }
  mg_connect(mgos_get_mgr(), mgos_sys_config_get_prometheus_pushgateway(), mgos_prometheus_metrics_post_handler, NULL);

  (void)instance;
}
