#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;
  struct proc* curproc = myproc();

  if(argint(0, &n) < 0)
    return -1;
  addr = curproc->sz;
  if (n < 0 && growproc(n) < 0)
    return -1;

  // Avoid heap grows higher than stack.
  if (curproc->sz + n > USERTOP - curproc->stack_size - PGSIZE)
    return -1;

  curproc->sz += n;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_reparent(void)
{
  int pid;
  int parent;

  if(argint(0, &pid) < 0)
    return -1;
  if (argint(0, &parent) < 0)
    return -1;

  reparent(pid,parent);
  return 0;
}

int 
sys_getstate(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;

  return getstate(pid);
}
int
sys_suspend(void)
{
  return suspend();
}
sys_inittaskmgr(void)
{
  return inittaskmgr();
}

int
sys_closetaskmgr(void)
{
  return closetaskmgr();
}

int
sys_getprocinfo(void)
{
  int *pid;
  char (*name)[16];
  int *state;
  uint *sz;

  if(argptr(0, (void*)&pid, 64*sizeof(int)) < 0 ||
     argptr(1, (void*)&name, 64*16*sizeof(char)) < 0 ||
     argptr(2, (void*)&state, 64*sizeof(int)) < 0 ||
     argptr(3, (void*)&sz, 64*sizeof(uint)) < 0)
    return -1;
  return getprocinfo(pid, name, state, sz);
}

int
sys_updscrcont(void)
{
  char *buf;
  int curline;

  if(argptr(0, &buf, 24 * 80) < 0 || argint(1, &curline))
    return -1;
  return updscrcont(buf, curline);
}


int sys_nfpgs(void)
{
    return get_num_free_pages();
}

// return 1 when succeed, return 0 when fail
int
sys_createshm(void)
{
    int sig, bytes, type;
    if(argint(0, &sig) < 0 || argint(1, &bytes) < 0 || argint(2, &type) < 0)
    {
        return 0;
    }
    return createshm(sig, bytes, type);
}

// decrease some sig counts in shmlist
// return 1 when succeed, return 0 when delelte a sig, return -1 when fail.
int
sys_deleteshm(void)
{
    int sig;
    if(argint(0, &sig) < 0)
    {
        return 0;
    }
    return deleteshm(sig);
}

// write data from wstr to shared pages with offset "offset"
// return 0 when succeed and return -1 when failed
int
sys_writeshm(void)
{
    int sig;
    char* str;
    int num;
    int offset;
    if(argint(0, &sig) < 0 || argstr(1, &str) < 0 || argint(2, &num) < 0 || argint(3, &offset) < 0)
    {
        return 0;
    }
    return writeshm(sig, str, num, offset);
}

// return 0 when succeed and return -1 when failed
int
sys_readshm(void)
{
    int sig;
    char* str;
    int num;
    int offset;
    if(argint(0, &sig) < 0 || argstr(1, &str) < 0 || argint(2, &num) < 0 || argint(3 ,&offset) < 0)
    {
        return 0;
    }
    return readshm(sig, str, num, offset);
}

int
sys_getsharedpages(void)
{
  return getsharedpages();
}

// Halt (shutdown) the system by sending a special signal to QEMU.
// Based on: https://github.com/noah-mcaulay/XV6-Operating-System/blob/master/sysproc.c
int
sys_shutdown(void)
{
    outw(0x604, 0x0 | 0x2000);
    return 0;
}

//get current system timestamp
//Added by Zhao Zhehui
int
sys_gettimestamp(void)
{
    rtcdate date;
    datetime(&date);
    uint ret = 946684800; //utc+0 2000/1/1 0:0:0 946684800
    int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
    int year4num = (date.year - 2000) / 4;
    ret += year4num*(365*4+1)*86400;
    int yearfor4 = date.year - year4num * 4 - 2000;
    if(yearfor4>0)
    {
        ret += 366 * 86400;
        yearfor4--;
        ret += yearfor4 * 365 * 86400;
    }
    else
        days[1] = 29;
    int i;
    for(i = 1; i < date.month; i++)
        ret += days[i-1]*86400;
    ret += (date.day - 1) * 86400;
    ret += (date.hour * 3600 + date.minute * 60 + date.second);
    return ret;
}

void
sys_showproc(void)
{
  showproc();
}
