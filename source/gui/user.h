struct stat;

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

int writeSoundBuf(void*, int);
int wavSectionPlay();
int pause();
int setSampleRate(int);
int beginMP3Decode();
int waitForMP3Decode();
int endMP3Decode();
int getCoreBuf();

void paintWindow(void *, int, int, void *, int, int, int, int);
void registWindow(void *);
void destroyWindow(void *);
void sendMessage(int, void *);
void getMessage(void *);
void setTimer(void *, int, int);
void killTimer(void *, int);
void initStringFigure(void *, int, void *, int);
void getStringFigure(void *, void *, int);
uint getTime();
uint getdate();
void setCursor(void *);
void flushRect(int, int, int, int);
void resetWindow(void *, int);
void getWindowInfo(int, void *);
void directPaintWindow(void *, int, int, void *, int, int, int, int);

// ulib.c
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
