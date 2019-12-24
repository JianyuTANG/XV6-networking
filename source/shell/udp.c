#include "udp.h"


int udp_send(char *data, uint16_t len, sock_addr *dest_addr, sock_addr *src_addr)
{
  char *target_ip = dest_addr->ip_addr;
  uint16_t target_port = dest_addr->port;
  uint16_t source_port = src_addr->port;

  char *buffer = (uint8_t*)kalloc();
  if (make_udp_pkt(buffer, source_port, target_port, data, len))
  {
    cprintf("ERROR:fail send_udpRequest\n");
    return -1;
  }
  len += 8;

  data = buffer;
  buffer = (uint8_t*)kalloc();
  if (make_ip_pkt(buffer, target_ip, data, len))
  {
    cprintf("ERROR:fail send_udpRequest\n");
    return -1;
  }
  kfree(data);
  len += 20;

  uint64_t tarmac = 0x52550a000202l;
  uint64_t srcmac = 0x525400123456l;
  data = buffer;
  buffer = (uint8_t*)kalloc();
  if (make_ethernet_pkt(buffer, data, len, tarmac, srcmac))
  {
    cprintf("ERROR:fail send_udpRequest\n");
    return -1;
  }
  kfree(data);
  len += 14;

  struct nic_device *nd;
  if (get_device("mynet0", &nd) < 0)
  {
    cprintf("ERROR:fail send_udpRequest\n");
    return -1;
  }

  nd->send_packet(nd->driver, (uint8_t *)buffer, len);
}
