#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"
#include "PVCObject.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  if(addr >= proc->sz || addr+4 > proc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;

  if(addr >= proc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)proc->sz;
  for(s = *pp; s < ep; s++)
    if(*s == 0)
      return s - *pp;
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint(proc->tf->esp + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size n bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;

  if(argint(n, &i) < 0)
    return -1;
  if((uint)i >= proc->sz || (uint)i+size > proc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);

extern int sys_paintWindow(void);
extern int sys_registWindow(void);
extern int sys_sendMessage(void);
extern int sys_getMessage(void);
extern int sys_setTimer(void);
extern int sys_destroyWindow(void);
extern int sys_killTimer(void);
extern int sys_initStringFigure(void);
extern int sys_getStringFigure(void);
extern int sys_getTime(void);
extern int sys_getDate(void);
extern int sys_setCursor(void);
extern int sys_flushRect(void);
extern int sys_resetWindow(void);
extern int sys_getWindowInfo(void);
extern int sys_directPaintWindow(void);
extern int sys_writeSoundBuf(void);
extern int sys_setSampleRate(void);
extern int sys_pause(void);
extern int sys_wavSectionPlay(void);
extern int sys_waitForMP3Decode(void);
extern int sys_beginMP3Decode(void);
extern int sys_endMP3Decode(void);
extern int sys_getCoreBuf(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,

[SYS_paintWindow]    sys_paintWindow,
[SYS_registWindow] sys_registWindow,
[SYS_sendMessage] sys_sendMessage,
[SYS_getMessage] sys_getMessage,
[SYS_setTimer] sys_setTimer,
[SYS_destroyWindow] sys_destroyWindow,
[SYS_killTimer] sys_killTimer,
[SYS_initStringFigure] sys_initStringFigure,
[SYS_getStringFigure] sys_getStringFigure,
[SYS_getTime] sys_getTime,
[SYS_getDate] sys_getDate,
[SYS_setCursor] sys_setCursor,
[SYS_flushRect] sys_flushRect,
[SYS_resetWindow] sys_resetWindow,
[SYS_getWindowInfo] sys_getWindowInfo,
[SYS_directPaintWindow] sys_directPaintWindow,
[SYS_writeSoundBuf] sys_writeSoundBuf,
[SYS_setSampleRate] sys_setSampleRate,
[SYS_pause] sys_pause,
[SYS_wavSectionPlay] sys_wavSectionPlay,
[SYS_beginMP3Decode] sys_beginMP3Decode,
[SYS_waitForMP3Decode] sys_waitForMP3Decode,
[SYS_endMP3Decode] sys_endMP3Decode,
[SYS_getCoreBuf] sys_getCoreBuf,
};

void
syscall(void)
{
  int num;

  num = proc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    proc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            proc->pid, proc->name, num);
    proc->tf->eax = -1;
  }
}
