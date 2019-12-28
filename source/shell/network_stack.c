//
// networking protocol support (IP, UDP, ARP, etc.).
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "x86.h"
#include "spinlock.h"
#include "network_stack.h"
#include "defs.h"
#include "nic.h"


static uint32_t local_ip = MAKE_IP_ADDR(10, 0, 2, 15); // qemu's idea of the guest IP
static uint8_t local_mac[ETHADDR_LEN] = { 0x52, 0x54, 0x00, 0x12, 0x34, 0x56 };
static uint8_t broadcast_mac[ETHADDR_LEN] = { 0xFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF };

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

// Strips data from the start of the buffer and returns a pointer to it.
// Returns 0 if less than the full requested length is available.
char *
mbufpull(struct mbuf *m, unsigned int len)
{
  char *tmp = m->head;
  if (m->len < len)
    return 0;
  m->len -= len;
  m->head += len;
  return tmp;
}

// Prepends data to the beginning of the buffer and returns a pointer to it.
char *
mbufpush(struct mbuf *m, unsigned int len)
{
  m->head -= len;
  if (m->head < m->buf)
    panic("mbufpush");
  m->len += len;
  return m->head;
}

// Appends data to the end of the buffer and returns a pointer to it.
char *
mbufput(struct mbuf *m, unsigned int len)
{
  char *tmp = m->head + m->len;
  m->len += len;
  if (m->len > MBUF_SIZE)
    panic("mbufput");
  return tmp;
}

// Strips data from the end of the buffer and returns a pointer to it.
// Returns 0 if less than the full requested length is available.
char *
mbuftrim(struct mbuf *m, unsigned int len)
{
  if (len > m->len)
    return 0;
  m->len -= len;
  return m->head + m->len;
}

// Allocates a packet buffer.
struct mbuf *
mbufalloc(unsigned int headroom)
{
  struct mbuf *m;
 
  if (headroom > MBUF_SIZE)
    return 0;
  m = kalloc();
  if (m == 0)
    return 0;
  m->next = 0;
  m->head = (char *)m->buf + headroom;
  m->len = 0;
  memset(m->buf, 0, sizeof(m->buf));
  return m;
}

// Frees a packet buffer.
void
mbuffree(struct mbuf *m)
{
  kfree(m);
}

// Pushes an mbuf to the end of the queue.
void
mbufq_pushtail(struct mbufq *q, struct mbuf *m)
{
  m->next = 0;
  if (!q->head){
    q->head = q->tail = m;
    return;
  }
  q->tail->next = m;
  q->tail = m;
}

// Pops an mbuf from the start of the queue.
struct mbuf *
mbufq_pophead(struct mbufq *q)
{
  struct mbuf *head = q->head;
  if (!head)
    return 0;
  q->head = head->next;
  return head;
}

// Returns one (nonzero) if the queue is empty.
int
mbufq_empty(struct mbufq *q)
{
  return q->head == 0;
}

// Intializes a queue of mbufs.
void
mbufq_init(struct mbufq *q)
{
  q->head = 0;
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

uint16_t calc_checksum(const char *buffer, int size)
{
  const uint16_t *buf = (const uint16_t *)buffer;
  unsigned long cksum = 0;
  while (size > 1)
  {
    cksum += *buffer++;
    size -= 2;
  }

  if(size == 1)
  {
    cksum += *(const char *)buf;
  }

  cksum = (cksum >> 16) + (cksum & 0xffff);
  cksum += (cksum >> 16);
  return (uint16_t)(~cksum);
}

// sends an ethernet packet
static void
net_tx_eth(struct mbuf *m, uint16_t ethtype)
{
  struct eth *ethhdr;

  ethhdr = mbufpushhdr(m, *ethhdr);
  memmove(ethhdr->shost, local_mac, ETHADDR_LEN);
  // In a real networking stack, dhost would be set to the address discovered
  // through ARP. Because we don't support enough of the ARP protocol, set it
  // to broadcast instead.
  memmove(ethhdr->dhost, broadcast_mac, ETHADDR_LEN);
  ethhdr->type = htons(ethtype);

  struct nic_device *nd;
  if (get_device("mynet0", &nd) < 0)
  {
    cprintf("ERROR:send_arpRequest:Device not loaded\n");
    return -1;
  }
  nd->send_packet(nd->driver, (uint8_t *)m->head, m->len);

  // if (e1000_transmit(m)) {
  //   mbuffree(m);
  // }
}

// sends an IP packet
static void
net_tx_ip(struct mbuf *m, uint8_t proto, uint32_t dip)
{
  static uint16_t id = 1;
  struct ip *iphdr;

  // push the IP header
  iphdr = mbufpushhdr(m, *iphdr);
  memset(iphdr, 0, sizeof(*iphdr));

  uint16_t TOL = htons(m->len);
  uint16_t ID = id++;
  uint16_t TTL = 32;
  // uint16_t protocal = 17; //UDP protocol值是17 见 计算机网络自顶向下方法
  uint16_t cksum = in_cksum((unsigned char *)iphdr, sizeof(*iphdr));
  // src ip 获取方式可改善
  uint32_t srcip = getIP("10.0.2.15");

  iphdr->ip_vhl = (4 << 4) | (20 >> 2);
  iphdr->ip_p = proto;
  iphdr->ip_id = ID;
  iphdr->ip_src = htonl(local_ip);
  iphdr->ip_dst = htonl(dip);
  iphdr->ip_len = TOL;
  iphdr->ip_ttl = TTL;
  iphdr->ip_sum = cksum;

  // now on to the ethernet layer
  net_tx_eth(m, ETHTYPE_IP);
}

// sends a UDP packet
void
net_tx_udp(struct mbuf *m, uint32_t dip,
           uint16_t sport, uint16_t dport)
{
  struct udp *udphdr;

  // put the UDP header
  udphdr = mbufpushhdr(m, *udphdr);
  udphdr->sport = htons(sport);
  udphdr->dport = htons(dport);
  udphdr->ulen = htons(m->len);
  udphdr->sum = 0; // zero means no checksum is provided

  // now on to the IP layer
  net_tx_ip(m, IPPROTO_UDP, dip);
}

// receives a UDP packet
static void
net_rx_udp(struct mbuf *m, uint16_t len, struct ip *iphdr)
{
  struct udp *udphdr;
  uint32_t sip;
  uint16_t sport, dport;


  udphdr = mbufpullhdr(m, *udphdr);
  if (!udphdr)
    goto fail;

  // TODO: validate UDP checksum

  // validate lengths reported in headers
  if (ntohs(udphdr->ulen) != len)
    goto fail;
  len -= sizeof(*udphdr);
  if (len > m->len)
    goto fail;
  // minimum packet size could be larger than the payload
  mbuftrim(m, m->len - len);

  // parse the necessary fields
  sip = ntohl(iphdr->ip_src);
  sport = ntohs(udphdr->sport);
  dport = ntohs(udphdr->dport);
  sockrecvudp(m, sip, dport, sport);
  return;

fail:
  mbuffree(m);
}

// receives an IP packet
static void
net_rx_ip(struct mbuf *m)
{
  struct ip *iphdr;
  uint16_t len;

  iphdr = mbufpullhdr(m, *iphdr);
  if (!iphdr)
	  goto fail;

  // check IP version and header len
  if (iphdr->ip_vhl != ((4 << 4) | (20 >> 2)))
    goto fail;
  // validate IP checksum
  if (in_cksum((unsigned char *)iphdr, sizeof(*iphdr)))
    goto fail;
  // can't support fragmented IP packets
  if (htons(iphdr->ip_off) != 0)
    goto fail;
  // is the packet addressed to us?
  if (htonl(iphdr->ip_dst) != local_ip)
    goto fail;
  // can only support UDP
  if (iphdr->ip_p != IPPROTO_UDP)
    goto fail;

  len = ntohs(iphdr->ip_len) - sizeof(*iphdr);
  net_rx_udp(m, len, iphdr);
  return;

fail:
  mbuffree(m);
}


void deliver_pkt(char *buf_addr, uint32_t len)
{
  struct mbuf *m;
  m = mbufalloc(MBUF_DEFAULT_HEADROOM);
  char *startpos = (char *)m->head;
  uint32_t maxlen = MBUF_SIZE - MBUF_DEFAULT_HEADROOM;
  if(maxlen < len)
  {
    len = maxlen;
  }
  for(int i = 0; i < len; i++)
  {
    startpos[i] = buf_addr[i];
  }
  net_rx_ip(m);
}
