#include "types.h"
#include "util.h"
#include "defs.h"
#include "network.h"
#include "e1000.h"

#define BROADCAST_MAC "FF:FF:FF:FF:FF:FF"
#define PROT_IP 0x0800
#define PROT_ARP 0x0806

#define PROT_ICMP 1

struct ICMPHeader
{
    uint8_t type;
    uint8_t code;
    uint16_t cksum;
    uint16_t flag;
    uint16_t seq;
};

struct EthrHeader
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
    // uint16_t dip; //This should be 4 bytes. But alignment issues are creating a padding b/w arp_dmac and dip if dip is kept 4 bytes.
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
    uint ip_vals[4];

    for (int i = 0; i >= 0; i--)
    {
        ip_vals[i] = ip && v;
        v = v << 8;
    }

    int c = 0;
    for (int i = 0; i < 4; i++)
    {
        uint ip1 = ip_vals[i];

        if (ip1 == 0)
        {
            ip_str[c++] = '0';
            ip_str[c++] = ':';
        }
        else
        {
            //unsigned int n_digits = 0;
            char arr[3];
            int j = 0;

            while (ip1 > 0)
            {
                arr[j++] = (ip1 % 10) + '0';
                ip1 /= 10;
            }

            for (j = j - 1; j >= 0; j--)
            {
                ip_str[c++] = arr[j];
            }

            ip_str[c++] = ':';
        }
    }

    ip_str[c - 1] = '\0';
}

// ethernet packet arrived; parse and get the MAC address
// void parse_arp_reply(struct ethr_hdr eth) {
// 	if (eth.ethr_type != 0x0806) {
// 		cprintf("Not an ARP packet");
// 		return;
// 	}

// 	if (eth.protype != 0x0800) {
// 		cprintf("Not IPV4 protocol\n");
// 		return;
// 	}

// 	if (eth.opcode != 2) {
// 		cprintf("Not an ARP reply\n");
// 		return;
// 	}

// 	char* my_mac = (char*)"FF:FF:FF:FF:FF:FF";
// 	char dst_mac[18];

// 	unpack_mac(eth.arp_dmac, dst_mac);

// 	if (util_strcmp((const char*)my_mac, (const char*)dst_mac)) {
// 		cprintf("Not the intended recipient\n");
// 		return;
// 	}

// 	//parse sip; it should be equal to the one we sent
// 	char* my_ip = (char*)"255.255.255.255";
// 	char dst_ip[16];

// 	parse_ip(eth.dip, dst_ip);

// 	if (util_strcmp((const char*)my_ip, (const char*)dst_ip)) {
// 	    cprintf("Not the intended recipient\n");
// 	    return;
// 	}

// 	char mac[18];
// 	unpack_mac(eth.arp_smac, mac);

// 	cprintf((char*)mac);
// }

int send_LAN_frame(struct nic_device *nd, uint8_t *payload, int payload_len, uint64_t tarsrc, uint16_t protocal)
{
    struct e1000 *the_e1000 = (struct e1000 *)nd->driver;
    uint8_t *buffer = (uint8_t *)kalloc();
    uint8_t posiphdrcks;
    uint8_t posicmphdrcks;
    uint8_t pos = 0;
    //mac header
    uint8_t *srcmac = the_e1000->mac_addr;

    pos = fillbuf(buffer, pos, tarsrc, 6);
    pos += memcpy(buffer + pos, srcmac, 6);
    // pos = fillbuf(buffer, pos, srcmac, 6);
    pos = fillbuf(buffer, pos, protocal, 2);

    pos += memcpy(buffer + pos, payload, payload_len);

    nd->send_packet(nd->driver, buffer, pos);
    kfree(buffer);
    return 0;
}

int send_IP_datagram(struct nic_device *nd, uint8_t *payload, int payload_len, char *str_tarip, uint16_t protocal)
{
    static uint16_t id = 1;

    uint8_t *buffer = (uint8_t *)kalloc();
    uint8_t posiphdrcks;
    uint8_t pos = 0;

    //ip header
    uint16_t vrs = 4;
    uint16_t IHL = 5;
    uint16_t TOS = 0;
    uint16_t TOL = 28;
    uint16_t ID = id++;
    uint16_t flag = 0;
    uint16_t offset = 0;
    uint16_t TTL = 32;
    // uint16_t protocal = 1; //ICMP

    uint8_t *piphdr = &buffer[pos];
    pos = fillbuf(buffer, pos, (vrs << 4) + IHL, 1);
    pos = fillbuf(buffer, pos, TOS, 1);
    pos = fillbuf(buffer, pos, TOL, 2);
    pos = fillbuf(buffer, pos, ID, 2);
    pos = fillbuf(buffer, pos, (flag << 13) + offset, 2);
    pos = fillbuf(buffer, pos, TTL, 1);
    pos = fillbuf(buffer, pos, protocal, 1);

    uint16_t cksum = 0; //calc_checksum((uint16_t*)piphdr,5);

    uint32_t srcip = IP2int("10.0.2.15");

    uint32_t tarip = IP2int(str_tarip);
    posiphdrcks = pos;
    pos = fillbuf(buffer, pos, cksum, 2);
    pos = fillbuf(buffer, pos, srcip, 4);
    pos = fillbuf(buffer, pos, tarip, 4);

    // fill payload
    // pos = fillbuf(buffer, pos, payload, payload_len);
    pos += memcpy(buffer + pos, payload, payload_len);

    // cksum = calc_checksum((uint16_t *)piphdr, 10);
    // fillbuf(buffer, posiphdrcks, cksum, 2);

    // uint16_t macprotocal = 0x0800;

    int res = send_LAN_frame(nd, buffer, pos, 0x52550a000202l, PROT_IP);

    kfree(buffer);

    return res;
}

int send_arpRequest(struct nic_device *nd, char *ipAddr)
{

    struct e1000 *the_e1000 = (struct e1000 *)nd->driver;
    cprintf("Create ARP frame\n");
    struct EthrHeader eth;

    char *sdmac = BROADCAST_MAC;
    uint64_t dmac = pack_mac(sdmac);

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

    return send_LAN_frame(nd, &eth, sizeof(struct EthrHeader), dmac, PROT_ARP);
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

    return send_IP_datagram(nd, &hdr, sizeof(struct ICMPHeader), tarips, PROT_ICMP);
}

void recv_LAN_frame(struct nic_device *nd, uint8_t *data, int data_len)
{

}