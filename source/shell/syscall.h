// System call numbers
#define SYS_fork       1
#define SYS_exit       2
#define SYS_wait       3
#define SYS_pipe       4
#define SYS_read       5
#define SYS_kill       6
#define SYS_exec       7
#define SYS_fstat      8
#define SYS_chdir      9
#define SYS_dup       10
#define SYS_getpid    11
#define SYS_sbrk      12
#define SYS_sleep     13
#define SYS_uptime    14
#define SYS_open      15
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

#define SYS_reparent 50
#define SYS_getstate 51
#define SYS_suspend 52
#define SYS_gettime 47
#define SYS_isatty 48
#define SYS_lseek 49
#define SYS_getsharedpages 50
#define SYS_showproc 51

// Error results
#define EPERM		 -1		/* Operation not permitted */
#define ENOENT		 -2		/* No such file or directory */
#define E2BIG		 -7		/* Argument list too long */
#define EBADF		 -9		/* Bad file descriptor */
#define ENOMEM		-12		/* Cannot allocate memory */
#define EACCES		-13		/* Permission denied */
#define EEXIST		-17		/* File exists */
#define ENOTDIR		-20		/* Not a directory */
#define EISDIR		-21		/* Is a directory */
#define EINVAL		-22		/* Invalid argument */
#define ENFILE		-23		/* Too many open files in system */
#define EMFILE		-24		/* Too many open files */
#define ENOTTY		-25		/* Inappropriate ioctl for device */
#define EFBIG		-27		/* File too large */
#define EPIPE		-32		/* Broken pipe */
