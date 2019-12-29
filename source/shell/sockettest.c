#include "types.h"
#include "user.h"

int main(int argc,char** argv) {
  int fd;
  uint32_t dst = (10 << 24) | (0 << 16) | (2 << 8) | (2 << 0);
  if((fd = connect(dst, 2000, 5555)) < 0)
  {
    printf(1, "fail to build socket on port 2000\n");
  }

  char *pos = argv[1];
  int i = 0;
  while(pos[i] != 0)
  {
    i++;
  }
  i++;
  if(write(fd, argv[1], i) < 0){
    printf(1, "socket: sending failed\n");
    exit();
  }
  printf(1, "sent to host: %s\n", argv[1]);

  char recvbuf[256];
  int t=read(fd, recvbuf, 256);
  if(t>0)
  {
    printf(1, "received: %s\n", recvbuf);
  }

  close(fd);
  
  exit();
}
