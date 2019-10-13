#pragma once
#ifndef PVCMP3_H
#define PVCMP3_H

#include "PVC.h"

#define MP3_WIDTH  400
#define MP3_HEIGHT 400
#define WNDWIDTH    400
#define WNDHEIGHT   400
#define MBEGINE_X 10
#define MBEGINE_Y WND_EDGE_SIZE + 10


/*游戏状态结构体*/
typedef struct
{
	PBitmap	hBmp;
	PPoint   pos;
	PSize	size;
	bool	isPaused;
	int     totalDist;
}GameStatus;

typedef struct
{
	PBitmap	hBmp;
	PPoint	pos;
	PSize	size;
}LastMusic;

typedef struct
{
	PBitmap	hBmp;
	PPoint	pos;
	PSize	size;
}NextMusic;

typedef struct
{
	PBitmap	hBmp;
	PPoint	pos;
	PSize	size;
}Building;


int c_id = 1;


GameStatus CreateGameStatus(int posX, int posY, int sizeX, int sizeY, PBitmap hBmp);
LastMusic  CreateLast(int posX, int posY, int sizeX, int sizeY, PBitmap hBmp);
NextMusic  CreateNext(int posX, int posY, int sizeX, int sizeY, PBitmap hBmp);

/*声明位图句柄*/
Building      m_building;
PBitmap m_hBuildingBmp;
PBitmap m_hGameStatusBmp;
PBitmap m_hLastMusic;
PBitmap m_hNextMusic;

/*状态*/
LastMusic 	m_last;
NextMusic	m_next;	
GameStatus	m_gameStatus;


char* sound_name[] = {"test.wav", "qian.wav","in.mp3"};
int sound_num = 3;

//双缓冲绘制
void Render(PHwnd hwnd);

bool wndProc(PHwnd, PMessage);

void init(PHwnd hwnd, PMessage);

void DrawMP3Name(PHwnd hwnd);

Building CreateBuilding(int posX, int posY, int sizeX, int sizeY, PBitmap hBmp);
//左鼠标点击事件
void LButtonDownPause(PHwnd hwnd, PMessage msg);

void LButtonDownNext(PHwnd hwnd, PMessage msg);

void LButtonDownLast(PHwnd hwnd, PMessage msg);

//判断是否点击暂停
bool Paused(PPoint);

bool Next(PPoint); 	

bool Last(PPoint);
#endif 
