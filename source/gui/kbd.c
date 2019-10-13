#include "types.h"
#include "x86.h"
#include "defs.h"
#include "kbd.h"

extern void sendMessage(int wndId, PMessage msg);
extern PWndList wndList;

void kbdInterupt()
{
  static int shift = 0;
  static uchar *charcode[2] = {
    normalMap, e0Map
  };

  uint st, data;

  st = inb(KBSTATP);
  data = inb(KBDATAP);
  if((st & KBS_DIB) == 0 || (st & 0x20) != 0)
  {
    //cprintf("kbdInterupt return : %d\n", st);
    return;
  }

  if(data == 0xE0){
    shift = 1;
    return;
  } else if(data & 0x80){
    // Key released
    data &= 0x7F;

    PMessage msg;
    msg.type = MSG_KEY_UP;
    msg.param = charcode[shift][data];
    if (shift)
      shift = 0;
    sendMessage(wndList.entry, msg);
    return;
  }
  PMessage msg;
  msg.type = MSG_KEY_DOWN;
  msg.param = charcode[shift][data];
  if (shift)
    shift = 0;
  sendMessage(wndList.entry, msg);
}

int
kbdgetc(void)
{
  static uchar shift;
  static uchar *charcode[4] = {
    normalmap, shiftmap, ctlmap, ctlmap
  };
  uint st, data, c;

  st = inb(KBSTATP);
  data = inb(KBDATAP);
  if((st & KBS_DIB) == 0 || (st & 0x20) != 0)
  {
    //cprintf("kbdInterupt return : %d\n", st);
    return -1;
  }

  if (st & 0xc0)
  {
    return -1;
  }

  //cprintf("kbdInterupt : %d\n", st);

  //kbdInterupt(data);

  if(data == 0xE0){
    shift |= E0ESC;
    return 0;
  } else if(data & 0x80){
    // Key released
    data = (shift & E0ESC ? data : data & 0x7F);
    shift &= ~(shiftcode[data] | E0ESC);
    return 0;
  } else if(shift & E0ESC){
    // Last character was an E0 escape; or with 0x80
    data |= 0x80;
    shift &= ~E0ESC;
  }

  shift |= shiftcode[data];
  shift ^= togglecode[data];
  c = charcode[shift & (CTL | SHIFT)][data];
  if(shift & CAPSLOCK){
    if('a' <= c && c <= 'z')
      c += 'A' - 'a';
    else if('A' <= c && c <= 'Z')
      c += 'a' - 'A';
  }
  return c;
}

void
kbdintr(void)
{
  //consoleintr(kbdgetc);
  kbdInterupt();
}
