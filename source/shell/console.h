void            consoleinit(void);
extern void            cprintf(char*, ...);
void            consoleintr(int(*)(void));
void            panic(char*) __attribute__((noreturn));
void            clearc(void);
void            insertc(int);

void 			datetime(void*);