#ifndef PVCPAINTER_H
#define PVCPAINTER_H

#include "PVCObject.h"

PHdc pvcGetDC(PHwnd hwnd);

PHdc pvcGetWholeDC(PHwnd hwnd);

PHdc pvcCreateCompatibleDC(PHdc hdc);

void pvcDeleteDC(PHdc hdc);

void pvcBltbit(PHdc buf, int x, int y, PHdc src, int sx, int sy, int w, int h);

PPen pvcSetPen(PHdc hdc, PPen pen);

PBrush pvcSetBrush(PHdc hdc, PBrush brush);

PFont pvcSetFont(PHdc hdc, PFont font);

void pvcSetPixel(PHdc hdc, int x, int y, PColor color);

PColor pvcPixel(PHdc hdc, int x, int y);

void pvcDrawPoint(PHdc hdc, int x, int y);

void pvcDrawLine(PHdc hdc, int x1, int y1, int x2, int y2);

void pvcDrawRect(PHdc hdc, int x, int y, int w, int h);

//void pvcFillRect(PHdc hdc, int x, int y, int w, int h);

//void pvcFrameRect(PHdc hdc, int x, int y, int w, int h);

void pvcDrawEllipse(PHdc hdc, int x, int y, int w, int h);

void pvcFloodFill(PHdc hdc, int x, int y, PColor color, PFlag type);

void pvcDrawText(PHdc hdc, char * str, int x, int y);

PBitmap pvcLoadBitmap(char * filename);

PHdc pvcCreateCompatibleDCFromBitmap(PBitmap bmp);

void pvcTransparentBit(PHdc buf, int x, int y, PHdc src, int sx, int sy, int w, int h, PColor tspt);

#endif
