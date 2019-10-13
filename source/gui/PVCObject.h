#ifndef PVCOBJECT_H
#define PVCOBJECT_H

#include "PVCConst.h"
#include "spinlock.h"

typedef char PFlag;

typedef struct PPoint
{
  int x;
  int y;
} PPoint;

typedef struct PRect
{
  int x;
  int y;
  int w;
  int h;
} PRect;

typedef struct PSize
{
  int cx;
  int cy;
} PSize;

typedef struct PColor
{
  uchar b;
  uchar g;
  uchar r;
} PColor;

static inline PColor RGB(uchar r, uchar g, uchar b)
{
  PColor rgb;
  rgb.r = r;
  rgb.g = g;
  rgb.b = b;
  return rgb;
}

typedef struct PPen
{
  PColor color;
  int size;
} PPen;

typedef struct PBrush
{
  PColor color;
} PBrush;

typedef struct PFont
{
  PColor color;
} PFont;

typedef struct PDc
{
  PSize size;
  PPen pen;
  PBrush brush;
  PFont font;
  PColor * content;
} PDc;

typedef PDc * PHdc;

typedef struct PMessage
{
  uint type;
  uint param;
  uint wndId;
} PMessage;

typedef struct PControl
{
  PPoint pos;
  PDc dc;
  PColor * background;
  bool state;
  struct PWindow * hwnd;
  void * data;
  bool (*ctrlProc)(struct PControl *, PMessage);
} PControl;

typedef PControl * PHControl;

typedef struct PButtonData
{
  void * param;
  void (*call)(void *);
  void (*paintButton)(struct PButtonData *, PHdc);
  char * name;
  int state;
} PButtonData;
typedef PControl * PHButton;

typedef struct PLineEditData
{
  char * buf;
  int len;
  int index;
  int cursor;
  void (*call)(void);
  void (*paintLineEdit)(struct PLineEditData *, PHdc);
  int state;
  int sx;
  int num;
} PLineEditData;
typedef PControl * PHLineEdit;

typedef struct PSingleLine
{
  int length;
  char* buf;
  int size;
}PSingleLine;

typedef struct PMultiLineEditData
{
  PSingleLine* buf;
  int rows;//输入内容的最大行数
  int rows_size;//行数的数组大小
  int position_x;//当前操作列数
  int position_y;//当前操作行数
  int cursor_x;//光标所在屏幕列数
  int cursor_y;//光标所在屏幕行数
  void (*paintMultiLineEdit)(struct PMultiLineEditData *, PHdc);
  int state;
  int sy;//开始绘制打行数
  int num;//编辑框中每行显示的字符数
  int row_num;//要显示的行数
  int rows_all;//内容转化成屏幕上的行数
}PMultiLineEditData;
typedef PControl * PHMultiLineEdit;


typedef struct PMenuUnit
{
  int state;
  void * param;
  void (*call)(void *);
  char * name;
} PMenuUnit;

typedef struct PMenuData
{
  PControl * parent;
  PMenuUnit * data;
  int len;
  int num;
  int select;
  int h;
  int w;
  bool flag;
} PMenuData;
typedef PControl * PHMenu;

typedef struct PSubMenuData
{
  PHMenu menu;
  PHMenu subMenu;
  int index;
} PSubMenuData;

typedef struct PMenuBarUnit
{
  PHMenu menu;
  int w;
  char * name;
} PMenuBarUnit;

typedef struct PMenuBarData
{
  PMenuBarUnit * data;
  int len;
  int num;
  int h;
  int select;
} PMenuBarData;
typedef PControl * PHMenuBar;

typedef struct PSliderBarData
{
  int blockSize;
  bool pressed;
  int pos;
  int dPos;
  double scale;
  int min;
  int max;
  int type;
  int id;
  void (*paintSliderBar)(PControl *, PHdc);
} PSliderBarData;
typedef PControl * PHSliderBar;

typedef struct PControlListNode
{
  PHControl ctrl;
  struct PControlListNode * next;
  struct PControlListNode * prev;
} PControlListNode;

typedef struct PControlList
{
  PControlListNode * head;
  PControlListNode * tail;
  PControlListNode * entry;
  PControlListNode * mouseIn;
  PControl * mouseCatch;
} PControlList;

typedef struct PWindow
{
  int id;
  int pid;
  int msgQueueId;
  int parentId;
  int childFocusId;
  int focusState;
  PPoint pos;
  PPoint clientPos;

  PDc dc;
  PDc wholeDc;

  PControlList ctrlLst;

  bool state;
  PPoint cursor;

  PMessage msg;
  bool (*wndProc)(struct PWindow *, PMessage);

  char title[WND_MAX_TITLE_LENGHT];
} PWindow;

typedef PWindow * PHwnd;

typedef struct PDialog
{
  PWindow * hwnd;
  PWindow * caller;
  void * data;
  int shakeCount;
} PDialog;

typedef PDialog * PHdlg;

typedef struct PMessageBoxData
{
  char * content;
  int result;
  int col;
  int row;
} PMessageBoxData;

typedef struct PGetInputDialogData
{
  char * buf;
  int len;
  int result;
} PGetInputDialogData;

typedef struct PMsgQueue
{
  PMessage data[MESSAGE_QUEUE_SIZE];
  uint head;
  uint tail;
} PMsgQueue;

typedef struct PWndListNode
{
  int prev;
  int next;
  int parentId;
  int pid;
  PHwnd hwnd;
  struct spinlock lock;
  PMessage msg;
  PRect rect;
  PRect clientRect;
  char title[WND_MAX_TITLE_LENGHT];
  int msgQueueId;
  PMsgQueue msgQueue;
} PWndListNode;

typedef struct PWndList
{
	PWndListNode data[MAX_WND_NUM];
	int head;
	int tail;
  int space;
	int desktop;
  int entry;
  int hasMouse;
  int catchMouse;
  struct spinlock lock;
} PWndList;

typedef struct PTimerList
{
	struct
	{
		int wndId;
	  int id;
	  int interval;
	  int curItrvl;
		int next;
	} data[MAX_TIMER_NUM];
	int head;
	int space;
	struct spinlock lock;
} PTimerList;

typedef struct PBitmap
{
  int width;
  int height;
  PColor * data;
} PBitmap;

#pragma pack(2)
typedef struct
{
	unsigned short	bfType;
	unsigned long   bfSize;
	unsigned short  bfReserved1;
	unsigned short  bfReserved2;
	unsigned long   bfOffBits;
} PBitmapFileHeader;

typedef struct
{
	unsigned long		biSize;
	long				biWidth;
	long				biHeight;
	unsigned short		biPlanes;
	unsigned short      biBitCount;
	unsigned long		biCompression;
	unsigned long		biSizeImage;
	long				biXPelsPerMeter;
	long				biYPelsPerMeter;
	unsigned long		biClrUsed;
	unsigned long		biClrImportant;
} PBitmapInfoHeader;

void pvcError(int);

#endif
