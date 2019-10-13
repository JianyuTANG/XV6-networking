#ifndef PVCCONTROL_H
#define PVCCONTROL_H

#include "PVCObject.h"

static inline bool ctrlContain(PHControl ctrl, int x, int y)
{
  return (x >= ctrl->pos.x && x < ctrl->pos.x + ctrl->dc.size.cx
    && y >= ctrl->pos.y && y < ctrl->pos.y + ctrl->dc.size.cy);
}

void pvcCtrlLstInit(PControlList * list);

void pvcCtrlLstAddToTail(PControlList * list, PHControl ctrl);

void pvcCtrlLstRemove(PControlList * list, PHControl ctrl, bool flag);

void pvcCtrlLstDestroy(PControlList * list, bool flag);

bool pvcDispatchMsgToCtrlLst(PControlList *, PMessage msg);

void pvcPaintControl(PHControl ctrl);

bool pvcCtrlProc(PHControl, PMessage);

PHControl pvcCreateControl(PHwnd hwnd, int x, int y, int w, int h,
  void * data, bool (*ctrlProc)(PHControl, PMessage));

void pvcCtrlSendMessage(PHControl ctrl, PMessage msg);

void pvcCtrlInvalidate(PHControl ctrl);

void pvcPaintButton(PButtonData *, PHdc hdc);

bool pvcButtonProc(PHButton btn, PMessage msg);

PHButton pvcCreateButton(PHwnd hwnd, char * name, int x, int y, int w, int h,
   void *, void (*call)(void *), void (*paintButton)(PButtonData *, PHdc));

void pvcPaintMenu(PMenuData * data, PHdc hdc);

bool pvcMenuProc(PHMenu menu, PMessage msg);

void pvcShowMenuAt(PHMenu menu, int x, int y);

void pvcShowMenu(PHMenu menu);

void pvcHideMenu(PHMenu menu);

void * pvcShowSubMenu(void * param);

PHMenu pvcCreateMenu(PHwnd hwnd, bool flag);

void pvcDestroySubMenu(PHMenu menu, bool flag);

void pvcDestroyMenu(PHMenu menu, bool flag);

void pvcMenuAddItem(PHMenu menu, char * name,
  void * param, void (*call)(void *));

void pvcMenuAddMenu(PHMenu menu, char * name,
  PHMenu subMenu);

void pvcPaintMenuBar(PMenuBarData * data, PHdc hdc);

bool pvcMenuBarProc(PHMenuBar menuBar, PMessage msg);

PHMenuBar pvcCreateMenuBar(PHwnd hwnd);

void pvcMenuBarAddMenu(PHMenuBar menuBar, char * name,
  PHMenu menu);

void pvcSliderBarSetPos(PHSliderBar sliderBar, int pos);

void pvcSliderBarSetScale(PHSliderBar sliderBar, double scale);

void pvcSliderBarSetRange(PHSliderBar sliderBar, int min, int max);

void pvcPaintSliderBar(PHSliderBar sliderBar, PHdc hdc);

bool pvcSliderBarProc(PHSliderBar sliderBar, PMessage msg);

PHSliderBar pvcCreateSliderBar(PHwnd hwnd, int id, int x, int y, int w, int h,
  int min, int max, double scale, int type, void (*paintSliderBar)(PHSliderBar, PHdc));

void pvcPaintLineEdit(PLineEditData *, PHdc hdc);

bool pvcLineEditProc(PHLineEdit lineEdit, PMessage msg);

void pvcGetLineEditString(PHLineEdit lineEdit, char * buf);

void pvcSetLineEditString(PHLineEdit lineEdit, char * buf);

PHLineEdit pvcCreateLineEdit(PHwnd hwnd, int x, int y, int w, int h, int len,
  void (*call)(void), void (*paintLineEdit)(PLineEditData *, PHdc));

void pvcPaintMultiLineEdit(PMultiLineEditData *, PHdc hdc);

bool pvcMultiLineEditProc(PHMultiLineEdit multiLineEdit, PMessage msg);

PHMultiLineEdit pvcCreateMultiLineEdit(PHwnd hwnd, int x, int y, int w, int h,
  void (*paintMultiLineEdit)(PMultiLineEditData *, PHdc));

#endif
