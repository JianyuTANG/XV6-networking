//
// Created by gerw on 6/9/18.
//

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#undef errno
extern int errno;

int process_start_time;

char *__env[1] = {0};
char **environ = __env; /* pointer to array of char * strings that define the current environment variables */

extern int exec(char *name, char **argv);

int execve(char *name, char **argv, char **env) {
    exec(name, argv);
}


#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

struct xv6_stat {
    short type;  // Type of file
    int dev;     // File system's disk device
    uint ino;    // Inode number
    short nlink; // Number of links to file
    uint size;   // Size of file in bytes
};

extern int xv6_fstat(int file, struct xv6_stat *st);

int fstat(int file, struct stat *st) {
    struct xv6_stat mystat;
    int res = xv6_fstat(file, &mystat);
    if (st) {
        switch (mystat.type) {
            case T_DIR:
                st->st_mode |= S_IFDIR;
                break;
            case T_FILE:
                st->st_mode |= S_IFREG;
                break;
            case T_DEV://must be console in xv6
                st->st_mode |= S_IFCHR;
                break;
            default:
                break;
        }
        st->st_dev = (dev_t) mystat.dev;
        st->st_ino = (ino_t) mystat.ino;
        st->st_nlink = (nlink_t) mystat.nlink;
        st->st_size = mystat.size;
    }
    return res;
    //TODO: file open mode, more fields in struct stat
}

extern int xv6_kill(int pid);

int kill(int pid, int sig) {
    return xv6_kill(pid);
    //TODO: support different sigs
}

extern int xv6_open(const char *name, int flags);

int open(const char *name, int flags, ...) {
    //ignore extra flags
    return xv6_open(name, flags);
}

extern int close(int fd);

int stat(const char *file, struct stat *st) {
    int fd = open(file, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    int r = fstat(fd, st);
    close(fd);
    return r;
}

extern int uptime();

clock_t times(struct tms *buf) {
    buf->tms_cutime = buf->tms_utime = (clock_t) (uptime() - process_start_time);
    buf->tms_cstime = buf->tms_stime = 0;
    return buf->tms_utime;
}

extern int xv6_wait();

int wait(int *status) {
    if (status) {
        *status = 0;
    }
    return xv6_wait();
    //TODO get child return value in status
}

struct rtcdate {
    uint second;
    uint minute;
    uint hour;
    uint day;
    uint month;
    uint year;
};

extern int gettime(struct rtcdate *);

extern time_t mktime(struct tm *tim_p);

// The use of the timezone structure is obsolete;
// the tz argument should normally be specified as NULL. (See NOTES below.)
// So we don't need to implement it
int gettimeofday(struct timeval *p, void *tz) {
    struct timezone *z = tz;
    struct rtcdate content;
    int res = gettime(&content);
    struct tm tm;
    tm.tm_sec = content.second;
    tm.tm_min = content.minute;
    tm.tm_hour = content.hour;
    tm.tm_mday = content.day;
    tm.tm_mon = content.month - 1;
    tm.tm_year = content.year - 1900;
    tm.tm_isdst = 0;
    time_t timestamp = mktime(&tm) + 28800;//convert to china
    if (p) {
        p->tv_sec = timestamp;
        p->tv_usec = 0;
    }
    if (z) {
        z->tz_dsttime = 0;
        z->tz_minuteswest = 0;
    }
    return res;
}
