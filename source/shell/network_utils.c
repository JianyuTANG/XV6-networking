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

int make_udp_pkt(uint8_t* buffer, uint16_t source_port, uint16_t target_port, char* data, uint16_t len)
{
  // udp header
  uint8_t pos = 0;
  pos = fillbuf(buffer, pos, source_port, 2);
  pos = fillbuf(buffer, pos, target_port, 2);
  pos = fillbuf(buffer, pos, len + 8, 2);
  uint16_t udp_checksum = 0;
  udp_checksum = calc_checksum((uint16_t*)buffer, (int)len);
  pos = fillbuf(buffer, pos, udp_checksum, 2);

  // udp data
  // 受制于内存分配的方式(kalloc)只有4096bytes，对udp包的大小有限制
  // Ethernet header 14 bytes
  // ip header 20 bytes
  // udp header 8 bytes
  // data length <= 4096 - 14 - 20 - 8 = 4054 bytes
  if (len > 4054)
  {
    cprintf("ERROR: data is too long");
    return -1;
  }
  for (uint16_t i = 0; i < len; i++)
  {
    buffer[pos] = data[i];
    pos++;
  }

  return 1;
}

int make_ip_pkt(uint8_t* buffer, char* target_ip, char* data, uint16_t len)
{
  static uint16_t id = 1;
  //ip header
  uint16_t vrs = 4;
  uint16_t IHL = 5;
  uint16_t TOS = 0;
  uint16_t TOL = 20 + len;
  uint16_t ID = id++;
  uint16_t flag = 0;
  uint16_t offset = 0;
  uint16_t TTL = 32;
  uint16_t protocal = 17; //UDP protocol值是17 见 计算机网络自顶向下方法
  uint16_t cksum = calc_checksum((uint16_t*)buffer, (int)len);
  // src ip 获取方式可改善
  uint32_t srcip = getIP("10.0.2.15");
  uint32_t tarip = getIP(target_ip);

  uint8_t pos = 0;
  pos = fillbuf(buffer, pos, (vrs << 4) + IHL, 1);
  pos = fillbuf(buffer, pos, TOS, 1);
  pos = fillbuf(buffer, pos, TOL, 2);
  pos = fillbuf(buffer, pos, ID, 2);
  pos = fillbuf(buffer, pos, (flag << 13) + offset, 2);
  pos = fillbuf(buffer, pos, TTL, 1);
  pos = fillbuf(buffer, pos, protocal, 1);
  pos = fillbuf(buffer, pos, cksum, 2);
  pos = fillbuf(buffer, pos, srcip, 4);
  pos = fillbuf(buffer, pos, tarip, 4);

  for (uint16_t i = 0; i < len; i++)
  {
    buffer[pos] = data[i];
    pos++;
  }

  return 0;
}

int make_ethernet_pkt(uint8_t* buffer, char* data, uint16_t len, uint32_t target_mac, uint64_t source_mac)
{
  // 获取mac地址方式可以改善
  uint64_t tarmac = target_mac;
  uint64_t srcmac = source_mac;
  uint16_t macprotocal = 0x0800; // IP protocol is 0x0800

  uint8_t pos = 0;
  pos = fillbuf(buffer, pos, tarmac, 6);
  pos = fillbuf(buffer, pos, srcmac, 6);
  pos = fillbuf(buffer, pos, macprotocal, 2);

  for (uint16_t i = 0; i < len; i++)
  {
    buffer[pos] = data[i];
    pos++;
  }

  return 0;
}