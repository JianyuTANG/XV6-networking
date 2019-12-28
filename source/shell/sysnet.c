//
// network system calls.
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "x86.h"
#include "spinlock.h"
#include "network_stack.h"
#include "defs.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"


struct sock {
  struct sock *next; // the next socket in the list
  uint32_t raddr;      // the remote IPv4 address
  uint16_t lport;      // the local UDP port number
  uint16_t rport;      // the remote UDP port number
  struct spinlock lock; // protects the rxq
  struct mbufq rxq;  // a queue of packets waiting to be received
};

static struct spinlock lock;
static struct sock *sockets;

void
sockinit(void)
{
  initlock(&lock, "socktbl");
}

int
sockalloc(struct file **f, uint32_t raddr, uint16_t lport, uint16_t rport)
{
  struct sock *si, *pos;

  si = 0;
  *f = 0;
  if ((*f = filealloc()) == 0)
    goto bad;
  if ((si = (struct sock*)kalloc()) == 0)
    goto bad;

  // initialize objects
  si->raddr = raddr;
  si->lport = lport;
  si->rport = rport;
  initlock(&si->lock, "sock");
  mbufq_init(&si->rxq);
  (*f)->type = FD_SOCK;
  (*f)->readable = 1;
  (*f)->writable = 1;
  (*f)->sock = si;

  // add to list of sockets
  acquire(&lock);
  pos = sockets;
  while (pos) {
    if (pos->raddr == raddr &&
        pos->lport == lport &&
	      pos->rport == rport) {
      release(&lock);
      goto bad;
    }
    pos = pos->next;
  }
  si->next = sockets;
  sockets = si;
  release(&lock);
  return 0;

bad:
  if (si)
    kfree((char*)si);
  if (*f)
    fileclose(*f);
  return -1;
}

void socksendudp(struct file *f, int n, char *addr)
{
  struct sock *s = f->sock;
  struct mbuf *m;
  m = mbufalloc(MBUF_DEFAULT_HEADROOM);
  char *startpos = (char *)m->head;
  for(int i = 0; i < n; i++)
  {
    startpos[i] = addr[i];
  }
  mbufput(m, n);
  uint32_t destination_ip = s->raddr;
  uint16_t destination_port = s->rport;
  uint16_t source_port = s->lport;
  net_tx_udp(m, destination_ip, source_port, destination_port);
}

void sockclose(struct file *f)
{
  acquire(&lock);
  struct sock *s = f->sock;
  // delete s from linked list
  struct sock *pos = sockets;
  if(pos == s)
  {
    sockets = s->next;
  }
  else
  {
    while(pos)
    {
      struct sock *temp = pos->next;
      if(temp == s)
      {
        break;
      }
      pos = temp;
    }
    pos->next = s->next;
  }
  release(&lock);
  kfree((char *)s);
  f->sock = 0;
  return 0;
}

int sockread(struct file *f, char *addr, int n)
{
  // do something
  struct sock *s = f->sock;
  struct mbufq rxq = s->rxq;
  struct mbuf *cur = mbufq_pophead(&rxq);
  if(cur)
  {
    int len = cur->len;
    if(len > n)
    {
      len = n;
    }
    char *buf = cur->buf;
    for(int i = 0; i < len; i++)
    {
      addr[i] = buf[i];
    }
    return len;
  }
  return -1;
}

// called by protocol handler layer to deliver UDP packets
void sockrecvudp(struct mbuf *m, uint32_t raddr, uint16_t lport, uint16_t rport)
{
  // Find the socket that handles this mbuf and deliver it, waking
  // any sleeping reader. Free the mbuf if there are no sockets
  // registered to handle it.
  //
  struct sock* pos;
  
  acquire(&lock);
  pos = sockets;
  while (pos) {
    if (pos->raddr == raddr &&
        pos->lport == lport &&
	      pos->rport == rport) 
    {
      // add buffer to the queue
      mbufq_pushtail(&(pos->rxq), m);
      release(&lock);
      return;
    }
    pos = pos->next;
  }
  release(&lock);  
  mbuffree(m);
}
