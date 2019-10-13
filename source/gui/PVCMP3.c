#include "PVCMP3.h"


GameStatus CreateGameStatus(int posX, int posY, int sizeX, int sizeY, PBitmap hBmp)
{
	GameStatus gameStatus;
	gameStatus.pos.x = posX;
	gameStatus.pos.y = posY;
	gameStatus.size.cx = sizeX;
	gameStatus.size.cy = sizeY;
	gameStatus.hBmp = hBmp;
	gameStatus.totalDist = 0;
	gameStatus.isPaused = false;
	return gameStatus;
}

LastMusic  CreateLast(int posX, int posY, int sizeX, int sizeY, PBitmap hBmp)
{
	LastMusic lastMusic;
	lastMusic.pos.x = posX;
	lastMusic.pos.y = posY;
	lastMusic.size.cx = sizeX;
	lastMusic.size.cy = sizeY;
	lastMusic.hBmp = hBmp;
	return lastMusic;
}

Building CreateBuilding(int posX, int posY, int sizeX, int sizeY, PBitmap hBmp)
{
	Building building;
	building.hBmp = hBmp;
	building.pos.x = posX;
	building.pos.y = posY;
	building.size.cx = sizeX;
	building.size.cy = sizeY;
	return building;
}

NextMusic  CreateNext(int posX, int posY, int sizeX, int sizeY, PBitmap hBmp)
{
	NextMusic nextMusic;
	nextMusic.pos.x = posX;
	nextMusic.pos.y = posY;
	nextMusic.size.cx = sizeX;
	nextMusic.size.cy = sizeY;
	nextMusic.hBmp = hBmp;
	return nextMusic;
}

void DrawMP3Name(PHwnd hwnd)
{
	int i, t = 0; 
	PHdc hdc = pvcGetDC(hwnd);
	hdc->font.color = RGB(0, 0, 0);
	for(i = 0; i < sound_num; i++)
	{
	    pvcDrawText(hdc, sound_name[i], MBEGINE_X, MBEGINE_Y + t);
	    t += 20;
	}
	pvcDrawText(hdc, sound_name[c_id - 1], 155, 270);
}

void PlayMusic() 
{
	int pid, playid;
	char* name = sound_name[c_id - 1];
	char* argv1[] = { "playWav", name};
	char* argv2[] = { "playmp3", name};
	printf(0, "start playing music %s \n", name);
	pid = fork();
	if (pid > 0) playid = pid;
	if (pid < 0)
	{
		printf(1, "init playmusic: fork failed\n");
		exit();
	}
	if (pid == 0)
	{
		if (name[strlen(name) - 1] == '3') {
			if (playid) {
				kill(playid);
				playid = -1;
			}
			exec("playmp3", argv2);
		} else {
			if (playid) {
				kill(playid);
				playid = -1;
			}
        	exec("playWav", argv1);
		}
		printf(1, "init playmusic: exec play failed\n");
		exit();
	}
}

void LButtonDownPause(PHwnd hwnd, PMessage msg)
{
	PPoint ptMouse;
	ptMouse.x = ((msg.param >> 16) & 0xffff) - hwnd->clientPos.x;
	ptMouse.y = (msg.param & 0xffff) - hwnd->clientPos.y;

	//如果点击了继续或暂停图标
	if (Paused(ptMouse))
	{
		if (!m_gameStatus.isPaused)
		{
			m_gameStatus.isPaused = true;
		}
		else
		{
			m_gameStatus.isPaused = false;
		}
		pvcInvalidate(hwnd);
		pause();
	}
}

void LButtonDownNext(PHwnd hwnd, PMessage msg)
{
	PPoint ptMouse;
	ptMouse.x = ((msg.param >> 16) & 0xffff) - hwnd->clientPos.x;
	ptMouse.y = (msg.param & 0xffff) - hwnd->clientPos.y;

	//如果点击了next图标
	if (Next(ptMouse))
	{
		m_gameStatus.isPaused = false;
		c_id++;
		if(c_id > sound_num)
		{
		    c_id = 1;
		}
		pvcInvalidate(hwnd);
		PlayMusic();
	}
}

void LButtonDownLast(PHwnd hwnd, PMessage msg)
{
	PPoint ptMouse;
	ptMouse.x = ((msg.param >> 16) & 0xffff) - hwnd->clientPos.x;
	ptMouse.y = (msg.param & 0xffff) - hwnd->clientPos.y;

	//如果点击了last图标
	if (Last(ptMouse))
	{
		m_gameStatus.isPaused = false;
		c_id--;
		if(c_id == 0)
		{
		    c_id = 3;
		}
		pvcInvalidate(hwnd);
		PlayMusic();
	}
}

bool Paused(PPoint ptMouse)
{
	if (ptMouse.x > m_gameStatus.pos.x && ptMouse.x < m_gameStatus.pos.x + m_gameStatus.size.cx)
		if (ptMouse.y > m_gameStatus.pos.y && ptMouse.y < m_gameStatus.pos.y + m_gameStatus.size.cy)
			return true;
	return false;
}

bool Next(PPoint ptMouse)
{
	if (ptMouse.x > m_next.pos.x && ptMouse.x < m_next.pos.x + m_next.size.cx)

		if (ptMouse.y > m_next.pos.y && ptMouse.y < m_next.pos.y + m_next.size.cy)
			return true;
	return false;
}

bool Last(PPoint ptMouse)
{
	if (ptMouse.x > m_last.pos.x && ptMouse.x < m_last.pos.x + m_last.size.cx)


		if (ptMouse.y > m_last.pos.y && ptMouse.y < m_last.pos.y + m_last.size.cy)
			return true;
	return false;
}

void Render(PHwnd hwnd)
{
	PHdc hdc = pvcGetDC(hwnd);
	hdc->pen.color = COLOR_NULL;
    	hdc->brush.color = RGB(235, 235, 235);
    	pvcDrawRect(hdc, 0, 0, hwnd->dc.size.cx, hwnd->dc.size.cy);
	hdc->brush.color = RGB(255, 255, 255);
	pvcDrawRect(hdc, 5, WND_EDGE_SIZE + 5, hwnd->dc.size.cx - 10, hwnd->dc.size.cy - 150);
    	PHdc hdcBmp;
	     hdcBmp = pvcCreateCompatibleDCFromBitmap(m_last.hBmp);
		pvcTransparentBit(hdc, m_last.pos.x, m_last.pos.y,
		hdcBmp, 0, 0, m_last.size.cx, m_last.size.cy,
		RGB(255, 255, 255));

	     hdcBmp = pvcCreateCompatibleDCFromBitmap(m_next.hBmp);
		pvcTransparentBit(hdc, m_next.pos.x, m_next.pos.y,
		hdcBmp, 0, 0, m_next.size.cx, m_next.size.cy,
		RGB(255, 255, 255));
 
	     hdcBmp = pvcCreateCompatibleDCFromBitmap(m_gameStatus.hBmp);
		pvcTransparentBit(hdc, m_gameStatus.pos.x, m_gameStatus.pos.y,
		hdcBmp, 0, m_gameStatus.size.cy * m_gameStatus.isPaused,
		m_gameStatus.size.cx, m_gameStatus.size.cy, RGB(255, 255, 255));

	DrawMP3Name(hwnd);

}


bool wndProc(PHwnd hwnd, PMessage msg)
{
  switch (msg.type)
  {
	case MSG_CREATE:
  	{
    	//加载游戏状态位图
	m_hGameStatusBmp =  pvcLoadBitmap("RHGameStatus.bmp");
	m_hLastMusic = pvcLoadBitmap("last.bmp");
	m_hNextMusic = pvcLoadBitmap("next.bmp");

	//创建游戏状态
	m_gameStatus = CreateGameStatus(170, 320, 40, 30, m_hGameStatusBmp);
	m_last = CreateLast(50, 320, 80, 30, m_hLastMusic);
	m_next = CreateNext(250, 320, 80, 30, m_hNextMusic);

  	}
	break;
	case MSG_LBUTTON_DOWN:
		//左鼠标事件
		LButtonDownPause(hwnd, msg);
		LButtonDownNext(hwnd, msg);
		LButtonDownLast(hwnd, msg);
		break;
	case MSG_PAINT:
  	{
    		Render(hwnd);
		if (msg.type == WPT_DC)
			msg.type = WPT_DIRECT;
  	}
		break;
  	case MSG_DESTROY:
  {
    if (MB_ACCEPT == pvcMessageBox (hwnd, "MP3", "Do you want to quit?"))
      break;
    return FINISH;
  }
  default:
    break;
  }
  return pvcWndProc(hwnd, msg);
}

int main()
{
	PHwnd hwnd = pvcCreateWindow("MP3", 0, 100, 100, MP3_WIDTH, MP3_HEIGHT);
	printf(0, "start playing music \n");
	PlayMusic();
	pvcWndExec(hwnd, wndProc);
	exit();
}
