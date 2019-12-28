//
// networking protocol support (IP, UDP, ARP, etc.).
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "x86.h"
#include "spinlock.h"
#include "network_transmission.h"
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
  struct nic_device *nd;
  if (get_device("mynet0", &nd) < 0)
  {
    cprintf("ERROR:send_arpRequest:Device not loaded\n");
    return;
  }
  send_IP_datagram(nd, m->head, m->len, dip, IPPROTO_UDP);
  mbuffree(m);
}

// receives a UDP packet
static void
net_rx_udp(struct mbuf *m, uint16_t len, uint32_t source_ip)
{
  struct udp *udphdr;
  uint32_t sip;
  uint16_t sport, dport;


  udphdr = mbufpullhdr(m, *udphdr);
  if (!udphdr)
    goto fail;

  // TODO: validate UDP checksum

  // validate lengths reported in headers
  if (ntohs(udphdr->ulen) <= len)
  {
    len = ntohs(udphdr->ulen);
  }
  else
  {
    goto fail;
  }
  
  len -= sizeof(*udphdr);
  if (len > m->len)
    goto fail;
  // minimum packet size could be larger than the payload
  mbuftrim(m, m->len - len);

  // parse the necessary fields
  // sip = ntohl(iphdr->ip_src);
  sport = ntohs(udphdr->sport);
  dport = ntohs(udphdr->dport);
  sockrecvudp(m, source_ip, dport, sport);
  return;

fail:
  mbuffree(m);
}


void deliver_pkt(char *buf_addr, uint32_t len, uint32_t source_ip)
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
  mbufput(m, len);
  net_rx_udp(m, len, source_ip);
}
