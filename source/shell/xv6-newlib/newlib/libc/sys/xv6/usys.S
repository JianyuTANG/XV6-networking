// System call numbers
#define SYS_fork       1
#define SYS__exit       2
#define SYS_xv6_wait       3
#define SYS_pipe       4
#define SYS_read       5
#define SYS_xv6_kill       6
#define SYS_exec       7
#define SYS_xv6_fstat      8
#define SYS_chdir      9
#define SYS_dup       10
#define SYS_getpid    11
#define SYS_sbrk      12
#define SYS_sleep     13
#define SYS_uptime    14
#define SYS_xv6_open      15
#define SYS_write     16
#define SYS_mknod     17
#define SYS_unlink    18
#define SYS_link      19
#define SYS_mkdir     20
#define SYS_close     21
#define SYS_nfpgs     32
#define SYS_writeshm  33
#define SYS_readshm   34
#define SYS_createshm 35
#define SYS_deleteshm 36
#define SYS_clearc      23
#define SYS_insertc     24
#define SYS_setconsole  25
#define SYS_shutdown    26
#define SYS_gettimestamp 28
#define SYS_getcwd  29
#define SYS_hide   	45
#define SYS_show   	46

#define SYS_inittaskmgr  41
#define SYS_closetaskmgr 42
#define SYS_getprocinfo  43
#define SYS_updscrcont   44

#define SYS_gettime 47
#define SYS_isatty 48
#define SYS_lseek 49

// These are arbitrarily chosen, but with care not to overlap
// processor defined exceptions or interrupt vectors.
#define T_SYSCALL       64      // system call

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(_exit)
SYSCALL(xv6_wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(xv6_kill)
SYSCALL(exec)
SYSCALL(xv6_open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(xv6_fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(gettime)
SYSCALL(isatty)
SYSCALL(lseek)
