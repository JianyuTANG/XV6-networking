#ifndef USER_H
#define USER_H

struct stat;
struct rtcdate;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int reparent(int,int);
void jobs (void);
int getstate(int);
int suspend(int);
int inittaskmgr(void);
int closetaskmgr(void);
int getprocinfo(int*, char(*)[16], int*, uint*);
int updscrcont(char*, int);
int nfpgs(void);
int createshm(uint, uint, int);
int deleteshm(uint);
int readshm(uint, char*, uint, uint);
int writeshm(uint, char*, uint, uint);
int getsharedpages(void);
void showproc(void);

int setconsole(int, int, int, int, int);
void clearc(void);
void insertc(int c);
int shutdown(void);
int gettimestamp(void);
int getcwd(char *);
int hide(char*);
int show(char*);
int gettime(struct rtcdate*);
int isatty(int fd);
int lseek(int fd, int ptr, int dir);

// ulib.c
int stat(char*, struct stat*);
void printf(int, char*, ...);
char* gets(char*, int max);
int atoi(const char*);
int jobs_readline(int fd, char* line, int n);
int partition(char *src, char *par, int pos);
char getc_from_stdin(void);

#include <stdlib.h>
#include <string.h>

#endif

//char* strcpy(char*, char*);
//void *memmove(void*, void*, int);
//char* strchr(const char*, char c);
//int strcmp(const char*, const char*);
