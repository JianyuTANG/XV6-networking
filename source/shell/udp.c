#include "udp.h"


int udp_send(char *data, uint16_t len, sock_addr *dest_addr)
{
  char *target_ip = dest_addr->ip_addr;
  int target_port = dest_addr->port;
}