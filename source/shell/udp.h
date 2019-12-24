#include "e1000.h"
#include "x86.h"
#include "network_utils.h"


int make_udp_pkt(uint8_t* buffer, uint16_t source_port, uint16_t target_port, char* data, uint16_t len);
int make_ip_pkt(uint8_t* buffer, char* target_ip, char* data, uint16_t len);
int make_ethernet_pkt(uint8_t* buffer, char* data, uint16_t len);
