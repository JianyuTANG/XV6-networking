#include "types.h"
#include "user.h"

int main(int argc,char** argv) {
  int MAC_SIZE = 18;
//  char* ip = "104.236.20.60";
  // char* ip = "10.0.2.2";
  // char* mac = malloc(MAC_SIZE);
  // if(arp("mynet0", argv[1], mac, MAC_SIZE) < 0) {
  //   printf(1, "ARP for IP:%s Failed.\n", ip);
  // }
  // exit();
  int fd;
  char obuf[13] = "hello world!";
  uint32_t dst;

  // 10.0.2.2, which qemu remaps to the external host,
  // i.e. the machine you're running qemu on.
  dst = (10 << 24) | (0 << 16) | (2 << 8) | (2 << 0);

  // you can send a UDP packet to any Internet address
  // by using a different dst.
  
  if((fd = connect(dst, 2000, 5555)) < 0){
    fprintf(2, "ping: connect() failed\n");
    exit();
  }
  if(write(fd, obuf, sizeof(obuf)) < 0){
    fprintf(2, "ping: send() failed\n");
    exit();
  }
  exit();
}
