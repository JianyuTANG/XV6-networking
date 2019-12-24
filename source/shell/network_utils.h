#include "e1000.h"
#include "x86.h"


uint16_t calc_checksum(uint16_t *buffer, int size);
uint32_t getIP(char *sIP);
static uint8_t fillbuf(uint8_t *buf, uint8_t k, uint64_t num, uint8_t len);
