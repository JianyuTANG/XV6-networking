//
// packet buffer management
//
#define MBUF_SIZE              2048
#define MBUF_DEFAULT_HEADROOM  128

struct mbuf {
  struct mbuf  *next; // the next mbuf in the chain
  char         *head; // the current start position of the buffer
  unsigned int len;   // the length of the buffer
  char         buf[MBUF_SIZE]; // the backing store
};

char *mbufpull(struct mbuf *m, unsigned int len);
char *mbufpush(struct mbuf *m, unsigned int len);
char *mbufput(struct mbuf *m, unsigned int len);
char *mbuftrim(struct mbuf *m, unsigned int len);

// The above functions manipulate the size and position of the buffer:
//            <- push            <- trim
//             -> pull            -> put
// [-headroom-][------buffer------][-tailroom-]
// |----------------MBUF_SIZE-----------------|
//
// These marcos automatically typecast and determine the size of header structs.
// In most situations you should use these instead of the raw ops above.
#define mbufpullhdr(mbuf, hdr) (typeof(hdr)*)mbufpull(mbuf, sizeof(hdr))
#define mbufpushhdr(mbuf, hdr) (typeof(hdr)*)mbufpush(mbuf, sizeof(hdr))
#define mbufputhdr(mbuf, hdr) (typeof(hdr)*)mbufput(mbuf, sizeof(hdr))
#define mbuftrimhdr(mbuf, hdr) (typeof(hdr)*)mbuftrim(mbuf, sizeof(hdr))

struct mbuf *mbufalloc(unsigned int headroom);
void mbuffree(struct mbuf *m);

struct mbufq {
  struct mbuf *head;  // the first element in the queue
  struct mbuf *tail;  // the last element in the queue
};

void mbufq_pushtail(struct mbufq *q, struct mbuf *m);
struct mbuf *mbufq_pophead(struct mbufq *q);
int mbufq_empty(struct mbufq *q);
void mbufq_init(struct mbufq *q);


//
// endianness support
//

static inline uint16_t bswaps(uint16_t val)
{
  return (((val & 0x00ffU) << 8) |
          ((val & 0xff00U) >> 8));
}

static inline uint32_t bswapl(uint32_t val)
{
  return (((val & 0x000000ffUL) << 24) |
          ((val & 0x0000ff00UL) << 8) |
          ((val & 0x00ff0000UL) >> 8) |
          ((val & 0xff000000UL) >> 24));
}

// Use these macros to convert network bytes to the native byte order.
// Note that Risc-V uses little endian while network order is big endian.
#define ntohs bswaps
#define ntohl bswapl
#define htons bswaps
#define htonl bswapl


//
// useful networking headers
//

#define ETHADDR_LEN 6

// an Ethernet packet header (start of the packet).
struct eth {
  uint8_t  dhost[ETHADDR_LEN];
  uint8_t  shost[ETHADDR_LEN];
  uint16_t type;
} __attribute__((packed));

#define ETHTYPE_IP  0x0800 // Internet protocol
#define ETHTYPE_ARP 0x0806 // Address resolution protocol

// an IP packet header (comes after an Ethernet header).
struct ip {
  uint8_t  ip_vhl; // version << 4 | header length >> 2
  uint8_t  ip_tos; // type of service
  uint16_t ip_len; // total length
  uint16_t ip_id;  // identification
  uint16_t ip_off; // fragment offset field
  uint8_t  ip_ttl; // time to live
  uint8_t  ip_p;   // protocol
  uint16_t ip_sum; // checksum
  uint32_t ip_src, ip_dst;
};

#define IPPROTO_ICMP 1  // Control message protocol
#define IPPROTO_TCP  6  // Transmission control protocol
#define IPPROTO_UDP  17 // User datagram protocol

#define MAKE_IP_ADDR(a, b, c, d)           \
  (((uint32_t)a << 24) | ((uint32_t)b << 16) | \
   ((uint32_t)c << 8) | (uint32_t)d)

// a UDP packet header (comes after an IP header).
struct udp {
  uint16_t sport; // source port
  uint16_t dport; // destination port
  uint16_t ulen;  // length, including udp header, not including IP header
  uint16_t sum;   // checksum
};

// an ARP packet (comes after an Ethernet header).
struct arp {
  uint16_t hrd; // format of hardware address
  uint16_t pro; // format of protocol address
  uint8_t  hln; // length of hardware address
  uint8_t  pln; // length of protocol address
  uint16_t op;  // operation

  char   sha[ETHADDR_LEN]; // sender hardware address
  uint32_t sip;              // sender IP address
  char   tha[ETHADDR_LEN]; // target hardware address
  uint32_t tip;              // target IP address
} __attribute__((packed));

#define ARP_HRD_ETHER 1 // Ethernet

enum {
  ARP_OP_REQUEST = 1, // requests hw addr given protocol addr
  ARP_OP_REPLY = 2,   // replies a hw addr given protocol addr
};

// an DNS packet (comes after an UDP header).
struct dns {
  uint16_t id;  // request ID

  uint8_t rd: 1;  // recursion desired
  uint8_t tc: 1;  // truncated
  uint8_t aa: 1;  // authoritive
  uint8_t opcode: 4; 
  uint8_t qr: 1;  // query/response
  uint8_t rcode: 4; // response code
  uint8_t cd: 1;  // checking disabled
  uint8_t ad: 1;  // authenticated data
  uint8_t z:  1;  
  uint8_t ra: 1;  // recursion available
  
  uint16_t qdcount; // number of question entries
  uint16_t ancount; // number of resource records in answer section
  uint16_t nscount; // number of NS resource records in authority section
  uint16_t arcount; // number of resource records in additional records
} __attribute__((packed));

struct dns_question {
  uint16_t qtype;
  uint16_t qclass;
} __attribute__((packed));
  
#define ARECORD (0x0001)
#define QCLASS  (0x0001)

struct dns_data {
  uint16_t type;
  uint16_t class;
  uint32_t ttl;
  uint16_t len;
} __attribute__((packed));
