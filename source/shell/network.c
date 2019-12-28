#include "types.h"
#include "util.h"
#include "defs.h"
#include "network.h"
#include "e1000.h"

#define BROADCAST_MAC 0xFFFFFFFFFFFF
#define PROT_IP 0x0800
#define PROT_ARP 0x0806
#define PROT_UDP 17
#define PROT_ICMP 1

struct EthernetHeader
{
    uint8_t tarmac[6];
    uint8_t srcmac[6];
    uint16_t protocal;
};

struct IPHeader
{
    uint8_t header[9];
    uint8_t protocal;
    uint16_t cksum;
    uint32_t srcip;
    uint32_t tarip;
};

struct ICMPHeader
{
    uint8_t type;
    uint8_t code;
    uint16_t cksum;
    uint16_t flag;
    uint16_t seq;
};

struct ARPHeader
{
    uint16_t hwtype;
    uint16_t protype;
    uint8_t hwsize;
    uint8_t prosize;
    uint16_t opcode;
    uint8_t data[20];
    // uint8_t arp_smac[6];
    // uint32_t sip;
    // uint8_t arp_dmac[6];
    // uint32_t dip;
};

static uint8_t fillbuf(uint8_t *buf, uint8_t k, uint64_t num, uint8_t len)
{
    static uint8_t mask = -1;
    for (short j = len - 1; j >= 0; --j)
    {
        buf[k++] = (num >> (j << 3)) & mask;
    }
    return k;
}

static uint8_t memcpy(uint8_t *dest, uint8_t *src, uint8_t len)
{
    for (int i = 0; i < len; ++i)
    {
        dest[i] = src[i];
    }
    return len;
}

uint16_t calc_checksum(uint16_t *buffer, int size)
{
    unsigned long cksum = 0;
    while (size > 1)
    {
        cksum += *buffer++;
        --size;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (uint16_t)(~cksum);
}

// This code is lifted from FreeBSD's ping.c, and is copyright by the Regents
// of the University of California.
static unsigned short
in_cksum(const unsigned char *addr, int len)
{
  int nleft = len;
  const unsigned short *w = (const unsigned short *)addr;
  unsigned int sum = 0;
  unsigned short answer = 0;

  /*
   * Our algorithm is simple, using a 32 bit accumulator (sum), we add
   * sequential 16 bit words to it, and at the end, fold back all the
   * carry bits from the top 16 bits into the lower 16 bits.
   */
  while (nleft > 1)  {
    sum += *w++;
    nleft -= 2;
  }

  /* mop up an odd byte, if necessary */
  if (nleft == 1) {
    *(unsigned char *)(&answer) = *(const unsigned char *)w;
    sum += answer;
  }

  /* add back carry outs from top 16 bits to low 16 bits */
  sum = (sum & 0xffff) + (sum >> 16);
  sum += (sum >> 16);
  /* guaranteed now that the lower 16 bits of sum are correct */

  answer = ~sum; /* truncate to 16 bits */
  return answer;
}

uint32_t IP2int(char *sIP)
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

int hex_to_int(char ch)
{

    uint i = 0;

    if (ch >= '0' && ch <= '9')
    {
        i = ch - '0';
    }
    else if (ch >= 'A' && ch <= 'F')
    {
        i = 10 + (ch - 'A');
    }
    else if (ch >= 'a' && ch <= 'f')
    {
        i = 10 + (ch - 'a');
    }

    return i;
}

/**
 * Pack the XX:Xx:XX:XX:XX:XX representation of MAC address
 * into I:I:I:I:I:I
 */
uint64_t pack_mac(char *src)
{
    uint64_t res;
    uint8_t *dest = &res;

    for (int i = 0, j = 0; i < 17; i += 3)
    {
        uint i1 = hex_to_int(src[i]);
        uint i2 = hex_to_int(src[i + 1]);
        dest[j++] = (i1 << 4) + i2;
    }
    return res;
}

uint32_t get_ip(char *ip, uint len)
{
    uint ipv4 = 0;
    char arr[4];
    int n1 = 0;

    uint ip_vals[4];
    int n2 = 0;

    for (int i = 0; i < len; i++)
    {
        char ch = ip[i];
        if (ch == '.')
        {
            arr[n1++] = '\0';
            n1 = 0;
            ip_vals[n2++] = util_atoi(arr);
            //cprintf("Check ipval:%d , arr:%s",ip_vals[n2],arr);
        }
        else
        {

            arr[n1++] = ch;
        }
    }

    arr[n1++] = '\0';
    n1 = 0;
    ip_vals[n2++] = util_atoi(arr);
    //cprintf("Final Check ipval:%d , arr:%s",ip_vals[n2],arr);

    //	ipv4 = (ip_vals[0]<<24) + (ip_vals[1]<<16) + (ip_vals[2]<<8) + ip_vals[3];
    ipv4 = (ip_vals[3] << 24) + (ip_vals[2] << 16) + (ip_vals[1] << 8) + ip_vals[0];
    return ipv4;
}
uint16_t htons(uint16_t v)
{
    return (v >> 8) | (v << 8);
}
uint32_t htonl(uint32_t v)
{
    return htons(v >> 16) | (htons((uint16_t)v) << 16);
}

char int_to_hex(uint n)
{

    char ch = '0';

    if (n >= 0 && n <= 9)
    {
        ch = '0' + n;
    }
    else if (n >= 10 && n <= 15)
    {
        ch = 'A' + (n - 10);
    }

    return ch;
}
// parse the mac address
void unpack_mac(uint8_t *mac, char *mac_str)
{

    int c = 0;

    for (int i = 0; i < 6; i++)
    {
        uint m = mac[i];

        uint i2 = m & 0x0f;
        uint i1 = (m & 0xf0) >> 4;

        mac_str[c++] = int_to_hex(i1);
        mac_str[c++] = int_to_hex(i2);

        mac_str[c++] = ':';
    }

    mac_str[c - 1] = '\0';
}

// parse the ip value
void parse_ip(uint ip, char *ip_str)
{

    uint v = 255;
    uint8_t *ip_vals = &ip;

    int c = 0;
    for (int i = 3; i >= 0; i--)
    {
        uint8_t ip1 = ip_vals[i];

        if (ip1 == 0)
        {
            ip_str[c++] = '0';
        }
        else
        {
            if (ip1 >= 100)
            {
                ip_str[c++] = ip1 / 100 + '0';
                ip1 = ip1 % 100;
                ip_str[c++] = ip1 / 10 + '0';
                ip1 = ip1 % 10;
                ip_str[c++] = ip1 + '0';
            }
            else if (ip1 >= 10)
            {
                ip_str[c++] = ip1 / 10 + '0';
                ip1 = ip1 % 10;
                ip_str[c++] = ip1 + '0';
            }
            else
            {
                ip_str[c++] = ip1 + '0';
            }
        }
        ip_str[c++] = '.';
    }

    ip_str[c - 1] = '\0';
}

int send_Ethernet_frame(struct nic_device *nd, uint8_t *payload, int payload_len, uint8_t *tarmac, uint16_t protocal)
{
    struct e1000 *the_e1000 = (struct e1000 *)nd->driver;
    uint8_t *buffer = (uint8_t *)kalloc();
    uint8_t posiphdrcks;
    uint8_t posicmphdrcks;
    uint8_t pos = 0;
    //mac header
    uint8_t *srcmac = the_e1000->mac_addr;

    // pos = fillbuf(buffer, pos, tarmac, 6);
    pos += memcpy(buffer, tarmac, 6);
    pos += memcpy(buffer + pos, srcmac, 6);
    // pos = fillbuf(buffer, pos, srcmac, 6);
    pos = fillbuf(buffer, pos, protocal, 2);

    pos += memcpy(buffer + pos, payload, payload_len);

    nd->send_packet(nd->driver, buffer, pos);
    kfree(buffer);
    return 0;
}

int send_IP_datagram(struct nic_device *nd, uint8_t *payload, int payload_len, uint32_t tarip, uint16_t protocal)
{
    struct e1000 *e1000 = (struct e1000 *)nd->driver;
    static uint16_t id = 1;

    uint8_t *buffer = (uint8_t *)kalloc();
    uint8_t posiphdrcks;
    uint8_t pos = 0;

    //ip header
    uint8_t vrs = 4;
    uint8_t IHL = 5;
    uint8_t TOS = 0;
    uint16_t TOL = payload_len + sizeof(struct IPHeader);
    uint16_t ID = id++;
    uint16_t flag = 0;
    uint16_t offset = 0;
    uint16_t TTL = 32;

    uint8_t *piphdr = &buffer[pos];
    pos = fillbuf(buffer, pos, (vrs << 4) + IHL, 1);
    pos = fillbuf(buffer, pos, TOS, 1);
    pos = fillbuf(buffer, pos, TOL, 2);
    pos = fillbuf(buffer, pos, ID, 2);
    pos = fillbuf(buffer, pos, (flag << 13) + offset, 2);
    pos = fillbuf(buffer, pos, TTL, 1);
    pos = fillbuf(buffer, pos, protocal, 1);

    uint16_t cksum = 0;

    uint32_t srcip = e1000->ip;

    // uint32_t tarip = IP2int(str_tarip);
    posiphdrcks = pos;
    pos = fillbuf(buffer, pos, cksum, 2);
    pos = fillbuf(buffer, pos, srcip, 4);
    pos = fillbuf(buffer, pos, tarip, 4);

    // fill payload
    // pos = fillbuf(buffer, pos, payload, payload_len);
    pos += memcpy(buffer + pos, payload, payload_len);

    cksum = calc_checksum((uint16_t *)piphdr, 10);
    fillbuf(buffer, posiphdrcks, cksum, 2);

    // uint16_t macprotocal = 0x0800;
    // uint64_t dmac;
    // memmove(&dmac, e1000->gateway_mac_addr, 6);

    int res = send_Ethernet_frame(nd, buffer, pos, e1000->gateway_mac_addr, PROT_IP);

    kfree(buffer);

    return res;
}

int send_arpRequest(struct nic_device *nd, char *ipAddr)
{
    struct e1000 *the_e1000 = (struct e1000 *)nd->driver;
    cprintf("Create ARP frame\n");
    struct ARPHeader eth;

    // char *sdmac = BROADCAST_MAC;
    uint64_t dmac = BROADCAST_MAC;

    /** ARP packet filling **/
    eth.hwtype = htons(1);
    eth.protype = htons(0x0800);

    eth.hwsize = 0x06;
    eth.prosize = 0x04;

    //arp request
    eth.opcode = htons(1);

    memmove(eth.data, the_e1000->mac_addr, 6);
    memmove(eth.data + 10, &dmac, 6);

    *(uint32_t *)(eth.data + 6) = htonl(IP2int("10.0.2.15"));
    *(uint32_t *)(eth.data + 16) = htonl(IP2int(ipAddr));

    return send_Ethernet_frame(nd, &eth, sizeof(struct ARPHeader), &dmac, PROT_ARP);
}

int send_icmpRequest(struct nic_device *nd, char *tarips, uint8_t type, uint8_t code)
{
    // static uint16_t id = 1;
    // uint8_t *buffer = (uint8_t *)kalloc();

    struct ICMPHeader hdr;
    fillbuf(&hdr.type, 0, type, sizeof(hdr.type));
    fillbuf(&hdr.code, 0, code, sizeof(hdr.code));
    uint16_t flag = 1108;
    fillbuf(&hdr.flag, 0, flag, sizeof(flag));
    uint16_t cksum = 0;
    fillbuf(&hdr.cksum, 0, cksum, sizeof(cksum));
    uint16_t seq = 921;
    fillbuf(&hdr.seq, 0, seq, sizeof(seq));
    cksum = calc_checksum((uint16_t *)&hdr, 4);
    fillbuf(&hdr.cksum, 0, cksum, sizeof(hdr.cksum));

    uint32_t tarip = IP2int(tarips);

    return send_IP_datagram(nd, &hdr, sizeof(struct ICMPHeader), tarip, PROT_ICMP);
}

void recv_ARP(struct nic_device *nd, struct ARPHeader *data)
{
    if (htons(data->protype) != 0x0800)
    {
        cprintf("Not IPV4 protocol:%x\n", data->protype);
        return;
    }

    struct e1000 *e1000 = (struct e1000 *)nd->driver;

    switch (htons(data->opcode))
    {
    case 1:
        cprintf("ARP Request\n");
        {
            uint32_t tar_ip = htonl(*(uint32_t *)(data->data + 16));
            if (tar_ip != e1000->ip)
            {
                cprintf("not my ip:%x\n", tar_ip);
                return;
            }

            uint8_t *dmac = data->data;
            // memmove(&dmac, request->data + 10, 6);

            if (tar_ip == e1000->gateway_ip)
                memmove(e1000->gateway_mac_addr, &dmac, 6);

            cprintf("Create ARP frame\n");
            struct ARPHeader eth;

            /** ARP packet filling **/
            eth.hwtype = htons(1);
            eth.protype = htons(0x0800);

            eth.hwsize = 0x06;
            eth.prosize = 0x04;

            //arp request
            eth.opcode = htons(2);

            memmove(eth.data, e1000->mac_addr, 6);
            *(uint32_t *)(eth.data + 6) = htonl(e1000->ip);
            memmove(eth.data + 10, data->data, 10);

            send_Ethernet_frame(nd, &eth, sizeof(struct ARPHeader), dmac, PROT_ARP);
        }
        return;

    case 2:
        cprintf("ARP Reply\n");
        {
            uint32_t tar_ip = htonl(*(uint32_t *)(data->data + 16));
            uint8_t *dmac = data->data;
            if (tar_ip == e1000->gateway_ip)
                memmove(e1000->gateway_mac_addr, &dmac, 6);
        }
        return;

    default:
        cprintf("Not an ARP reply: %x\n", data->opcode);
        return;
    }
}

void recv_IP_datagram(uint8_t *data, uint len)
{
    struct IPHeader *header = data;
    uint cksum = htons(header->cksum);
    // header->cksum = 0;
    if (calc_checksum(data, 10) != cksum)
    {
        cprintf("ERROR:recv_IP_datagram: invalid checksum.\n");
        return;
    }

    switch(header->protocal)
    {
        case PROT_UDP:
        cprintf("UDP Reply\n");
        break;

        default:
        cprintf("unsupported IP protocal:%d\n", header->protocal);
    }
}

void recv_Ethernet_frame(struct nic_device *nd, uint8_t *frame, int frame_len)
{
    struct EthernetHeader *header = frame;
    struct e1000 *e1000 = nd->driver;
    uint64_t broadcast = BROADCAST_MAC;
    if (memcmp(header->tarmac, e1000->mac_addr, 6) && memcmp(header->tarmac, &broadcast, 6))
    {
        cprintf("invalid target mac address: %x", *(uint64_t *)(header->tarmac));
        return;
    }

    switch (htons(header->protocal))
    {
    case PROT_IP:
        recv_IP_datagram(frame + sizeof(struct EthernetHeader), frame_len - sizeof(struct EthernetHeader));
        break;

    case PROT_ARP:
        recv_ARP(nd, frame + sizeof(struct EthernetHeader));
        break;

    default:
        cprintf("unsupported protocal:%x\n", header->protocal);
        break;
    }
}