#ifndef PVCDESKTOP_H
#define PVCDESKTOP_H

#include "PVC.h"

#define TASKBAR_WIDTH 720
#define TASKBAR_ITEM_MAX_WIDTH 100

typedef struct TaskBarItem
{
  int wndId;
  char title[WND_MAX_TITLE_LENGHT];
  int state;
  struct TaskBarItem * next;
} TaskBarItem;

PHControl taskBar;

typedef struct TaskBarData
{
  int num;
  TaskBarItem * select;
  TaskBarItem * mouse;
  TaskBarItem * focus;
  TaskBarItem * head;
  TaskBarItem * tail;
} TaskBarData;

TaskBarData taskBarData;

bool taskBarProc(PControl * taskBar, PMessage msg);

typedef struct PItemData
{
  void * param;
  PHdc hdc;
  int state;
} PItemData;

PHControl desktopCreateItem(PHwnd hwnd, int x, int y, int w, int h, void * param);

bool desktopItemProc(PHControl item, PMessage msg);

void desktopPaintItem(PItemData * data, PHdc hdc);

void desktopInitStringFigure();

char* desktopFmtname(char *path);

void desktopLs(char *path, PHwnd hwnd);

#endif
