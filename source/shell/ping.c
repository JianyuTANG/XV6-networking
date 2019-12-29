#include "types.h"
#include "network_transmission.h"
#include "stat.h"
#include "user.h"
#include "nic.h"
#include "e1000.h"

uint32_t ip2int(char *sIP)
{
    int i = 0;
    uint32_t v1 = 0, v2 = 0, v3 = 0, v4 = 0;
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

int main(int argc, char **argv)
{
    if(ping(argv[1]) < 0){
        printf(1, "ifconfig command failed");
    }
    int fd;
  char obuf[13] = "hello world!";
  uint32_t dst=ip2int(argv[1]);
  uint16_t sport=35545;
  uint16_t dport=12000;
  
  if((fd = connect(dst, sport, dport)) < 0){
    printf(2, "ping: connect() failed\n");
    return -1;
  }
  // 效仿linux下ping，一直发送

//   for(int i = 0; i < attempts; i++) {
    if(write(fd, obuf, sizeof(obuf)) < 0){
      printf(2, "ping: send() failed\n");
    //   exit(1);
    return -1;
    }
//   }

  char ibuf[128];
  int cc = read(fd, ibuf, sizeof(ibuf));
  printf(1,"From %s: %s",argv[1],ibuf);
  if(cc < 0){
    printf(2, "ping: recv() failed\n");
    // exit(1);
    return -1;
  }

  if (strcmp(obuf, ibuf) || cc != sizeof(obuf)){
    printf(2, "ping didn't receive correct payload\n");
    // exit(1);
    return -1;
  }

    close(fd);
    return 0;
}