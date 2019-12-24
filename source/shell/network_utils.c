#include "network_utils.h"


uint16_t calc_checksum(uint16_t *buffer, int size)
{
  unsigned long cksum = 0;
  while (size > 1)
  {
    cksum += *buffer++;
    --size;
  }
    //  if(size)
    //  {
    //      cksum += *(UCHAR*)buffer;
    //  }
  cksum = (cksum >> 16) + (cksum & 0xffff);
  cksum += (cksum >> 16);
  return (uint16_t)(~cksum);
}

uint32_t getIP(char *sIP)
{
  int i = 0;
  uint32_t v1 = 0, v2 = 0, v3 = 0, v4 = 0;
  cprintf(sIP);
  cprintf("\n");
  cprintf("%d\n", sIP[9]);
  for (i = 0; sIP[i] != '\0'; ++i)
    ;
  for (i = 0; sIP[i] != '.'; ++i)
    v1 = v1 * 10 + sIP[i] - '0';
  for (++i; sIP[i] != '.'; ++i)
    v2 = v2 * 10 + sIP[i] - '0';
  for (++i; sIP[i] != '.'; ++i)
    v3 = v3 * 10 + sIP[i] - '0';
  for (++i; sIP[i] != '\0'; ++i)
    v4 = v4 * 10 + sIP[i] - '0';
  return (v1 << 24) + (v2 << 16) + (v3 << 8) + v4;
}

static uint8_t fillbuf(uint8_t *buf, uint8_t k, uint64_t num, uint8_t len)
{
  static uint8_t mask = -1;
  for (short j = len - 1; j >= 0; --j)
  {
    buf[k++] = (num >> (j << 3)) & mask;
  }
  return k;
}