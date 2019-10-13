#include "types.h"
#include "user.h"
#include "PVCWindow.h"
#include "PVCPainter.h"
#include "PVCControl.h"

PHwnd pvcCreateWindow(char * title, PHwnd parent, int x, int y, int width, int height)
{
  PHwnd r = (PHwnd)malloc(sizeof(PWindow));
  if (r == 0)
    pvcError(0);

  strcpy(r->title, title);

  r->pos.x = x;
  r->pos.y = y;

  r->wholeDc.size.cx = width;
  r->wholeDc.size.cy = height;
  r->wholeDc.content = (PColor *)malloc(sizeof(PColor) * r->wholeDc.size.cx * r->wholeDc.size.cy);
  if (r->wholeDc.content == 0)
    pvcError(0);
  memset(r->wholeDc.content, 0x0c, sizeof(PColor) * r->wholeDc.size.cx * r->wholeDc.size.cy);

  r->clientPos.x = WND_EDGE_SIZE;
  r->clientPos.y = WND_TITLE_HEIGHT;
  r->dc.size.cx = width - WND_EDGE_SIZE * 2;
  r->dc.size.cy = height - WND_TITLE_HEIGHT - WND_EDGE_SIZE;
  r->dc.content = (PColor *)malloc(sizeof(PColor) * r->dc.size.cx * r->dc.size.cy);
  if (r->dc.content == 0)
    pvcError(0);
  memset(r->dc.content, 0x0, sizeof(PColor) * r->dc.size.cx * r->dc.size.cy);

  r->msg.type = MSG_NULL;
  r->state = 0;
  r->pid = getpid();
  r->msgQueueId = -1;

  if (parent)
    r->parentId = parent->id;
  else
    r->parentId = -1;

  r->focusState = WFS_LOSE_FOCUS;
  r->childFocusId = -1;

  pvcCtrlLstInit(&r->ctrlLst);

  return r;
}

bool pvcWndPreTranslateMessage(PHwnd hwnd, PMessage * msg)
{
  if (msg->wndId != hwnd->id)
  {
    msg->wndId = msg->type;
    msg->type = MSG_WNDID_WRONG;
    return SUBMIT;
  }
  int x = (msg->param >> 16) & 0xffff;
  int y = msg->param & 0xffff;
  switch (msg->type)
  {
  case MSG_RECTPAINT:
  {
    int x = ((msg->param >> 24) & 0xff) * 10;
    int y = ((msg->param >> 16) & 0xff) * 10;
    int w = ((msg->param >> 8) & 0xff) * 10;
    int h = (msg->param & 0xff) * 10;
    x -= hwnd->pos.x;
    y -= hwnd->pos.y;
    w += x;
    h += y;
    if (x < 0)
      x = 0;
    if (y < 0)
      y = 0;
    if (w > hwnd->wholeDc.size.cx)
      w = hwnd->wholeDc.size.cx;
    if (h > hwnd->wholeDc.size.cy)
      h = hwnd->wholeDc.size.cy;
    w -= x;
    h -= y;
    //printf(1, "%s %d %d %d %d\n", hwnd->title, x, y, w, h);
    paintWindow(hwnd, x, y, &hwnd->wholeDc, x, y, w, h);
  }
    return FINISH;
  case MSG_DIRECT_PAINT:
    //printf(1, "directPaintWindow %d\n", hwnd->id);
    directPaintWindow(hwnd, 0, 0, &hwnd->wholeDc, 0, 0, hwnd->wholeDc.size.cx, hwnd->wholeDc.size.cy);
    return FINISH;
  case MSG_LBUTTON_UP:
  case MSG_LBUTTON_DOWN:
  case MSG_LBUTTON_DCLK:
  case MSG_RBUTTON_UP:
  case MSG_RBUTTON_DOWN:
  case MSG_RBUTTON_DCLK:
  case MSG_MBUTTON_UP:
  case MSG_MBUTTON_DOWN:
  case MSG_MBUTTON_DCLK:
  case MSG_WHEEL_UP:
  case MSG_WHEEL_DOWN:
  case MSG_MOUSE_MOVE:
    x -= hwnd->pos.x;
    if (x < 0)
      x = 0;
    else if (x >= hwnd->wholeDc.size.cx)
      x = hwnd->wholeDc.size.cx - 1;
    y -= hwnd->pos.y;
    if (y < 0)
      y = 0;
    else if (y >= hwnd->wholeDc.size.cy)
      y = hwnd->wholeDc.size.cy - 1;
    msg->param = ((x << 16) & 0xffff0000) | (y & 0xffff);
    break;
  case MSG_LOSE_FOCUS:
    if (hwnd->focusState == WFS_LOSE_FOCUS)
      return FINISH;
    hwnd->focusState = WFS_LOSE_FOCUS;
    resetWindow(hwnd, WSS_LOSE_FOCUS);
    break;
  case MSG_HAS_FOCUS:
    if (hwnd->focusState == WFS_HAS_FOCUS)
      return FINISH;
    hwnd->focusState = WFS_HAS_FOCUS;
    resetWindow(hwnd, WSS_HAS_FOCUS);
    break;
  default:
    break;
  }
  if (msg->type == MSG_MOUSE_MOVE && hwnd->state & WS_NONCLIENT_BIT)
  {
    x += hwnd->pos.x;
    y += hwnd->pos.y;
    hwnd->pos.x += x - hwnd->cursor.x;
    hwnd->pos.y += y - hwnd->cursor.y;
    hwnd->cursor.x = x;
    hwnd->cursor.y = y;
    resetWindow(hwnd, WSS_GEOMETRY);
    return FINISH;
  }
  return SUBMIT;
}

bool pvcWndTranslateMessage(PHwnd hwnd, PMessage * msg)
{
  int x = (msg->param >> 16) & 0xffff;
  int y = msg->param & 0xffff;
  switch (msg->type)
  {
  case MSG_LBUTTON_UP:
    if (!(x > hwnd->clientPos.x && x < hwnd->clientPos.x + hwnd->dc.size.cx &&
       y > hwnd->clientPos.y && y < hwnd->clientPos.y + hwnd->dc.size.cy))
    {
      msg->type = MSG_NC_LBUTTON_UP;
    }
    break;
  case MSG_LBUTTON_DOWN:
    if (!(x > hwnd->clientPos.x && x < hwnd->clientPos.x + hwnd->dc.size.cx &&
       y > hwnd->clientPos.y && y < hwnd->clientPos.y + hwnd->dc.size.cy))
    {
      msg->type = MSG_NC_LBUTTON_DOWN;
    }
    break;
  case MSG_MOUSE_MOVE:
    break;
  default:
    break;
  }
  return SUBMIT;
}

void pvcSendMessage(PHwnd hwnd, PMessage msg)
{
  sendMessage(hwnd->id, &msg);
}

void pvcCloseWindow(void * hwnd)
{
  PMessage msg;
  msg.type = MSG_DESTROY;
  pvcSendMessage((PHwnd)hwnd, msg);
}

void pvcMinimizeWindow(void * hwnd)
{
  resetWindow(hwnd, WSS_MINIMIZE);
}

void pvcRefresh(void * param)
{
  flushRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

bool pvcWndProc(PHwnd hwnd, PMessage msg)
{
  PMessage m;
  switch (msg.type)
  {
  case MSG_CREATE:
    pvcCreateButton(hwnd, 0, WND_EDGE_SIZE, WND_EDGE_SIZE, WND_TITLE_HEIGHT - 2 * WND_EDGE_SIZE, WND_TITLE_HEIGHT - 2 * WND_EDGE_SIZE,
    (void *)hwnd, pvcCloseWindow, pvcPaintCloseBtn);
    pvcCreateButton(hwnd, 0, WND_EDGE_SIZE + WND_TITLE_HEIGHT - WND_EDGE_SIZE, WND_EDGE_SIZE, WND_TITLE_HEIGHT - 2 * WND_EDGE_SIZE, WND_TITLE_HEIGHT - 2 * WND_EDGE_SIZE,
    (void *)hwnd, pvcMinimizeWindow, pvcPaintMinimizeBtn);
    pvcDrawWindowFrame(hwnd);
    break;
  case MSG_PAINT:
    if (msg.param == WPT_WHOLEDC)
    {
      pvcBltbit(&hwnd->wholeDc, hwnd->clientPos.x, hwnd->clientPos.y, &hwnd->dc, 0, 0, hwnd->dc.size.cx, hwnd->dc.size.cy);
      paintWindow(hwnd, 0, 0, &hwnd->wholeDc, 0, 0, hwnd->wholeDc.size.cx, hwnd->wholeDc.size.cy);
    }
    else if (msg.param == WPT_DC)
    {
      pvcBltbit(&hwnd->wholeDc, hwnd->clientPos.x, hwnd->clientPos.y, &hwnd->dc, 0, 0, hwnd->dc.size.cx, hwnd->dc.size.cy);
      paintWindow(hwnd, hwnd->clientPos.x, hwnd->clientPos.y, &hwnd->dc, 0, 0, hwnd->dc.size.cx, hwnd->dc.size.cy);
    }
    else if (msg.param == WPT_DIRECT)
      paintWindow(hwnd, hwnd->clientPos.x, hwnd->clientPos.y, &hwnd->dc, 0, 0, hwnd->dc.size.cx, hwnd->dc.size.cy);
    PControlListNode * p = hwnd->ctrlLst.head;
    while (p)
    {
      for (int i = 0; i < p->ctrl->dc.size.cy; ++i)
      {
        for (int j = 0; j < p->ctrl->dc.size.cx; ++j)
        {
          p->ctrl->background[i * p->ctrl->dc.size.cx + j] =
            pvcPixel(&hwnd->wholeDc, p->ctrl->pos.x + j, p->ctrl->pos.y + i);
        }
      }
      p->ctrl->ctrlProc(p->ctrl, msg);
      p = p->next;
    }
    break;
  case MSG_DESTROY:
    pvcDestroyWindow(hwnd);
    return SUBMIT;
  case MSG_KEY_DOWN:
    if (msg.param == VK_ESC)
    {
      PMessage msg;
      msg.type = MSG_DESTROY;
      pvcSendMessage(hwnd, msg);
    }
    else if (msg.param == VK_TAB)
    {
      hwnd->ctrlLst.entry = hwnd->ctrlLst.head;
      PMessage t;
      t.type = MSG_HAS_FOCUS;
      t.param = msg.type;
      hwnd->ctrlLst.entry->ctrl->ctrlProc(hwnd->ctrlLst.entry->ctrl, t);
    }
    else if (msg.param == VK_F5)
      pvcRefresh(0);
    else if (msg.param == VK_F4)
      pvcMinimizeWindow(hwnd);
    break;
  case MSG_NC_LBUTTON_DOWN:
    hwnd->cursor.x = ((msg.param >> 16) & 0xffff) + hwnd->pos.x;
    hwnd->cursor.y = (msg.param & 0xffff) + hwnd->pos.y;
    hwnd->state |= WS_NONCLIENT_BIT;
    break;
  case MSG_NC_LBUTTON_UP:
    hwnd->state &= ~WS_NONCLIENT_BIT;
    break;
  case MSG_LOSE_FOCUS:
  case MSG_HAS_FOCUS:
    pvcDrawWindowFrame(hwnd);
    m.type = MSG_PAINT;
    m.param = WPT_WHOLEDC;
    pvcSendMessage(hwnd, m);
    break;
  case MSG_STATECHAGE:
  {
    switch (msg.param)
    {
    case WSS_MINIMIZE:
    case WSS_SHOW:
      resetWindow(hwnd, msg.param);
      break;
    }
  }
    break;
  default:
    break;
  }
  return FINISH;
}

void pvcWndExec(PHwnd hwnd, bool (*wndProc)(PHwnd, PMessage))
{
  hwnd->wndProc = wndProc;
  registWindow(hwnd);
  PMessage msg;
  msg.type = MSG_CREATE;
  pvcSendMessage(hwnd, msg);
  /*msg.type = MSG_PAINT;
  msg.param = WPT_WHOLEDC;
  pvcSendMessage(hwnd, msg);*/
  msg.type = MSG_HAS_FOCUS;
  msg.param = MSG_CREATE;
  pvcSendMessage(hwnd, msg);
  while (1)
  {
    getMessage(hwnd);
    if (pvcWndPreTranslateMessage(hwnd, &hwnd->msg))
      if (pvcDispatchMsgToCtrlLst(&hwnd->ctrlLst, hwnd->msg))
        if (pvcWndTranslateMessage(hwnd, &hwnd->msg))
          if (wndProc(hwnd, hwnd->msg))
          {
            break;
          }
    hwnd->msg.type = MSG_NULL;
  }
}

void pvcDestroyWindow(void * param)
{
  PHwnd hwnd = (PHwnd)param;
  destroyWindow(hwnd);
  free(hwnd->dc.content);
  free(hwnd->wholeDc.content);
  free(hwnd);
}

void pvcDrawWindowFrame(PHwnd hwnd)
{
  PHdc wHdc = pvcGetWholeDC(hwnd);

  wHdc->pen.size = WND_EDGE_SIZE;
  wHdc->pen.color = RGB(222, 222, 222);
  wHdc->brush.color = COLOR_NULL;
  pvcDrawRect(wHdc, (WND_EDGE_SIZE + 1) / 2, (WND_EDGE_SIZE + 1) / 2, wHdc->size.cx - WND_EDGE_SIZE, wHdc->size.cy - WND_EDGE_SIZE);
  wHdc->pen.color = COLOR_NULL;
  wHdc->pen.size = 0;
  if (hwnd->focusState == WFS_HAS_FOCUS)
    wHdc->brush.color = RGB(225, 225, 225);
  else if (hwnd->focusState == WFS_LOSE_FOCUS)
    wHdc->brush.color = RGB(185, 185, 185);
  pvcDrawRect(wHdc, (WND_EDGE_SIZE + 1) / 2, (WND_EDGE_SIZE + 1) / 2, wHdc->size.cx - WND_EDGE_SIZE, WND_TITLE_HEIGHT - WND_EDGE_SIZE + 2);

  wHdc->font.color = RGB(85, 85, 85);
  int s = strlen(hwnd->title);
  s *= FONT_SIZE_CX;
  pvcDrawText(wHdc, hwnd->title, (wHdc->size.cx - s) / 2, (WND_TITLE_HEIGHT - FONT_SIZE_CY) / 2);
}

void pvcInvalidate(PHwnd hwnd)
{
  memset(hwnd->dc.content, 0x0, sizeof(PColor) * hwnd->dc.size.cx * hwnd->dc.size.cy);
  PMessage msg;
  msg.type = MSG_PAINT;
  msg.param = WPT_DC;
  pvcSendMessage(hwnd, msg);
}

void pvcUpdate(PHwnd hwnd)
{
  PMessage msg;
  msg.type = MSG_PAINT;
  msg.param = WPT_DC;
  pvcSendMessage(hwnd, msg);
}

void pvcPaintCloseBtn(PButtonData * data, PHdc hdc)
{
  hdc->pen.size = 0;
  hdc->pen.color = COLOR_NULL;
  switch (data->state)
  {
  case BS_MOUSE_OUT:
    hdc->brush.color = RGB(180, 0, 0);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    break;
  case BS_MOUSE_IN:
    hdc->brush.color = RGB(255, 0, 0);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    break;
  case BS_BUTTON_DOWN:
    hdc->brush.color = RGB(125, 0, 0);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    break;
  }
  hdc->pen.size = 2;
  hdc->pen.color = RGB(0, 0, 0);
  pvcDrawLine(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
  pvcDrawLine(hdc, hdc->size.cx, 0, 0, hdc->size.cy);
}

void pvcPaintMinimizeBtn(PButtonData * data, PHdc hdc)
{
  hdc->pen.size = 0;
  hdc->pen.color = COLOR_NULL;
  switch (data->state)
  {
  case BS_MOUSE_OUT:
    hdc->brush.color = RGB(180, 180, 180);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    break;
  case BS_MOUSE_IN:
    hdc->brush.color = RGB(255, 255, 255);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    break;
  case BS_BUTTON_DOWN:
    hdc->brush.color = RGB(125, 125, 125);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    break;
  }
  hdc->pen.size = 2;
  hdc->pen.color = RGB(0, 0, 0);
  pvcDrawLine(hdc, 0, hdc->size.cy / 2, hdc->size.cx, hdc->size.cy / 2);
}

void runApp(void * param)
{
	int pid = fork();
	char temp[30];
	if (pid == 0)
	{
		sprintf(temp, "PVC%s", (char *)param);
		printf(2, "%s\n", param);
		char *argv[] = { temp, 0 };
		exec(temp, argv);
	}
}

void runAppWithCmds(void * param, char** cmds)
{
	int pid = fork();
	char temp[30];
	if (pid == 0)
	{
		sprintf(temp, "PVC%s", (char *)param);
		char *argv[MAX_ARGUMENT] = { temp, 0 };
		int i = 0;
		for (; cmds[i] != 0; i++)
		{
			argv[i + 1] = cmds[i];
		}
		argv[i + 1] = 0;
		exec(temp, argv);
	}
}
