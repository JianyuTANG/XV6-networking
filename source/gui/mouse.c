#include "types.h"
#include "x86.h"
#include "defs.h"
#include "mouse.h"
#include "traps.h"
#include "memlayout.h"
#include "PVCObject.h"

enum BtnState
{
  BtnFirstDown,
  BtnFirstUp,
  BtnSecondDown,
  BtnSecondUp,
  Null
};

#define TICKS_INTERVAL 20

typedef struct MouseMessageUnit
{
  int state;
  int param;
  int ticks;
} MouseMessageUnit;

MouseMessageUnit lBtnUnit;
MouseMessageUnit rBtnUnit;
MouseMessageUnit mBtnUnit;

#define LBTNDOWN 0x80
#define LBTNUP   0x40
#define RBTNDOWN 0x20
#define RBTNUP   0x10
#define MBTNDOWN 0x08
#define MBTNUP   0x04

#define LBTNBIT  0x01
#define RBTNBIT  0x02
#define MBTNBIT  0x04

extern void pvcDrawMouse();
extern void sendMessage(int wndId, PMessage msg);
extern PWndList wndList;

int x_position = SCREEN_WIDTH / 2;
int y_position = SCREEN_HEIGHT / 2;

void mouseInterupt(int ticks)
{
  static int recovery = 0;
  static int count = -1;
	static int x_sign = 0;
	static int y_sign = 0;
  extern ushort videoWidth;
  extern ushort videoHeight;

  static uchar button = 0;
  static uchar buttonState = 0;

  int state = inb(0x64);

  if ((state & 1) == 0 || (state & 0x20) == 0)
  {
    //cprintf("mouseInterupt return : %d\n", state);
    return;
  }

  //cprintf("mouseInterupt : %d\n", state);
  char ch = inb(MOUSEATAP);

  if (recovery == 0 && (ch & 255) == 0)
    recovery = 1;
  else if (recovery == 1 && (ch & 255) == 0)
    recovery = 2;
  else if ((ch & 255) == 12)
    recovery = 0;
  else
    recovery = -1;

  if (count == -1)
  {
    if (ch == (char)0xfa)
    {
      cprintf("0xfa return\n");
      count = 0;
    }
    return;
  }
	switch (++count)
	{
	case 1:
    if(ch & 0x08)
    {
      //LBUTTON
      if (ch & LBTNBIT)
      {
        if (buttonState & LBTNBIT)
          button &= ~LBTNDOWN;
        else
        {
          button |= LBTNDOWN;
          buttonState |= LBTNBIT;
        }
      }
      else
      {
        if (!(buttonState & LBTNBIT))
          button &= ~LBTNUP;
        else
        {
          button |= LBTNUP;
          buttonState &= ~LBTNBIT;
        }
      }
      //RBUTTON
      if (ch & RBTNBIT)
      {
        if (buttonState & RBTNBIT)
          button &= ~RBTNDOWN;
        else
        {
          button |= RBTNDOWN;
          buttonState |= RBTNBIT;
        }
      }
      else
      {
        if (!(buttonState & RBTNBIT))
          button &= ~RBTNUP;
        else
        {
          button |= RBTNUP;
          buttonState &= ~RBTNBIT;
        }
      }
      //MBUTTON
      if (ch & MBTNBIT)
      {
        if (buttonState & MBTNBIT)
          button &= ~MBTNDOWN;
        else
        {
          button |= MBTNDOWN;
          buttonState |= MBTNBIT;
        }
      }
      else
      {
        if (!(buttonState & MBTNBIT))
          button &= ~MBTNUP;
        else
        {
          button |= MBTNUP;
          buttonState &= ~MBTNBIT;
        }
      }

      x_sign = ch & 0x10 ? 0xffffff00 : 0 ;
      y_sign = ch & 0x20 ? 0xffffff00 : 0 ;
    }
    else
      count = 0 ;
  break;
	case 2:
	  x_position += ( x_sign | ch ) ;
    if (x_position < 0)
      x_position = 0;
    else if (x_position >= videoWidth)
      x_position = videoWidth - 1;
	 	break;
	case 3:
    y_position += -( y_sign | ch ) ;
    if (y_position < 0)
      y_position = 0;
    else if (y_position >= videoHeight)
      y_position = videoHeight - 1;
	  break;
  default:
    count = 0;
  }
  if (recovery == 2)
  {
    count = 0;
    recovery = -1;
  }
  else if (count == 3)
  {
    count = 0;
    pvcDrawMouse();
    PMessage msg;
    msg.param = (x_position << 16 & 0xffff0000) | (y_position & 0x0000ffff);

    //LBUTTON
    if (button & LBTNDOWN)
    {
      if (lBtnUnit.state == Null)
      {
        lBtnUnit.state = BtnFirstDown;
        lBtnUnit.param = msg.param;
        lBtnUnit.ticks = ticks;
      }
      else if (lBtnUnit.state == BtnFirstUp)
      {
        lBtnUnit.state = BtnSecondDown;
        msg.type = MSG_LBUTTON_DCLK;
        sendMessage(wndList.head, msg);
      }
    }

    if (button & LBTNUP)
    {
      if (lBtnUnit.state == Null)
      {
        msg.type = MSG_LBUTTON_UP;
        sendMessage(wndList.head, msg);
      }
      else if (lBtnUnit.state == BtnFirstDown)
      {
        lBtnUnit.state = BtnFirstUp;
      }
      else if (lBtnUnit.state == BtnSecondDown)
      {
        lBtnUnit.state = Null;
      }
    }

    //RBUTTON
    if (button & RBTNDOWN)
    {
      if (rBtnUnit.state == Null)
      {
        rBtnUnit.state = BtnFirstDown;
        rBtnUnit.param = msg.param;
        rBtnUnit.ticks = ticks;
      }
      else if (rBtnUnit.state == BtnFirstUp)
      {
        rBtnUnit.state = BtnSecondDown;
        msg.type = MSG_RBUTTON_DCLK;
        sendMessage(wndList.head, msg);
      }
    }

    if (button & RBTNUP)
    {
      if (rBtnUnit.state == Null)
      {
        msg.type = MSG_RBUTTON_UP;
        sendMessage(wndList.head, msg);
      }
      else if (rBtnUnit.state == BtnFirstDown)
      {
        rBtnUnit.state = BtnFirstUp;
      }
      else if (rBtnUnit.state == BtnSecondDown)
      {
        rBtnUnit.state = Null;
      }
    }

    //MBUTTON
    if (button & MBTNDOWN)
    {
      if (mBtnUnit.state == Null)
      {
        mBtnUnit.state = BtnFirstDown;
        mBtnUnit.param = msg.param;
        mBtnUnit.ticks = ticks;
      }
      else if (mBtnUnit.state == BtnFirstUp)
      {
        mBtnUnit.state = BtnSecondDown;
        msg.type = MSG_MBUTTON_DCLK;
        sendMessage(wndList.head, msg);
      }
    }

    if (button & MBTNUP)
    {
      if (mBtnUnit.state == Null)
      {
        msg.type = MSG_MBUTTON_UP;
        sendMessage(wndList.head, msg);
      }
      else if (mBtnUnit.state == BtnFirstDown)
      {
        mBtnUnit.state = BtnFirstUp;
      }
      else if (mBtnUnit.state == BtnSecondDown)
      {
        mBtnUnit.state = Null;
      }
    }

    if (button == 0)
    {
      msg.type = MSG_MOUSE_MOVE;
      sendMessage(wndList.head, msg);
    }
    button = 0;
  }
}

void checkMouseMessage(int ticks)
{
  if (!(lBtnUnit.state == Null || lBtnUnit.state == BtnSecondDown || lBtnUnit.ticks + TICKS_INTERVAL > ticks))
  {
    PMessage msg;
    msg.param = lBtnUnit.param;
    msg.type = MSG_LBUTTON_DOWN;
    sendMessage(wndList.head, msg);
    if (lBtnUnit.state == BtnFirstUp)
    {
      msg.type = MSG_LBUTTON_UP;
      sendMessage(wndList.head, msg);
    }
    lBtnUnit.state = Null;
  }

  if (!(rBtnUnit.state == Null || rBtnUnit.state == BtnSecondDown || rBtnUnit.ticks + TICKS_INTERVAL > ticks))
  {
    PMessage msg;
    msg.param = rBtnUnit.param;
    msg.type = MSG_RBUTTON_DOWN;
    sendMessage(wndList.head, msg);
    if (rBtnUnit.state == BtnFirstUp)
    {
      msg.type = MSG_RBUTTON_UP;
      sendMessage(wndList.head, msg);
    }
    rBtnUnit.state = Null;
  }

  if (!(mBtnUnit.state == Null || mBtnUnit.state == BtnSecondDown || mBtnUnit.ticks + TICKS_INTERVAL > ticks))
  {
    PMessage msg;
    msg.param = mBtnUnit.param;
    msg.type = MSG_MBUTTON_DOWN;
    sendMessage(wndList.head, msg);
    if (mBtnUnit.state == BtnFirstUp)
    {
      msg.type = MSG_MBUTTON_UP;
      sendMessage(wndList.head, msg);
    }
    mBtnUnit.state = Null;
  }
}

void initMouse(void)
{
   outb(0x64, 0xa8);
   outb(0x64, 0xd4);
   outb(0x60, 0xf4);
   outb(0x64, 0x60);
   outb(0x60, 0x47);

   lBtnUnit.state = Null;
   rBtnUnit.state = Null;
   mBtnUnit.state = Null;

   picenable(IRQ_MOUSE);
   ioapicenable(IRQ_MOUSE, 0);
}
