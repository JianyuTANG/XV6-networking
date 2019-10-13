#include "types.h"
#include "user.h"
#include "PVCLib.h"

int random(int seed)
{
  static int rand_num = 3;
  if (seed > 0)
    rand_num = seed;
  rand_num *= 3;
  if(rand_num < 0)
    rand_num = -rand_num;
  return rand_num % 997;
}

static void
sprintint(char * dst, int * j, int xx, int base, int sgn, int size)
{
  static char digits[] = "0123456789ABCDEF";
  char buf[16];
  int i, neg;
  uint x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  size -= i;
  while (size > 0)
  {
    dst[(*j)++] = '0';
    --size;
  }

  while(--i >= 0)
    dst[(*j)++] = buf[i];
}

void sprintf(char * dst, char * fmt, ...)
{
  char *s;
  char buf[10];
  int bi = 0;
  int c, i, state, j;
  uint *ap;
  j = 0;

  state = 0;
  ap = (uint*)(void*)&fmt + 1;
  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        dst[j++] = c;
      }
    } else if(state == '%'){
      if(c == 'd'){
        buf[bi] = '\0';
        int l = atoi(buf);
        bi = 0;
        sprintint(dst, &j, *ap, 10, 1, l);
        ap++;
      } else if(c == 'x' || c == 'p'){
        buf[bi] = '\0';
        int l = atoi(buf);
        bi = 0;
        sprintint(dst, &j, *ap, 16, 0, l);
        ap++;
      } else if(c == 's'){
        s = (char*)*ap;
        ap++;
        if(s == 0)
          s = "(null)";
        while(*s != 0){
          dst[j++] = *s;
          s++;
        }
      } else if(c == 'c'){
        dst[j++] = *ap;
        ap++;
      } else if(c == '%'){
        dst[j++] = c;
      } else if(c >= '0' && c <= '9')
      {
        buf[bi++] = c;
        continue;
      }
      else {
        // Unknown % sequence.  Print it to draw attention.
        dst[j++] = '%';
        dst[j++] = c;
      }
      state = 0;
    }
  }
  dst[j++] = '\0';
}

double nResult(double x, double n)
{
	return n == 1 ? x : nResult(x, n - 1) * x / n;
}
