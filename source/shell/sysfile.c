//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//
#include "e1000.h"

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "x86.h"
#include "memlayout.h"
#include "date.h"

#include "history.h"
#include "var_in_kernel.h"
#include "syscall.h"
#include "network.h"

#define CRTPORT 0x3d4
static ushort *crt = (ushort *)P2V(0xb8000); // CGA memory

//Edit console output
int sys_setconsole(void)
{
  int pos, ch, color, cursor, mode;
  if (argint(0, &pos) < 0 || argint(1, &ch) < 0)
    return -1;
  if (argint(2, &color) < 0)
    color = 0x0700;
  if (argint(3, &cursor) < 0)
    cursor = -1;
  if (argint(4, &mode) < 0)
    mode = 0;
  if (0 <= pos && pos < 80 * 25)
  { //屏幕输出范围在[0~80 x 25)
    crt[pos] = (ch & 0xff) | color;
  }
  if (cursor >= 0)
  {
    outb(CRTPORT, 14);
    outb(CRTPORT + 1, cursor >> 8);
    outb(CRTPORT, 15);
    outb(CRTPORT + 1, cursor);
  }
  if (mode < 0)
    mode = 0;
  consolemode = mode;
  return 0;
}

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if (argint(n, &fd) < 0)
    return -1;
  if (fd < 0 || fd >= NOFILE || (f = myproc()->ofile[fd]) == 0)
    return -1;
  if (pfd)
    *pfd = fd;
  if (pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *curproc = myproc();

  for (fd = 0; fd < NOFILE; fd++)
  {
    if (curproc->ofile[fd] == 0)
    {
      curproc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

int sys_dup(void)
{
  struct file *f;
  int fd;

  if (argfd(0, 0, &f) < 0)
    return -1;
  if ((fd = fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

int sys_read(void)
{
  struct file *f;
  int n;
  char *p;

  if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int sys_write(void)
{
  struct file *f;
  int n;
  char *p;

  if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return filewrite(f, p, n);
}

int sys_close(void)
{
  int fd;
  struct file *f;

  if (argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

int sys_fstat(void)
{
  struct file *f;
  struct stat *st;

  if (argfd(0, 0, &f) < 0 || argptr(1, (void *)&st, sizeof(*st)) < 0)
    return -1;
  return filestat(f, st);
}

// Create the path new as a link to the same inode as old.
int sys_link(void)
{
  char name[DIRSIZ], *new, *old;
  struct inode *dp, *ip;

  if (argstr(0, &old) < 0 || argstr(1, &new) < 0)
    return -1;

  begin_op();
  if ((ip = namei(old)) == 0)
  {
    end_op();
    return -1;
  }

  ilock(ip);
  if (ip->type == T_DIR)
  {
    iunlockput(ip);
    end_op();
    return -1;
  }

  ip->nlink++;
  iupdate(ip);
  iunlock(ip);

  if ((dp = nameiparent(new, name)) == 0)
    goto bad;
  ilock(dp);
  if (dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0)
  {
    iunlockput(dp);
    goto bad;
  }
  iunlockput(dp);
  iput(ip);

  end_op();

  return 0;

bad:
  ilock(ip);
  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);
  end_op();
  return -1;
}

// Is the directory dp empty except for "." and ".." ?
int isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de))
  {
    if (readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if (de.inum != 0)
      return 0;
  }
  return 1;
}

int kunlink(char *path)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ];
  uint off;

  begin_op();
  if ((dp = nameiparent(path, name)) == 0)
  {
    end_op();
    return -1;
  }

  ilock(dp);

  // Cannot unlink "." or "..".
  if (namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if ((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  ilock(ip);

  if (ip->nlink < 1)
    panic("unlink: nlink < 1");
  if (ip->type == T_DIR && !isdirempty(ip))
  {
    iunlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if (writei(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if (ip->type == T_DIR)
  {
    dp->nlink--;
    iupdate(dp);
  }
  iunlockput(dp);

  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);

  end_op();

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return -1;
}

//PAGEBREAK!
int sys_unlink(void)
{
  char *path;

  if (argstr(0, &path) < 0)
    return -1;

  return kunlink(path);
}

struct inode *
create(char *path, short type, short major, short minor)
{
  uint off;
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if ((dp = nameiparent(path, name)) == 0)
    return 0;
  ilock(dp);

  if ((ip = dirlookup(dp, name, &off)) != 0)
  {
    iunlockput(dp);
    ilock(ip);
    if (type == T_FILE && ip->type == T_FILE)
      return ip;
    iunlockput(ip);
    return 0;
  }

  if ((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  rtcdate date;
  datetime(&date);
  ip->ctime = dateToTimestamp(&date);
  iupdate(ip);

  if (type == T_DIR)
  {              // Create . and .. entries.
    dp->nlink++; // for ".."
    iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if (dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if (dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  iunlockput(dp);

  return ip;
}

int sys_open(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;

  if (argstr(0, &path) < 0 || argint(1, &omode) < 0)
    return -1;

  begin_op();

  if (omode & O_CREATE)
  {
    ip = create(path, T_FILE, 0, 0);
    if (ip == 0)
    {
      end_op();
      return -1;
    }
  }
  else
  {
    if ((ip = namei(path)) == 0)
    {
      end_op();
      return -1;
    }
    ilock(ip);
    if (ip->type == T_DIR && omode != O_RDONLY)
    {
      iunlockput(ip);
      end_op();
      return -1;
    }
  }

  if ((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0)
  {
    if (f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  if (omode & O_ADD)
  {
    f->off = ip->size;
  }
  else
  {
    f->off = 0;
  }
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  f->showable = (omode & O_SHOW);
  return fd;
}

int sys_mkdir(void)
{
  char *path;
  struct inode *ip;

  begin_op();
  if (argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0)
  {
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int sys_mknod(void)
{
  struct inode *ip;
  char *path;
  int major, minor;

  begin_op();
  if ((argstr(0, &path)) < 0 ||
      argint(1, &major) < 0 ||
      argint(2, &minor) < 0 ||
      (ip = create(path, T_DEV, major, minor)) == 0)
  {
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

void updatecwdname(char *cwdname, char *path)
{
  while (1)
  {
    int endflag = 0;
    switch (*path)
    {
    case '/':
      safestrcpy(cwdname, path, sizeof(path));
      endflag = 1;
      break;
    case '.':
      if (path[1] == '.')
      {
        char *slash = cwdname;
        char *p = cwdname;
        while (*p)
        {
          if (*p == '/')
            slash = p;
          p++;
        }
        p = slash;
        if (p == cwdname)
          p++;
        while (*p)
        {
          *p = 0;
          p++;
        }
        endflag = 1;
        if (path[2] == '/')
        {
          path += 3;
          endflag = 0;
        }
      }
      else if (path[1] == '/')
      {
        path += 2;
      }
      else
      {
        endflag = 1;
      }
      break;
    default:
    {
      char *p = cwdname;
      while (*p)
      {
        p++;
      }
      p--;
      if (*p == '/')
      {
        p++;
        safestrcpy(p, path, sizeof(path));
        //printf(2,"%s %s\n",cwdname,path);
      }
      else
      {
        p++;
        *p = '/';
        p++;
        safestrcpy(p, path, sizeof(path));
        //printf(2,"%s %s\n",cwdname,path);
      }
      endflag = 1;
    }
    break;
    }
    if (endflag)
      break;
  }
}
int sys_chdir(void)
{
  char *path;
  struct inode *ip;
  struct proc *curproc = myproc();

  begin_op();
  if (argstr(0, &path) < 0 || (ip = namei(path)) == 0)
  {
    end_op();
    return -1;
  }
  ilock(ip);
  if (ip->type != T_DIR)
  {
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  iput(curproc->cwd);
  end_op();
  curproc->cwd = ip;
  updatecwdname(curproc->cwdname, path);
  return 0;
}

int sys_exec(void)
{
  char *path, *argv[MAXARG];
  int i;
  uint uargv, uarg;

  if (argstr(0, &path) < 0 || argint(1, (int *)&uargv) < 0)
  {
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for (i = 0;; i++)
  {
    if (i >= NELEM(argv))
      return -1;
    if (fetchint(uargv + 4 * i, (int *)&uarg) < 0)
      return -1;
    if (uarg == 0)
    {
      argv[i] = 0;
      break;
    }
    if (fetchstr(uarg, &argv[i]) < 0)
      return -1;
  }
  return exec(path, argv);
}

int sys_pipe(void)
{
  int *fd;
  struct file *rf, *wf;
  int fd0, fd1;

  if (argptr(0, (void *)&fd, 2 * sizeof(fd[0])) < 0)
    return -1;
  if (pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0)
  {
    if (fd0 >= 0)
      myproc()->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  fd[0] = fd0;
  fd[1] = fd1;
  return 0;
}

int sys_getcwd(void)
{
  char *cwd;
  if (argstr(0, &cwd) < 0)
    return -1;
  safestrcpy(cwd, myproc()->cwdname, sizeof(myproc()->cwdname));
  return 0;
}

int sys_icmptest(void)
{
  struct nic_device *nd;
  if (get_device("mynet0", &nd) < 0)
  {
    cprintf("ERROR:icmptest:Device not loaded\n");
    return -1;
  }

  if (send_icmpRequest(nd, "10.0.2.2", 8, 0) < 0)
  {
    cprintf("ERROR:send request fails");
    return -1;
  }
  return 0;
}

int sys_hide(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], *path;
  uint off;

  if (argstr(0, &path) < 0)
    return -1;

  begin_op();
  if ((dp = nameiparent(path, name)) == 0)
  {
    end_op();
    return -1;
  }

  ilock(dp);

  // Cannot unlink "." or "..".
  if (namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if ((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  ilock(ip);

  if (ip->nlink < 1)
    panic("unlink: nlink < 1");
  if (ip->type == T_DIR && !isdirempty(ip))
  {
    iunlockput(ip);
    goto bad;
  }

  if (ip->type == T_DIR)
  {
    if (dp->showable != O_HIDE)
    {                        //if it is not hided
      dp->showable = O_HIDE; //hide it
      cprintf(name);
      cprintf(" delete completed(hide)\n");
    }
    else
    { //it has already hided
      memset(&de, 0, sizeof(de));
      if (writei(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
        panic("unlink: writei");
      dp->nlink--; //hide change to unlink
      cprintf(name);
      cprintf(" delete completed(unlink)\n");
    }
    iupdate(dp);
  }
  iunlockput(dp);
  if (ip->showable != O_HIDE)
  {                        //if it is not hided
    ip->showable = O_HIDE; //hide it
    cprintf(name);
    cprintf(" delete completed(hide)\n");
  }
  else
  { //it has already hided
    memset(&de, 0, sizeof(de));
    if (writei(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
      panic("unlink: writei");
    ip->nlink--; //hide change to unlink
    cprintf(name);
    cprintf(" delete completed(unlink)\n");
  }

  iupdate(ip);
  iunlockput(ip);
  end_op();

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return -1;
}

int sys_arp(void)
{
  char *ipAddr;
  int size;

  if (argstr(0, &ipAddr) < 0) // || argint(3, &size) < 0 || argptr(2, &arpResp, size) < 0)
  {
    cprintf("ERROR:sys_createARP:Failed to fetch arguments");
    return -1;
  }

  struct nic_device *nd;
  if (get_device("mynet0", &nd) < 0)
  {
    cprintf("ERROR:send_arpRequest:Device not loaded\n");
    return -1;
  }

  // if (send_arpRequest(interface, ipAddr, arpResp) < 0)
  if (send_arpRequest(nd, ipAddr) < 0)
  {
    cprintf("ERROR:sys_createARP:Failed to send ARP Request for IP:%s", "10.0.2.2");
    return -1;
  }
  return 0;
}

static uint32_t e1000_reg_read(uint32_t reg_addr, struct e1000 *the_e1000)
{
  uint32_t value = *(uint32_t *)(the_e1000->membase + reg_addr);
  return value;
}

int sys_checknic(void)
{
  int HEAD;
  int TAIL;

  if (argint(0, &HEAD) < 0 || argint(1, &TAIL) < 0)
  {
    cprintf("Error: invalid parameter");
    return -1;
  }
  struct e1000 *e1000p = (struct e1000 *)nic_devices[0].driver;
  uint32_t head = e1000_reg_read(E1000_RDH, e1000p);
  uint32_t tail = e1000_reg_read(E1000_RDT, e1000p);

  cprintf("HEAD: %x , TAIL: %x\n", head, tail);

  uint8_t *p = (uint8_t *)kalloc();
  uint16_t length = 0;
  {
    uint8_t mask = 15;
    e1000_recv(e1000p, p, &length);
    if (length != 0)
    {
      for (int i = 0; i < length; ++i)
      {
        cprintf("%x%x ", ((*p) >> 4) & mask, (*p) & mask);
        ++p;
      }
      cprintf("\n");
    }
  }

  return 0;
}

// Create the path new as a link to the same inode as old.
int sys_show(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], *path;
  uint off;

  if (argstr(0, &path) < 0)
    return -1;

  begin_op();
  if ((dp = nameiparent(path, name)) == 0)
  {
    end_op();
    return -1;
  }

  ilock(dp);

  // Cannot hide "." or "..".
  if (namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if ((ip = dirlookup(dp, name, &off)) == 0)
  {
    goto bad;
  }

  ilock(ip);

  if (ip->nlink < 1)
    panic("hide: nlink < 1");
  if (ip->type == T_DIR && !isdirempty(ip))
  {
    iunlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));

  if (ip->type == T_DIR)
  {
    if (dp->showable != O_SHOW)
    {                        //if it is not showed
      dp->showable = O_SHOW; //show it
      cprintf(name);
      cprintf(" refresh completed\n");
    }
    iupdate(dp);
  }
  iunlockput(dp);
  if (ip->showable != O_SHOW)
  {                        //if it is not showed
    ip->showable = O_SHOW; //show it
    cprintf(name);
    cprintf(" refresh completed\n");
  }

  iupdate(ip);
  iunlockput(ip);
  end_op();

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return -1;
}

int sys_isatty(void)
{
  int fd, res = 0;
  struct file *f;
  if (argfd(0, &fd, &f) < 0)
  {
    return 0;
  }
  if (f->type == FD_INODE)
  {
    ilock(f->ip);
    res = f->ip->type == T_DEV; //must be console
    iunlock(f->ip);
  }
  return res;
}

// lseek code derived from https://github.com/ctdk/xv6
int sys_lseek(void)
{
  int fd;
  int offset;
  int base;
  int newoff = -1;
  int zerosize, i;
  char *zeroed, *z;

  struct file *f;

  if ((argfd(0, &fd, &f) < 0) ||
      (argint(1, &offset) < 0) || (argint(2, &base) < 0))
    return (EINVAL);

  if (base == SEEK_SET)
  {
    newoff = offset;
  }
  else if (base == SEEK_CUR)
  {
    newoff = f->off + offset;
  }
  else if (base == SEEK_END)
  {
    newoff = f->ip->size + offset;
  }

  if (newoff < 0)
    return EINVAL;

  if (newoff > f->ip->size)
  {
    zerosize = newoff - f->ip->size;
    zeroed = kalloc();
    z = zeroed;
    for (i = 0; i < PGSIZE; i++)
      *z++ = 0;
    while (zerosize > 0)
    {
      filewrite(f, zeroed, zerosize);
      zerosize -= PGSIZE;
    }
    kfree(zeroed);
  }

  f->off = (uint)newoff;
  return newoff;
}

int sys_ipconfig(void)
{
  char *cmd, *val;
  if (argstr(0, &cmd) < 0 || argstr(1, &val) < 0)
  {
    cprintf("ERROR:sys_ipconfig:Failed to fetch arguments");
    return 0;
  }

  struct nic_device *nd;
  if (get_device("mynet0", &nd) < 0)
  {
    cprintf("ERROR:sys_ipconfig:Device not loaded\n");
    return -1;
  }

  // cprintf("cmd:%s\n\n", cmd);
  // cprintf("ip:%s\n\n", val);

  struct e1000 *e1000 = (struct e1000 *)(nd->driver);

  if (strncmp(cmd, "inet", 4) == 0)
  {
    e1000->ip = IP2int(val);
  }
  else if (strncmp(cmd, "gateway", 7) == 0)
  {
    e1000->gateway_ip = IP2int(val);
  }

  char mac_str[18] = { '\0' };
  unpack_mac(e1000->mac_addr, mac_str);

  cprintf("%x\n\n", e1000->ip);
  char inet[24] = {0};
  parse_ip(e1000->ip, inet);
  char gateway[24] = {0};
  parse_ip(e1000->gateway_ip, gateway);

  cprintf("mynet0\nLink encap:Ethernet  HWaddr %s\n", mac_str);
  cprintf("inet addr:%s  Gateway:%s\n", inet, gateway);
  return 0;
}
int
sys_connect(void)
{
  struct file *f;
  int fd;
  uint32_t raddr;
  uint32_t rport;
  uint32_t lport;

  if (argint(0, (int*)&raddr) < 0 ||
      argint(1, (int*)&lport) < 0 ||
      argint(2, (int*)&rport) < 0) {
    return -1;
  }

  if(sockalloc(&f, raddr, lport, rport) < 0)
    return -1;
  if((fd=fdalloc(f)) < 0){
    fileclose(f);
    return -1;
  }

  return fd;
}
