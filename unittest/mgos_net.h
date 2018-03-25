#ifndef __MGOS_NET_H
#define __MGOS_NET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MGOS_NET_IF_WIFI_STA 0
#define MGOS_NET_IF_WIFI_AP 1

enum mgos_net_if_type {
  MGOS_NET_IF_TYPE_WIFI,
  MGOS_NET_IF_TYPE_ETHERNET,
  MGOS_NET_IF_TYPE_PPP,
  /* This is a sentinel in case all networking interface types are disabled. */
  MGOS_NET_IF_MAX,
};

struct mgos_net_ip_info {
  struct sockaddr_in ip;
  struct sockaddr_in netmask;
  struct sockaddr_in gw;
};

bool mgos_net_get_ip_info(enum mgos_net_if_type if_type, int if_instance,
                          struct mgos_net_ip_info *ip_info);


void mgos_net_ip_to_str(const struct sockaddr_in *sin, char *out);


#endif // __MGOS_NET_H
