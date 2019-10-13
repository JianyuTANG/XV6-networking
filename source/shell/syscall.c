#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  // Check if addr is valid.
  if ((addr >= curproc->sz && addr < curproc->tf->esp) ||
      (addr + 4 > curproc->sz && addr < curproc->tf->esp) ||
      addr + 4 > USERTOP)
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
  struct proc *curproc = myproc();

  if ((addr >= curproc->sz && addr < curproc->tf->esp) || (addr > USERTOP))
    return -1;
  *pp = (char *)addr;

  if (addr < curproc->sz)
    ep = (char *)curproc->sz;
  else if (addr >= curproc->tf->esp && addr < USERTOP)
    ep = (char *)USERTOP;
  else
    return -1;

  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  if(argint(n, &i) < 0)
    return -1;
  if ((uint)i < PGSIZE || // Null pointer protection.
      (uint)i > USERTOP ||
      ((uint)i >= curproc->sz && (uint)i < USERTOP - curproc->stack_size) ||
      ((uint)(i + size) > curproc->sz && i + size < USERTOP - curproc->stack_size) ||
      (uint)(i + size) > USERTOP ||
      (((uint)i < curproc->sz) && (uint)(i + size) >= USERTOP - curproc->stack_size))
    return -1;

  *pp = (char *)i;
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
extern int sys_reparent(void);
extern int sys_getstate(void);
extern int sys_suspend(void);
extern int sys_nfpgs(void);
extern int sys_writeshm(void);
extern int sys_readshm(void);
extern int sys_createshm(void);
extern int sys_deleteshm(void);
extern int sys_getsharedpages(void);
extern int sys_showproc(void);

extern int sys_setconsole(void);
extern int sys_clearc(void);
extern int sys_insertc(void);
extern int sys_shutdown(void);
extern int sys_lseek(void);
extern int sys_gettimestamp(void);
extern int sys_getcwd(void);
extern int sys_inittaskmgr(void);
extern int sys_closetaskmgr(void);
extern int sys_getprocinfo(void);
extern int sys_updscrcont(void);
extern int sys_hide(void);
extern int sys_show(void);
extern int sys_gettime(void);
extern int sys_isatty(void);
extern int sys_lseek(void);


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
[SYS_reparent] sys_reparent,
[SYS_getstate] sys_getstate,
[SYS_suspend] sys_suspend,
[SYS_nfpgs]   sys_nfpgs,
[SYS_createshm] sys_createshm,
[SYS_deleteshm] sys_deleteshm,
[SYS_readshm] sys_readshm,
[SYS_writeshm] sys_writeshm,
[SYS_setconsole]  sys_setconsole,
[SYS_clearc]        sys_clearc,
[SYS_insertc]       sys_insertc,
[SYS_shutdown]    sys_shutdown,
[SYS_gettimestamp]   sys_gettimestamp,
[SYS_getcwd]    sys_getcwd,
[SYS_inittaskmgr]   sys_inittaskmgr,
[SYS_closetaskmgr]  sys_closetaskmgr,
[SYS_getprocinfo]   sys_getprocinfo,
[SYS_updscrcont]    sys_updscrcont,
[SYS_hide]    sys_hide,
[SYS_show]    sys_show,
[SYS_gettime] sys_gettime,
[SYS_isatty]  sys_isatty,
[SYS_lseek]   sys_lseek,
[SYS_getsharedpages] sys_getsharedpages,
[SYS_showproc] sys_showproc
};

void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    curproc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}
