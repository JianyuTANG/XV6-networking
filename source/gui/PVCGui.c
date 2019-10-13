#include "types.h"
#include "x86.h"
#include "memlayout.h"
#include "defs.h"
#include "PVCGui.h"
#include "PVCPainter.h"

ushort videoWidth = 0;
ushort videoHeight = 0;
uchar bitsPerPixel = 0;
static PColor * videoAddr = 0;
static PColor * videoBuf = 0;
static PColor * videoContent = 0;
struct spinlock videoLock;

PWndList wndList;

PRect screenRect;

PRect pointerRect;

PTimerList timerList;

int timerListReady = 0;

PColor PointerData[POINTER_SIZE_CY][POINTER_SIZE_CX] =
{
	{{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0xef, 0xef, 0xef} ,{ 0xef, 0xef, 0xef} ,{ 0xef, 0xef, 0xef} ,{ 0xef, 0xef, 0xef} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0xef, 0xef, 0xef} ,{ 0x63, 0x63, 0x63} ,{ 0x63, 0x63, 0x63} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} },
	{{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0xef, 0xef, 0xef} ,{ 0xef, 0xef, 0xef} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} ,{ 0x0c, 0x0c, 0x0c} }

	/*
	{#,@,@,@,@,@,@,@,@,@},
	{#,#,@,@,@,@,@,@,@,@},
	{#,~,#,@,@,@,@,@,@,@},
	{#,~,~,#,@,@,@,@,@,@},
	{#,~,~,~,#,@,@,@,@,@},
	{#,~,~,~,~,#,@,@,@,@},
	{#,~,~,~,~,~,#,@,@,@},
	{#,~,~,~,~,~,~,#,@,@},
	{#,~,~,~,~,~,~,~,#,@},
	{#,~,~,~,~,#,#,#,#,#},
	{#,~,~,~,~,#,@,@,@,@},
	{#,~,#,#,~,~,#,@,@,@},
	{#,#,@,#,~,~,#,@,@,@},
	{#,@,@,@,#,~,~,#,@,@},
	{@,@,@,@,#,~,~,#,@,@},
	{@,@,@,@,@,#,#,@,@,@}
	*/
};

void pvcGuiInit(void)
{
	videoAddr = (PColor*)(*((uint*)P2V(0x1028)));
	videoWidth = *((ushort*)P2V(0x1012));
	videoHeight = *((ushort*)P2V(0x1014));
	bitsPerPixel = *((uchar*)P2V(0x1019));
	videoBuf = videoAddr + videoWidth * videoHeight;
	videoContent = videoBuf + videoWidth * videoHeight;

	screenRect.x = screenRect.y = 0;
	screenRect.w = videoWidth;
	screenRect.h = videoHeight;

	pointerRect.w = POINTER_SIZE_CX;
	pointerRect.h = POINTER_SIZE_CY;

	cprintf("video addr : %x, video width : %d,  video height : %d, bitsPerPixel : %d\n", videoAddr, videoWidth, videoHeight, bitsPerPixel);

	initlock(&videoLock, "videoLock");
	pvcWndListInit(&wndList);
	pvcTimerListInit(&timerList);

	pvcDrawMouse();
}

struct spinlock videoLock;

extern int x_position;
extern int y_position;

int cursor_x = -1000;
int cursor_y = -1000;

void pvcBufPaint(int x1, int y1, int x2, int y2)
{
	acquire(&videoLock);
	if (cursor_x < x2 && x1 < cursor_x + POINTER_SIZE_CX
		&& cursor_y < y2 && y1 < cursor_y + POINTER_SIZE_CY)
	{
		if (cursor_x < x1)
			x1 = cursor_x;
		if (x2 < cursor_x + POINTER_SIZE_CX)
			x2 = cursor_x + POINTER_SIZE_CX;
		if (x1 < 0)
			x1 = 0;
		if (x2 > videoWidth)
			x2 = videoWidth;
		if (cursor_y < y1)
				y1 = cursor_y;
		if (y2 < cursor_y + POINTER_SIZE_CY)
			y2 = cursor_y + POINTER_SIZE_CY;
		if (y1 < 0)
			y1 = 0;
		if (y2 > videoHeight)
			y2 = videoHeight;
		for (int i = y1; i < y2; ++i)
		{
			int off = x1 + i * videoWidth;
			memmove(videoBuf + off, videoContent + off, (x2 - x1) * sizeof(PColor));
		}
		int h = cursor_y + POINTER_SIZE_CY;
		if (h > videoHeight)
		 	h = videoHeight;
		int w = cursor_x + POINTER_SIZE_CX;
		if (w > videoWidth)
			w = videoWidth;

		for (int i = cursor_y; i < h; ++i)
		{
			int off = i * videoWidth;
			for (int j = cursor_x; j < w; ++j)
			{
				PColor c = PointerData[i - cursor_y][j - cursor_x];
				if (c.r != COLOR_NULL_ALPHA || c.g != COLOR_NULL_ALPHA || c.b != COLOR_NULL_ALPHA)
					videoBuf[off + j] = c;
			}
		}

		x2 -= x1;
		x2 *= sizeof(PColor);
		int off = x1 + y1 * videoWidth;
		for (int y = y1; y < y2; ++y)
		{
			memmove(videoAddr + off, videoBuf + off, x2);
			off += videoWidth;
		}
	}
	else
	{
		x2 -= x1;
		x2 *= sizeof(PColor);
		int off = x1 + y1 * videoWidth;
		for (int y = y1; y < y2; ++y)
		{
			memmove(videoAddr + off, videoContent + off, x2);
			off += videoWidth;
		}
	}
	release(&videoLock);
}

void pvcDrawMouse()
{
	acquire(&videoLock);
	int x1 = cursor_x;
	int x2 = x_position + POINTER_SIZE_CX;
	if (x_position < x1)
	{
		x1 = x_position;
		x2 = cursor_x + POINTER_SIZE_CX;
	}
	if (x1 < 0)
		x1 = 0;
	else if (x2 > videoWidth)
		x2 = videoWidth;

	int y1 = cursor_y;
	int y2 = y_position + POINTER_SIZE_CY;
	if (y_position < y1)
	{
		y1 = y_position;
		y2 = cursor_y + POINTER_SIZE_CY;
	}
	if (y1 < 0)
		y1 = 0;
	else if (y2 > videoHeight)
		y2 = videoHeight;

	for (int i = y1; i < y2; ++i)
	{
		int off = x1 + i * videoWidth;
		memmove(videoBuf + off, videoContent + off, (x2 - x1) * sizeof(PColor));
	}
	cursor_y = y_position;
	cursor_x = x_position;

	int h = cursor_y + POINTER_SIZE_CY;
	if (h > videoHeight)
		h = videoHeight;
	int w = cursor_x + POINTER_SIZE_CX;
	if (w > videoWidth)
		w = videoWidth;

	for (int i = cursor_y; i < h; ++i)
	{
		int off = i * videoWidth;
		for (int j = cursor_x; j < w; ++j)
		{
			PColor c = PointerData[i - cursor_y][j - cursor_x];
			if (c.r != COLOR_NULL_ALPHA || c.g != COLOR_NULL_ALPHA || c.b != COLOR_NULL_ALPHA)
				videoBuf[off + j] = c;
		}
	}

	x2 -= x1;
	x2 *= sizeof(PColor);
	int off = x1 + y1 * videoWidth;
	for (int y = y1; y < y2; ++y)
	{
		memmove(videoAddr + off, videoBuf + off, x2);
		off += videoWidth;
	}
	release(&videoLock);
}

int sys_paintWindow(void)
{
	PHwnd hwnd = 0;
  PHdc hdc = 0;
	int wx, wy, sx, sy, w, h;
  if (argstr(0, (char **)&hwnd) < 0 || argint(1, &wx) < 0 || argint(2, &wy) < 0
				|| argstr(3, (char **)&hdc) < 0 || argint(4, &sx) < 0
				|| argint(5, &sy) < 0 || argint(6, &w) < 0 || argint(7, &h) < 0)
    return -1;

	if (sx < 0 || sy < 0 || h <= 0 || w <= 0 || sx + w > hdc->size.cx || sy + h > hdc->size.cy)
		return 0;

	if (wx < 0 || wy < 0 || wx + w > hwnd->wholeDc.size.cx || wy + h > hwnd->wholeDc.size.cy)
		return 0;

	wx += hwnd->pos.x;
	wy += hwnd->pos.y;

	int id = hwnd->id;
	PColor * data = hdc->content;

	//acquire(&videoLock);
	int i, j;
	for (i = 0; i < h; ++i)
	{
		if (wy + i < 0)
		{
			i = -wy - 1;
			continue;
		}
		else if (wy + i >= videoHeight)
			break;
		int off_x = (sy + i) * hdc->size.cx + sx;
		int video_off_x = (wy + i) * videoWidth + wx;
		for (j = 0; j < w; ++j)
		{
			if (wx + j < 0)
			{
				j = -wx - 1;
				continue;
			}
			else if (wx + j >= videoWidth)
				break;
			int p = wndList.data[id].prev;
			while (p != -1)
			{
				if (contain(wndList.data[p].rect, wx + j, wy + i))
					break;
				p = wndList.data[p].prev;
			}
			if (p != -1)
				continue;
			p = hwnd->parentId;
			while (p != -1)
			{
				if (!contain(wndList.data[p].clientRect, wx + j, wy + i))
					break;
				p = wndList.data[p].parentId;
			}
			if (p != -1)
				continue;
			PColor c = data[off_x + j];
			if (c.r != COLOR_NULL_ALPHA || c.g != COLOR_NULL_ALPHA || c.b != COLOR_NULL_ALPHA)
				videoContent[video_off_x + j] = c;
		}
	}
	w += wx;
	h += wy;
	if (wx < 0)
		wx = 0;
	if (wy < 0)
		wy = 0;
	if (h > videoHeight)
		h = videoHeight;
	if (w > videoWidth)
		w = videoWidth;
	//release(&videoLock);
	pvcBufPaint(wx, wy, w, h);

	return 0;
}


int sys_directPaintWindow(void)
{
	PHwnd hwnd = 0;
  PHdc hdc = 0;
	int wx, wy, sx, sy, w, h;
  if (argstr(0, (char **)&hwnd) < 0 || argint(1, &wx) < 0 || argint(2, &wy) < 0
				|| argstr(3, (char **)&hdc) < 0 || argint(4, &sx) < 0
				|| argint(5, &sy) < 0 || argint(6, &w) < 0 || argint(7, &h) < 0)
    return -1;

	if (sx < 0 || sy < 0 || h <= 0 || w <= 0 || sx + w > hdc->size.cx || sy + h > hdc->size.cy)
		return 0;

	if (wx < 0 || wy < 0 || wx + w > hwnd->wholeDc.size.cx || wy + h > hwnd->wholeDc.size.cy)
		return 0;

	wx += hwnd->pos.x;
	wy += hwnd->pos.y;

	PColor * data = hdc->content;

	//acquire(&videoLock);
	int i, j;
	for (i = 0; i < h; ++i)
	{
		if (wy + i < 0)
		{
			i = -wy - 1;
			continue;
		}
		else if (wy + i >= videoHeight)
			break;
		for (j = 0; j < w; ++j)
		{
			if (wx + j < 0)
			{
				j = -wx - 1;
				continue;
			}
			else if (wx + j >= videoWidth)
				break;
			PColor c = data[(sy + i) * hdc->size.cx + sx + j];
			if (c.r != COLOR_NULL_ALPHA || c.g != COLOR_NULL_ALPHA || c.b != COLOR_NULL_ALPHA)
				videoContent[(wy + i) * videoWidth + (wx + j)] = c;
		}
	}
	w += wx;
	h += wy;
	if (wx < 0)
		wx = 0;
	if (wy < 0)
		wy = 0;
	if (h > videoHeight)
		h = videoHeight;
	if (w > videoWidth)
		w = videoWidth;
	//release(&videoLock);
	pvcBufPaint(wx, wy, w, h);

	return 0;
}

int sys_registWindow(void)
{
	PHwnd hwnd = 0;
	if (argstr(0, (char **)&hwnd) < 0)
    return -1;
	pvcWndListAddToHead(&wndList, hwnd);
	return 0;
}

void pvcFlushWnd(int x, int y, int w, int h)
{
	w += x;
	h += y;
	x /= 10;
	y /= 10;
	w = (w + 9) / 10;
	h = (h + 9) / 10;
	w -= x;
	h -= y;
	PMessage msg;
	msg.type = MSG_RECTPAINT;
	msg.param = ((x & 0xff) << 24) | ((y & 0xff) << 16) | ((w & 0xff) << 8) | (h & 0xff);
	acquire(&wndList.lock);
	int p = wndList.head;
	while (p != -1)
	{
		sendMessage(p, msg);
		if (p == wndList.desktop)
			break;
		p = wndList.data[p].next;
	}
	release(&wndList.lock);
}

int sys_flushRect(void)
{
	int x, y, w, h;
	if (argint(0, &x) < 0 || argint(1, &y) < 0 || argint(2, &w) < 0 || argint(3, &h) < 0)
		return -1;
	pvcFlushWnd(x, y, w, h);
	return 0;
}

int sys_destroyWindow(void)
{
	PHwnd hwnd = 0;
	if (argstr(0, (char **)&hwnd) < 0)
		return -1;
	pvcWndListRemove(&wndList, hwnd->id);
	pvcTimerListRemoveWndId(&timerList, hwnd->id);

	pvcFlushWnd(hwnd->pos.x, hwnd->pos.y, hwnd->wholeDc.size.cx, hwnd->wholeDc.size.cy);
	return 0;
}

int sys_sendMessage(void)
{
	int wndId = 0;
	PMessage * msg = 0;
	if (argint(0, &wndId) < 0 || argstr(1, (char**)&msg) < 0)
    return -1;

	sendMessage(wndId, *msg);
	return 0;
}

void sendMessage(int wndId, PMessage msg)
{
	if (wndId == -1 || wndList.data[wndId].hwnd == 0)
		return;
	switch (msg.type)
	{
		case MSG_LBUTTON_UP:
		case MSG_RBUTTON_UP:
		case MSG_MBUTTON_UP:
		case MSG_MOUSE_MOVE:
			if (wndList.catchMouse != -1)
			{
				wndId = wndList.catchMouse;
				if (msg.type != MSG_MOUSE_MOVE)
					wndList.catchMouse = -1;
				break;
			}
		case MSG_LBUTTON_DOWN:
		case MSG_RBUTTON_DOWN:
		case MSG_MBUTTON_DOWN:
		case MSG_LBUTTON_DCLK:
		case MSG_RBUTTON_DCLK:
		case MSG_MBUTTON_DCLK:
		case MSG_WHEEL_UP:
		case MSG_WHEEL_DOWN:
		{
			int x = (int)((msg.param >> 16) & 0xffff);
			int y = (int)(msg.param & 0xffff);
			while (wndId != -1)
			{
				if (contain(wndList.data[wndId].rect, x, y))
					break;
				wndId = wndList.data[wndId].next;
			}
			int p = wndId;
			while (p != -1)
			{
				p = wndList.data[p].parentId;
				if (p == -1)
					break;
				if (!contain(wndList.data[p].clientRect, x, y))
					wndId = p;
			}
			if (wndId == -1)
				return;
	}
		if (msg.type == MSG_LBUTTON_DOWN || msg.type == MSG_RBUTTON_DOWN || msg.type == MSG_MBUTTON_DOWN)
		{
			wndList.catchMouse = wndId;
		}
		if (wndId != wndList.entry && msg.type != MSG_MOUSE_MOVE)
		{
			PMessage m;
			m.param = msg.param;
			m.type = MSG_HAS_FOCUS;
			sendMessage(wndId, m);
		}
		if (wndId != wndList.hasMouse)
		{
			PMessage m;
			m.param = msg.param;
			m.type = MSG_MOUSE_OUT;
			sendMessage(wndList.hasMouse, m);
			wndList.hasMouse = wndId;
			m.type = MSG_MOUSE_IN;
			sendMessage(wndId, m);
		}
		break;
	default:
		break;
	}
	int msgQueueId = wndList.data[wndId].msgQueueId;
	PMsgQueue * queue = &wndList.data[msgQueueId].msgQueue;
	msg.wndId = wndId;
	pvcMsgQueueEnQueue(queue, msg);
	wakeup((void *)wndList.data[wndId].pid);
}

int sys_getMessage(void)
{
	PHwnd hwnd = 0;
	if (argstr(0, (char **)&hwnd) < 0)
    return -1;
	int wndId = hwnd->id;
	int msgQueueId = hwnd->msgQueueId;
	int pid = hwnd->pid;

	acquire(&wndList.data[msgQueueId].lock);
	PMsgQueue * queue = &wndList.data[msgQueueId].msgQueue;

	if (queue->head == queue->tail)
		sleep((void *)pid,	&wndList.data[msgQueueId].lock);

	if (wndList.data[wndId].hwnd->msg.type == MSG_NULL)
		wndList.data[wndId].hwnd->msg = pvcMsgQueueDeQueue(queue);

	release(&wndList.data[msgQueueId].lock);
	return 0;
}

void updateTimer()
{
	if (!timerListReady)
		return;
	acquire(&timerList.lock);
	int p = timerList.head;
	while (p != -1)
	{
		++timerList.data[p].curItrvl;
		if (timerList.data[p].curItrvl >= timerList.data[p].interval)
		{
			timerList.data[p].curItrvl = 0;
			PMessage msg;
			msg.type = MSG_TIMEOUT;
			msg.param = (timerList.data[p].id << 16 & 0xffff0000) | (timerList.data[p].interval & 0x0000ffff);
			sendMessage(timerList.data[p].wndId, msg);
		}
		p = timerList.data[p].next;
	}
	release(&timerList.lock);
}

int sys_setTimer(void)
{
	PHwnd hwnd;
	int id;
	int interval;
	if (argstr(0, (char **)&hwnd) < 0 || argint(1, &id) < 0 || argint(2, &interval) < 0)
    return -1;
	pvcTimerListAddToHead(&timerList, hwnd->id, id, interval * TICKS_PER_SEC / 1000);
	return 0;
}

int sys_killTimer(void)
{
	PHwnd hwnd;
	int id;
	if (argstr(0, (char **)&hwnd) < 0 || argint(1, &id) < 0)
    return -1;
	pvcTimerListRemoveId(&timerList, hwnd->id, id);
	return 0;
}

char GBK2312[GBK2312_SIZE];
char ASCII[ASCII_SIZE];

int sys_initStringFigure(void)
{
	char * gbk2312 = 0;
	int n1;
	char * ascii = 0;
	int n2;
	if (argstr(0, (char **)&gbk2312) < 0 || argint(1, &n1) < 0 || argstr(2, (char **)&ascii) < 0 || argint(3, &n2) < 0)
    return -1;
	for (int i = 0; i < n1; i += 32)
	{
		for (int j = 0; j < 16; ++j)
		{
			GBK2312[i + j] = gbk2312[i + 2 * j];
			GBK2312[i + 16 + j] = gbk2312[i + 2 * j + 1];
		}
	}
	memmove(ASCII, ascii, sizeof(char) * n2);
	return 0;
}

static inline int GBK2312Index(ushort c)
{
	int index = c;
	c = index & 0xff;
	c <<= 8;
	index >>= 8;
	c |= index;
	index = 0;
	int d = ((c >> 8) & 0xff) - 0xa1;
	if (d < 0)
		return -1;
	index += d * 94;
	d = (c & 0xff) - 0xa0 - 1;
	if (d < 0)
		return -1;
	index += d;
	return index;
}

int sys_getStringFigure(void)
{
	char * string;
	char * buf;
	int n;
	if (argstr(0, (char **)&string) < 0 || argstr(1, (char **)&buf) < 0 || argint(2, &n) < 0)
    return -1;
	memset(buf, 0, sizeof(char) * n * FONT_SIZE_CY);
	for (int i = 0; i < n; )
	{
		if (string[i] > 0)
		{
			memmove(buf + (i << 4), ASCII + (string[i] << 4), sizeof(char) * 16);
			++i;
			continue;
		}
		else
			cprintf("getStringFigure Error\n");
		int index = GBK2312Index(*((ushort *)(string + i)));
		index <<= 5;
		memmove(buf + (i << 4), GBK2312 + index, sizeof(char) * 32);
		i += 2;
	}
	return 0;
}

int sys_getTime()
{
	uint t = 0;
	outb(0x70, 0x00);
	uchar d = inb(0x71);
	uchar n = (d >> 4) * 10 + (d & 0xf);
	t |= (n & 0xff);
	outb(0x70, 0x02);
	d = inb(0x71);
	n = (d >> 4) * 10 + (d & 0xf);
	t |= ((n & 0xff) << 8);
	outb(0x70, 0x04);
	d = inb(0x71);
	n = (d >> 4) * 10 + (d & 0xf);
	n = (n + 8) % 24;
	t |= (n << 16);
	return t;
}

int sys_getDate()
{
	uint t = 0;
	outb(0x70, 0x00);
	uchar s = inb(0x71);
	t |= (s & 0xff);
	outb(0x70, 0x02);
	uchar m = inb(0x71);
	t |= ((m & 0xff) << 8);
	outb(0x70, 0x04);
	uchar h = inb(0x71);
	t |= (h << 16);
	return t;
}

int sys_setCursor()
{
	PHdc hdc = 0;
	if (argstr(0, (char **)&hdc) < 0)
		return -1;
	for (int i = 0; i < POINTER_SIZE_CY; ++i)
	{
		for (int j = 0; j < POINTER_SIZE_CX; ++j)
		{
			PointerData[i][j] = hdc->content[i * hdc->size.cx + j];
		}
	}
	return 0;
}

int sys_resetWindow()
{
	PHwnd hwnd;
	int wss;
	if (argstr(0, (char**)&hwnd) < 0 || argint(1, (int *)&wss))
		return -1;
	int id = hwnd->id;
	if (wss == WSS_GEOMETRY)
	{
		int x, y, w, h;
		PMessage msg;
		msg.type = MSG_RECTPAINT;

		x = wndList.data[id].rect.x;
		y = wndList.data[id].rect.y;
		w = x + wndList.data[id].rect.w;
		h = y + wndList.data[id].rect.h;
		x /= 10;
		y /= 10;
		w = (w + 9) / 10;
		h = (h + 9) / 10;
		w -= x;
		h -= y;
		msg.param = ((x & 0xff) << 24) | ((y & 0xff) << 16) | ((w & 0xff) << 8) | (h & 0xff);
		acquire(&wndList.lock);
		int p = wndList.data[id].next;
		while (p != -1)
		{
			sendMessage(p, msg);
			p = wndList.data[p].next;
		}
		release(&wndList.lock);

		x = hwnd->pos.x;
		y = hwnd->pos.y;
		w = x + hwnd->wholeDc.size.cx;
		h = y + hwnd->wholeDc.size.cy;
		x /= 10;
		y /= 10;
		w = (w + 9) / 10;
		h = (h + 9) / 10;
		w -= x;
		h -= y;
		msg.param = ((x & 0xff) << 24) | ((y & 0xff) << 16) | ((w & 0xff) << 8) | (h & 0xff);
		sendMessage(id, msg);

		acquire(&wndList.lock);
		wndList.data[id].rect.x = hwnd->pos.x;
		wndList.data[id].rect.y = hwnd->pos.y;
		wndList.data[id].rect.w = hwnd->wholeDc.size.cx;
		wndList.data[id].rect.h = hwnd->wholeDc.size.cy;

		wndList.data[id].clientRect.x = hwnd->clientPos.x + hwnd->pos.x;
		wndList.data[id].clientRect.y = hwnd->clientPos.y + hwnd->pos.y;
		wndList.data[id].clientRect.w = hwnd->dc.size.cx;
		wndList.data[id].clientRect.h = hwnd->dc.size.cy;
		release(&wndList.lock);
	}
	else if (wss == WSS_HAS_FOCUS)
	{
		PMessage m;
		m.type = MSG_LOSE_FOCUS;
		sendMessage(wndList.entry, m);
		wndList.entry = id;
		if (id != wndList.desktop)
		{
			pvcWndListMoveToHead(&wndList, id);
			m.type = MSG_PAINT;
			m.param = WPT_WHOLEDC;
			sendMessage(id, m);
		}
		m.type = MSG_OTHER_WINDOW_ACTION;
		m.param = (OTHER_WINDOW_HAS_FOCUS << 16) | id;
		sendMessage(wndList.desktop, m);
	}
	else if (wss == WSS_LOSE_FOCUS)
	{
		PMessage m;
		m.type = MSG_OTHER_WINDOW_ACTION;
		m.param = (OTHER_WINDOW_LOSE_FOCUS << 16) | id;
		sendMessage(wndList.desktop, m);
	}
	else if (wss == WSS_MINIMIZE)
	{
		PMessage m;
		m.param = WSS_SHOW;
		m.type = MSG_HAS_FOCUS;
		sendMessage(wndList.data[wndList.entry].next, m);
		pvcWndListMoveToTail(&wndList, id);
		pvcFlushWnd(0, 0, videoWidth, videoHeight);
	}
	else if (wss == WSS_SHOW)
	{
		PMessage m;
		m.param = WSS_SHOW;
		m.type = MSG_HAS_FOCUS;
		sendMessage(id, m);
	}
	else if (wss == WSS_BACKGROUND)
	{
		PMessage m;
		m.param = WSS_BACKGROUND;
		acquire(&wndList.lock);
		int p = wndList.desktop;
		while (p != -1)
		{
			if (p == id)
				break;
			m.type = MSG_DIRECT_PAINT;
			sendMessage(p, m);
			p = wndList.data[p].prev;
		}
		release(&wndList.lock);
	}
	return 0;
}

int sys_getWindowInfo(void)
{
	int wndId;
	PWndListNode * node;
	if (argint(0, &wndId) < 0 || argstr(1, (char **)&node))
		return -1;
	acquire(&wndList.lock);
	node->rect = wndList.data[wndId].rect;
	node->clientRect = wndList.data[wndId].clientRect;
	int i;
	for (i = 0; i < wndList.data[wndId].title[i]; ++i)
		node->title[i] = wndList.data[wndId].title[i];
	node->title[i] = '\0';
	node->msgQueueId = wndList.data[wndId].title[i];
	release(&wndList.lock);
	return 0;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void pvcTimerListInit(PTimerList * list)
{
	int i;
	for (i = 0; i < MAX_TIMER_NUM; ++i)
		list->data[i].next = i + 1;
	list->head = -1;
	list->space = 0;
	list->data[i].next = -1;
	initlock(&list->lock, "timerLock");
	timerListReady = 1;
}

void pvcTimerListAddToHead(PTimerList * list, int wndId, int id, int interval)
{
	acquire(&list->lock);
	int p = list->space;
	if (p == -1)
	{
		cprintf("Error! Too much Timer!\n");
		return;
	}
	list->space = list->data[p].next;
	list->data[p].next = list->head;
	list->head = p;
	list->data[p].wndId = wndId;
	list->data[p].id = id;
	list->data[p].interval = interval;
	list->data[p].curItrvl = 0;
	release(&list->lock);
}

void pvcTimerListRemoveWndId(PTimerList * list, int wndId)
{
	acquire(&list->lock);
	int p = list->head;
	int q = p;
	while (p != -1)
	{
		if (list->data[p].wndId == wndId)
		{
			if (p == list->head)
			{
				list->head = list->data[p].next;
				list->data[p].next = list->space;
				list->space = p;
				p = list->head;
				q = p;
			}
			else
			{
				list->data[q].next = list->data[p].next;
				list->data[p].next = list->space;
				list->space = p;
				p = list->data[q].next;
			}
		}
		else
		{
			q = p;
			p = list->data[p].next;
		}
	}
	release(&list->lock);
}

void pvcTimerListRemoveId(PTimerList * list, int wndId, int id)
{
	acquire(&list->lock);
	int p = list->head;
	int q = p;
	while (p != -1)
	{
		if (list->data[p].wndId == wndId && list->data[p].id == id)
		{
			if (p == list->head)
			{
				list->head = list->data[p].next;
				list->data[p].next = list->space;
				list->space = p;
				p = list->head;
				q = p;
			}
			else
			{
				list->data[q].next = list->data[p].next;
				list->data[p].next = list->space;
				list->space = p;
				p = list->data[q].next;
			}
		}
		else
		{
			q = p;
			p = list->data[p].next;
		}
	}
	release(&list->lock);
}

////////////////////////////////////////////////////////////////////////////

void pvcWndListInit(PWndList * list)
{
	int i;
	for (i = 0; i < MAX_WND_NUM; ++i)
	{
		list->data[i].hwnd = 0;
		list->data[i].prev = -1;
		list->data[i].next = i + 1;
		pvcMsgQueueInit(&list->data[i].msgQueue);
		initlock(&list->data[i].lock, "msglock");
	}
	list->data[i - 1].next = -1;
	initlock(&list->lock, "wndListLock");
	list->head = list->tail = list->desktop = -1;
	list->hasMouse = list->catchMouse = -1;
	list->space = 0;
}

void pvcWndListAddToHead(PWndList * list, PHwnd hwnd)
{
	acquire(&list->lock);
	int p = list->space;
	if (p == -1)
	{
		cprintf("too much window\n");
		return;
	}
	list->space = list->data[list->space].next;
	list->data[p].hwnd = hwnd;
	hwnd->id = p;
	list->data[p].pid = hwnd->pid;
	if (hwnd->msgQueueId >= 0)
		list->data[p].msgQueueId = hwnd->msgQueueId;
	else
		list->data[p].msgQueueId = hwnd->msgQueueId = p;

	list->data[p].rect.x = hwnd->pos.x;
	list->data[p].rect.y = hwnd->pos.y;
	list->data[p].rect.w = hwnd->wholeDc.size.cx;
	list->data[p].rect.h = hwnd->wholeDc.size.cy;

	list->data[p].clientRect.x = hwnd->clientPos.x + hwnd->pos.x;
	list->data[p].clientRect.y = hwnd->clientPos.y + hwnd->pos.y;
	list->data[p].clientRect.w = hwnd->dc.size.cx;
	list->data[p].clientRect.h = hwnd->dc.size.cy;
	if (hwnd->parentId < 0)
		hwnd->parentId = list->desktop;
	list->data[p].parentId = hwnd->parentId;

	int i;
	for (i = 0; hwnd->title[i]; ++i)
		list->data[p].title[i] = hwnd->title[i];
	list->data[p].title[i] = '\0';

	list->data[p].next = list->head;
	list->data[p].prev = -1;
	if (list->head == -1)
	{
		list->tail = p;
	}
	else
	{
		list->data[list->head].prev = p;
	}
	if (list->desktop == -1)
		list->hasMouse = list->desktop = p;
	else
	{
		PMessage msg;
		msg.type = MSG_OTHER_WINDOW_ACTION;
		msg.param = (OTHER_WINDOW_OPEN << 16) | p;
		sendMessage(list->desktop, msg);
	}
	PMessage msg;
	msg.type = MSG_LOSE_FOCUS;
	sendMessage(list->entry, msg);
	list->head = p;
	release(&list->lock);
}

void pvcWndListMoveToHead(PWndList * list, int wndId)
{
	if (wndId < 0)
		return;
	acquire(&list->lock);
	if (wndId != list->head)
	{
		list->data[list->data[wndId].prev].next = list->data[wndId].next;
		if (list->data[wndId].next == -1)
		{
			list->tail = list->data[wndId].prev;
		}
		else
		{
			list->data[list->data[wndId].next].prev = list->data[wndId].prev;
		}
		list->data[list->head].prev = wndId;
		list->data[wndId].prev = -1;
		list->data[wndId].next = list->head;
		list->head = wndId;
	}
	release(&list->lock);
}

void pvcWndListMoveToTail(PWndList * list, int wndId)
{
	if (wndId < 0)
		return;
	acquire(&list->lock);
	if (wndId != list->tail)
	{
		list->data[list->data[wndId].next].prev = list->data[wndId].prev;
		if (list->data[wndId].prev == -1)
		{
			list->head = list->data[wndId].next;
		}
		else
		{
			list->data[list->data[wndId].prev].next = list->data[wndId].next;
		}
		list->data[list->tail].next = wndId;
		list->data[wndId].next = -1;
		list->data[wndId].prev = list->tail;
		list->tail = wndId;
	}
	release(&list->lock);
}

void pvcWndListRemove(PWndList * list, int wndId)
{
	if (wndId < 0)
		return;
	acquire(&list->lock);
	if (wndId == list->head)
	{
		if (list->data[wndId].next == -1)
		{
				list->head = list->tail = -1;
		}
		else
		{
			list->head = list->data[list->head].next;
			list->data[list->head].prev = -1;
		}
	}
	else if (wndId == list->tail)
	{
		list->tail = list->data[list->tail].prev;
		list->data[list->tail].next = -1;
	}
	else
	{
		list->data[list->data[wndId].prev].next = list->data[wndId].next;
		list->data[list->data[wndId].next].prev = list->data[wndId].prev;
	}
	list->data[wndId].prev = -1;
	list->data[wndId].next = list->space;
	list->data[wndId].hwnd = 0;
	list->space = wndId;
	PMessage msg;
	msg.type = MSG_OTHER_WINDOW_ACTION;
	msg.param = (OTHER_WINDOW_CLOSE << 16) | (wndId & 0xffff);
	sendMessage(list->desktop, msg);
	pvcMsgQueueInit(&list->data[wndId].msgQueue);
	release(&list->lock);
}

void pvcWndListDestroy(PWndList * list)
{
	acquire(&list->lock);
	release(&list->lock);
}

////////////////////////////////////////////////////////////////////////////

void pvcMsgQueueInit(PMsgQueue * queue)
{
	queue->head = queue->tail = 0;
}

void pvcMsgQueueEnQueue(PMsgQueue * queue, PMessage msg)
{
	if ((queue->tail + 1) % MESSAGE_QUEUE_SIZE == queue->head)
	{
		cprintf("Error! Message Queue is full\n");
		return;
	}
	switch (msg.type)
	{
	case MSG_MOUSE_MOVE:
	case MSG_PAINT:
	case MSG_RECTPAINT:
	case MSG_TIMEOUT:
	{
		int p = queue->head;
		while (p != queue->tail)
		{
			if (queue->data[p].type == msg.type && queue->data[p].wndId == msg.wndId)
			{
				if (msg.type == MSG_MOUSE_MOVE)
				{
					queue->data[p].param = msg.param;
					return;
				}
				else if (msg.type == MSG_RECTPAINT)
				{
					int x1 = queue->data[p].param >> 24;
					int y1 = (queue->data[p].param >> 16) & 0xff;
					int w1 = (queue->data[p].param >> 8) & 0xff;
					w1 += x1;
					int h1 = queue->data[p].param & 0xff;
					h1 += y1;
					int x2 = msg.param >> 24;
					int y2 = (msg.param >> 16) & 0xff;
					int w2 = (msg.param >> 8) & 0xff;
					w2 += x2;
					int h2 = msg.param & 0xff;
					h2 += y2;
					x1 = x1 < x2 ? x1 : x2;
					y1 = y1 < y2 ? y1 : y2;
					w1 = w1 > w2 ? w1 : w2;
					h1 = h1 > h2 ? h1 : h2;
					w1 -= x1;
					h1 -= y1;
					queue->data[p].param = (x1 << 24) | ((y1 & 0xff) << 16) | ((w1 & 0xff) << 8) | (h1 & 0xff);
					return;
				}
				else if ((msg.type == MSG_PAINT || msg.type == MSG_TIMEOUT) && msg.param == queue->data[p].param)
					return;
			}
			p = (p + 1) % MESSAGE_QUEUE_SIZE;
		}
	}
		break;
	default:
		break;
	}
	queue->data[queue->tail] = msg;
	queue->tail = (queue->tail + 1) % MESSAGE_QUEUE_SIZE;
}

PMessage pvcMsgQueueDeQueue(PMsgQueue * queue)
{
	if (queue->head == queue->tail)
	{
		PMessage msg;
		msg.type = MSG_NULL;
		return msg;
	}
	int p = queue->head;
	queue->head = (queue->head + 1) % MESSAGE_QUEUE_SIZE;
	return queue->data[p];
}

////////////////////////////////////////////////////////////////////////////
