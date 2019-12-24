#include "e1000.h"
#include "x86.h"
#include "network_utils.h"


int udp_send(char *data, uint16_t len, sock_addr *dest_addr, sock_addr *src_addr);
