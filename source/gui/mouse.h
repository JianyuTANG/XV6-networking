#define MOUSEATAP         0x60    // mouse data port(I)

void initMouse(void);

void mouseInterupt(int ticks);

void checkMouseMessage(int ticks);
