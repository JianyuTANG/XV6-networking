#ifndef PVCPHOTOVIEWER_H
#define PVCPHOTOVIEWER_H

#include"PVC.h"
#include"PVCDecodeJPEG.h"
PBitmap m_hBackgroundBmp;
PBitmap m_hBuildingBmp;
int  nowBmp=0;

struct photoNode
{
	int tag1;
	int tag2;
	char *name;
	int type;
	struct photoNode* prev;
	struct photoNode* next;
};

struct photoList
{
	struct photoNode* head;
	struct photoNode* tail;
	struct photoNode* nd;
};

enum IMGTYPE{
	JPG,
	BMP,
	NOTIMG,
};

struct photoList *PL;
bool wndProc(PHwnd hwnd, PMessage msg);
void keydown(PHwnd hwnd, PMessage msg);
void draw(PHwnd hwnd);
void init(PHwnd hwnd);
void photoListInit(struct photoList* pl);
void addPhotoByFileName(char *s,struct photoList* pl);
void update(PHwnd hwnd);
PHdc pvcLeftRotatePdc(PHdc src);
PHdc pvcRightRotatePdc(PHdc src);
PBitmap loadJpg(char *filename);
int type(char *filename);
PHdc enlarge(PHdc src);
PBitmap loadImg(char* filename);











#endif
