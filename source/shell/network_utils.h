#include "e1000.h"
#include "x86.h"

typedef struct Sock_addr{
    char *ip_addr;
    uint16_t port;
}sock_addr;


uint16_t calc_checksum(uint16_t *buffer, int size);
uint32_t getIP(char *sIP);
static uint8_t fillbuf(uint8_t *buf, uint8_t k, uint64_t num, uint8_t len);

int make_udp_pkt(uint8_t* buffer, uint16_t source_port, uint16_t target_port, char* data, uint16_t len);
int make_ip_pkt(uint8_t* buffer, char* target_ip, char* data, uint16_t len);
int make_ethernet_pkt(uint8_t* buffer, char* data, uint16_t len, uint32_t target_mac, uint64_t source_mac)
