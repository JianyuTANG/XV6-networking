#ifndef PVCCONST_H
#define PVCCONST_H

typedef char bool;
#define true (char)1
#define false (char)0
#define SUBMIT true
#define FINISH false
#define TERMINATE (char)2

#define MAX_ADDRESS_LEN 512
#define MAX_ARGUMENT 5

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define WND_MAX_TITLE_LENGHT 20
#define MAX_TIMER_NUM 10
#define MESSAGE_QUEUE_SIZE 64
#define MAX_WND_NUM 10

#define WND_TITLE_HEIGHT (int)30
#define WND_MENUBAR_HEIGHT 20
#define WND_EDGE_SIZE (int)4

#define COLOR_NULL_ALPHA 0x0c
#define COLOR_NULL RGB(COLOR_NULL_ALPHA, COLOR_NULL_ALPHA, COLOR_NULL_ALPHA)

#define GBK2312_SIZE 261696
#define ASCII_SIZE 2048

#define POINTER_SIZE_CX 10
#define POINTER_SIZE_CY 16

#define WS_NONCLIENT_BIT 1

#define CS_SHOW 1
#define CS_HIDE 0

#define BS_MOUSE_OUT 0
#define BS_MOUSE_IN 1
#define BS_BUTTON_DOWN 2
#define BS_DCLK 3

#define LES_INACTIVE 0
#define LES_ACTIVE_CURSORSHOW 1
#define LES_ACTIVE_CURSORUNSHOW 2
#define LES_CURSOR_SIZE 1

#define LE_SHIFT_BIT 1
#define LE_CTRL_BIT 2
#define LE_ALT_BIT 4
#define LE_CAPSLOCK_BIT 8
#define LE_NUMLOCK_BIT 16

#define LE_ACTIVE_EDGE_SIZE 5
#define LE_TIMER_ID 0xdadc

#define MS_SHOW CS_SHOW
#define MS_HIDE CS_HIDE
#define MS_CHILD 2

#define FONT_SIZE_CX 8
#define FONT_SIZE_CY 16

#define DLG_SHAKE_TIMER_ID 0x3cfb

#define NULL_FUNCTION (void (*)(void *))-1

enum MSG {
  MSG_NULL,

  MSG_LBUTTON_UP,
  MSG_LBUTTON_DOWN,
  MSG_LBUTTON_DCLK,
  MSG_RBUTTON_UP,
  MSG_RBUTTON_DOWN,
  MSG_RBUTTON_DCLK,
  MSG_MBUTTON_UP,
  MSG_MBUTTON_DOWN,
  MSG_MBUTTON_DCLK,
  MSG_WHEEL_UP,
  MSG_WHEEL_DOWN,
  MSG_MOUSE_MOVE,
  MSG_MOUSE_IN,
  MSG_MOUSE_OUT,

  MSG_HAS_FOCUS,
  MSG_LOSE_FOCUS,
  MSG_ENBLE_FOCUS,

  MSG_KEY_UP,
  MSG_KEY_DOWN,

  MSG_TIMEOUT,

  MSG_PAINT,
  MSG_RECTPAINT,

  MSG_CREATE,
  MSG_DESTROY,

  MSG_NC_LBUTTON_UP,
  MSG_NC_LBUTTON_DOWN,

  MSG_OTHER_WINDOW_ACTION,
  MSG_WNDID_WRONG,

  MSG_STATECHAGE,

  MSG_SLIDER_POS_CHANGE,

  MSG_CTRL_REMOVE,
  MSG_DIRECT_PAINT
};

enum OtherWindowAction
{
  OTHER_WINDOW_OPEN,
  OTHER_WINDOW_CLOSE,
  OTHER_WINDOW_HAS_FOCUS,
  OTHER_WINDOW_LOSE_FOCUS
};

enum WndSetState
{
  WSS_GEOMETRY,
  WSS_HAS_FOCUS,
  WSS_LOSE_FOCUS,
  WSS_MINIMIZE,
  WSS_SHOW,
  WSS_BACKGROUND
};

enum WndPaintType
{
  WPT_WHOLEDC,
  WPT_DC,
  WPT_CONTROL,
  WPT_DIRECT
};

enum WndFocusState
{
  WFS_HAS_FOCUS,
  WFS_LOSE_FOCUS,
  WFS_NO_FOCUS,
  WFS_CHILD_FOCUS
};

enum SliderBarType
{
  SBT_HORIZONAL,
  SBT_VECTICAL,
};

#define MB_HOR_LEFTOUT 10
#define MB_VEC_LEFTOUT 10
#define MB_BUTTON_HEIGHT 20
#define MB_BUTTON_WIDTH 30

enum MessageBoxResult
{
  MB_ACCEPT,
  MB_REJECT
};

#define TICKS_PER_SEC 100

#define VK_F1          0xC1
#define VK_F2          0xC2
#define VK_F3          0xC3
#define VK_F4          0xC4
#define VK_F5          0xC5
#define VK_F6          0xC6
#define VK_F7          0xC7
#define VK_F8          0xC8
#define VK_F9          0xC9
#define VK_F10         0xCA
#define VK_F11         0xCB
#define VK_F12         0xCC
#define VK_PRINTSCEEN  0xCD
//#define VK_PAUSE       0xCE
#define VK_NUMPAD0     0xD0
#define VK_NUMPAD1     0xD1
#define VK_NUMPAD2     0xD2
#define VK_NUMPAD3     0xD3
#define VK_NUMPAD4     0xD4
#define VK_NUMPAD5     0xD5
#define VK_NUMPAD6     0xD6
#define VK_NUMPAD7     0xD7
#define VK_NUMPAD8     0xD8
#define VK_NUMPAD9     0xD9
#define VK_DOT         0xDA
#define VK_RETURN      0xDB
#define VK_MULTIPLY    0xDC
#define VK_DIVIDE      0xDD
#define VK_ADD         0xDE
#define VK_MINUS       0xDF
#define VK_HOME        0xE0
#define VK_END         0xE1
#define VK_UP          0xE2
#define VK_DOWN        0xE3
#define VK_LEFT        0xE4
#define VK_RIGHT       0xE5
#define VK_PAGEUP      0xE6
#define VK_PAGEDOWN    0xE7
#define VK_INSERT      0xE8
#define VK_DELETE      0xE9
#define VK_ESC         0xEA
#define VK_TAB         0xEB
#define VK_LSHIFT      0xEC
#define VK_RSHIFT      0xED
#define VK_LCTRL       0xEE
#define VK_RCTRL       0xEF
#define VK_LALT        0xF0
#define VK_RALT        0xF1
#define VK_CAPSLOCK    0xF2
#define VK_NUMLOCK     0xF3
#define VK_SCROLLLOCK  0xF4
#define VK_SPACE       0xF5
#define VK_ENTER       0xF6
#define VK_BACKSPACE   0xF7
#define VK_LGUI        0xF8
#define VK_RGUI        0xF9
#define VK_APPS        0xFA

#endif
