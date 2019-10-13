#ifndef PVCFILEMANAGER_H
#define PVCFILEMANAGER_H

#include "PVC.h"

#define ITEM_SIZE_CY 30
#define SLIDER_ID 1

typedef struct ItemListNode
{
  char name[WND_MAX_TITLE_LENGHT];
  int type;
} ItemListNode;

typedef struct ItemList
{
  ItemListNode * data;
  int len;
  int num;
  int select;
  int mouse;
  int startIndex;
} ItemList;

void ItemListInit(ItemList * list);

void ItemListAdd(ItemList * list, ItemListNode * data);

void ItemListDestroy(ItemList * list);

bool ItemListProc(PHControl itemListCtrl, PMessage msg);

void ItemListPaintItem(ItemList * itemList, int index, PHControl itemListCtrl);

PHLineEdit addrEdit;

ItemList itemList;

PHControl itemListCtrl;

PHSliderBar sliderBar;

typedef struct ItemListData
{
  char name[WND_MAX_TITLE_LENGHT];
  int type;
} ItemData;

bool wndProc(PHwnd hwnd, PMessage msg);

void AddrReturn();

char* FmtName(char *path);

void ListItem(char *path);

void Back(void * param);

#endif
