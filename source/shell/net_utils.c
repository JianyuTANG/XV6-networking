#include "types.h"
#include "network_stack.h"
#include "stat.h"
#include "user.h"
#include "nic.h"
#include "e1000.h"
#include"console.h"

int ping_executor(uint16_t sport, uint16_t dport, int attempts){
  cprintf("into ping executor\n");
//   int fd;
//   char obuf[13] = "hello world!";
//   uint32_t dst;

//   // 10.0.2.2, which qemu remaps to the external host,
//   // i.e. the machine you're running qemu on.
//   dst = (10 << 24) | (0 << 16) | (2 << 8) | (2 << 0);

//   // you can send a UDP packet to any Internet address
//   // by using a different dst.
  
//   if((fd = connect(dst, sport, dport)) < 0){
//     cprintf("ping: connect() failed\n");
//     // exit(1);
//     return -1;
//   }

//   for(int i = 0; i < attempts; i++) {
//     if(write(fd, obuf, sizeof(obuf)) < 0){
//       cprintf("ping: send() failed\n");
//     //   exit(1);
//         return -1;
//     }
//   }

//   char ibuf[128];
//   int cc = read(fd, ibuf, sizeof(ibuf));
//   if(cc < 0){
//     cprintf("ping: recv() failed\n");
//     //exit(1);
//     return -1;
//   }

//   close(fd);
//   if (strcmp(obuf, ibuf) || cc != sizeof(obuf)){
//     cprintf("ping didn't receive correct payload\n");
//     //exit(1);
//     return -1;
//   }
  return 0;
}

// Encode a DNS name
void encode_qname(char *qn, char *host)
{
  char *l = host; 
  
  for(char *c = host; c < host+strlen(host)+1; c++) {
    if(*c == '.') {
      *qn++ = (char) (c-l);
      for(char *d = l; d < c; d++) {
        *qn++ = *d;
      }
      l = c+1; // skip .
    }
  }
  *qn = '\0';
}
