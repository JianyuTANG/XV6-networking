#ifndef PVCWINDOW_H
#define PVCWINDOW_H

#include "PVC.h"

PHwnd pvcCreateWindow(char * title, PHwnd parent, int x, int y, int width, int height);

bool pvcWndPreTranslateMessage(PHwnd hwnd, PMessage * msg);

bool pvcWndTranslateMessage(PHwnd hwnd, PMessage * msg);

void pvcSendMessage(PHwnd hwnd, PMessage msg);

void pvcCloseWindow(void * hwnd);

void pvcMinimizeWindow(void * hwnd);

void pvcRefresh(void * param);

bool pvcWndProc(PHwnd hwnd, PMessage msg);

void pvcWndExec(PHwnd hwnd, bool (*wndProc)(PHwnd, PMessage));

void pvcDestroyWindow(void * hwnd);

void pvcDrawWindowFrame(PHwnd hwnd);

void pvcInvalidate(PHwnd hwnd);

void pvcUpdate(PHwnd hwnd);

void pvcPaintCloseBtn(PButtonData * data, PHdc hdc);

void pvcPaintMinimizeBtn(PButtonData * data, PHdc hdc);

void runApp(void * param);

// Cmds without file name at beginning.
void runAppWithCmds(void * param, char** cmds);

#endif
