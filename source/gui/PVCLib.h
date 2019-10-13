#ifndef PVCLIB_H
#define PVCLIB_H

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#define BOUND(a, b, c) MAX(MIN(b, c), a)

#define Pi 3.14159265

void sprintf(char * dst, char * fmt, ...);

int random(int seed);

#endif
