#include "types.h"
#include "nic.h"

int send_ipDatagram(uint8_t *payload, int payload_len, char *str_tarip, uint16_t protocal);

int send_arpRequest(char *ipAddr);

int send_icmpRequest(char *interface, char *tarips, uint8_t type, uint8_t code);

// int create_eth_arp_frame(uint8_t* smac, char* ipAddr, struct ethr_hdr *eth);
void unpack_mac(uchar* mac, char* mac_str);
char int_to_hex (uint n);
