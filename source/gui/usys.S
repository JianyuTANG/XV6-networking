#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)

SYSCALL(paintWindow)
SYSCALL(registWindow)
SYSCALL(sendMessage)
SYSCALL(getMessage)
SYSCALL(setTimer)
SYSCALL(destroyWindow)
SYSCALL(killTimer)
SYSCALL(initStringFigure)
SYSCALL(getStringFigure)
SYSCALL(getTime)
SYSCALL(getDate)
SYSCALL(setCursor)
SYSCALL(flushRect)
SYSCALL(resetWindow)
SYSCALL(getWindowInfo)
SYSCALL(directPaintWindow)

SYSCALL(writeSoundBuf)
SYSCALL(setSampleRate)
SYSCALL(pause)
SYSCALL(wavSectionPlay)
SYSCALL(beginMP3Decode)
SYSCALL(waitForMP3Decode)
SYSCALL(endMP3Decode)
SYSCALL(getCoreBuf)
