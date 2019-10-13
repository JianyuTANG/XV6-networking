#include "types.h"
#include "stat.h"
#include "user.h"
#include "sound.h"
#include "fcntl.h"

#define SINGLE_BUF_SIZE 4096
#define BUF_SIZE  (SINGLE_BUF_SIZE * 8)

int
main(int argc, char *argv[])
{
  int i;
  int fd;
  struct wav info;

  fd = open(argv[1], O_RDWR);
  if (fd < 0)
  {
    printf(0, "open wav file fail\n");
    exit();
  }

  read(fd, &info, sizeof(struct wav));
  if ((info.riff_id != 0x46464952)||(info.wave_id != 0x45564157)) {
    printf(0, "invalid file format\n");
    close(fd);
    exit();
  }

  if ((info.info.id != 0x20746d66)||
      (info.info.channel != 0x0002)||
      (info.info.bytes_per_sample != 0x0004)||
      (info.info.bits_per_sample != 0x0010)) {
    printf(0, "data encoded in an unaccepted way\n");
    close(fd);
    exit();
  }

  printf(0, "%d", info.info.sample_rate);
  setSampleRate(info.info.sample_rate);
  uint rd = 0;
  char buf[BUF_SIZE]; 

  int wavpid = fork();
  if (wavpid == 0) {
      exec("wavBufPlay", argv);
  }
  printf(0, "info.dlen%x\n", info.dlen);
  while (rd < info.dlen)
  {
    int len = (info.dlen - rd < BUF_SIZE ? info.dlen - rd : BUF_SIZE);
    i = 0;
    read(fd, buf, BUF_SIZE);
    rd += len;
    while(len > SINGLE_BUF_SIZE)
    {
        writeSoundBuf(buf+(i++)*SINGLE_BUF_SIZE, SINGLE_BUF_SIZE);
        len -= SINGLE_BUF_SIZE;
    }
    if(len > 0)
        writeSoundBuf(buf+i*SINGLE_BUF_SIZE, len);
  }

  memset(buf, 0, BUF_SIZE);
  for (i = 0; i < DMA_BUF_NUM*DMA_BUF_SIZE/BUF_SIZE+1; i++)
  {
    writeSoundBuf(buf, BUF_SIZE);
  }
  close(fd);
  kill(wavpid);
  wait();
  exit();
}

