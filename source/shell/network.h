#include "types.h"
#include "nic.h"

// int send_IP_datagram(struct nic_device *nd, uint8_t *payload, int payload_len, char *str_tarip, uint16_t protocal);

struct nic_device;

int send_arpRequest(struct nic_device *nd, char *ipAddr);

int send_icmpRequest(struct nic_device *nd, char *tarips, uint8_t type, uint8_t code);

void recv_Ethernet_frame(struct nic_device *nd, uint8_t *data, int data_len);

// int create_eth_arp_frame(uint8_t* smac, char* ipAddr, struct ethr_hdr *eth);
void unpack_mac(uchar* mac, char* mac_str);
uint32_t IP2int(char *sIP);
void parse_ip(uint ip, char *ip_str);
uint64_t pack_mac(char *src);
char int_to_hex (uint n);
