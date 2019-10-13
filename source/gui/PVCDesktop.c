#include "PVCDesktop.h"

//#define BITMAP

void desktopInitStringFigure()
{
  /*printf(1, "init GBK2312\n");
  int fd = open("GBK2312", O_RDONLY);
  if (fd < 0)
  {
    printf(1, "Cannot open file\n");
    return;
  }
  char * GBK2312 = (char *)malloc(sizeof(char) * GBK2312_SIZE);
  read(fd, GBK2312, sizeof(char) * GBK2312_SIZE);
  close(fd);
  printf(1, "read GBK2312 complete\n");*/

  printf(1, "init ASCII\n");
  int fd = open("ASCII", O_RDONLY);
  if (fd < 0)
  {
    printf(1, "Cannot open file\n");
    return;
  }
  char * ASCII = (char *)malloc(sizeof(char) * ASCII_SIZE);
  read(fd, ASCII, sizeof(char) * ASCII_SIZE);
  close(fd);
  printf(1, "read ASCII complete\n");

  initStringFigure(0, 0, ASCII, ASCII_SIZE);
  //free(GBK2312);
  free(ASCII);
  printf(1, "init string figure complete\n");
}

PHControl desktopCreateItem(PHwnd hwnd, int x, int y, int w, int h, void * param)
{
  PHControl item = (PHControl)pvcCreateControl(hwnd, x, y, w, h, 0, desktopItemProc);
  PItemData * data = (PItemData *)malloc(sizeof(PItemData));
  data->state = BS_MOUSE_OUT;
  char buf[50];
  sprintf(buf, "%s.bmp", (char*)param);
  PBitmap bmp = pvcLoadBitmap(buf);
  data->hdc = pvcCreateCompatibleDCFromBitmap(bmp);
  data->param = param;
  item->data = (void *)data;
  return item;
}

bool desktopItemProc(PHControl item, PMessage msg)
{
  PItemData * data = (PItemData *)item->data;
  switch (msg.type)
  {
  case MSG_PAINT:
    memmove(item->dc.content, item->background, sizeof(PColor) * item->dc.size.cx * item->dc.size.cy);
    desktopPaintItem(data, &item->dc);
    break;
  case MSG_MOUSE_OUT:
  case MSG_LOSE_FOCUS:
    if (data->state != BS_MOUSE_OUT)
    {
      data->state = BS_MOUSE_OUT;
      pvcCtrlInvalidate(item);
    }
    return FINISH;
  case MSG_MOUSE_MOVE:
  case MSG_MOUSE_IN:
  case MSG_HAS_FOCUS:
    if (data->state == BS_MOUSE_OUT)
    {
      data->state = BS_MOUSE_IN;
      pvcCtrlInvalidate(item);
    }
    return SUBMIT;
  case MSG_LBUTTON_DCLK:
    data->state = BS_MOUSE_OUT;
    runApp(data->param);
    pvcCtrlInvalidate(item);
    return FINISH;
  default:
    break;
  }
  return pvcCtrlProc(item, msg);
}

void desktopPaintItem(PItemData * data, PHdc hdc)
{
  /*hdc->pen.size = 0;
  hdc->pen.color = COLOR_NULL;
  hdc->brush.color = RGB(235, 245, 245);
  pvcDrawRect(hdc, 15, 16, 60, 48);
  hdc->brush.color = RGB(65, 65, 65);
  pvcDrawRect(hdc, 17, 19, 56, 44);
  hdc->pen.size = 2;
  hdc->pen.color = RGB(235, 245, 245);
  pvcDrawLine(hdc, 23, 25, 23, 45);
  pvcDrawLine(hdc, 23, 25, 33, 25);
  pvcDrawLine(hdc, 33, 25, 33, 33);
  pvcDrawLine(hdc, 23, 33, 33, 33);
  pvcDrawLine(hdc, 39, 25, 44, 45);
  pvcDrawLine(hdc, 49, 25, 44, 45);
  pvcDrawLine(hdc, 54, 25, 65, 25);
  pvcDrawLine(hdc, 54, 25, 54, 45);
  pvcDrawLine(hdc, 54, 45, 65, 45);*/
  pvcTransparentBit(hdc, (90 - data->hdc->size.cx) / 2, (70 - data->hdc->size.cy) / 2, data->hdc,
    0, 0, data->hdc->size.cx, data->hdc->size.cy, RGB(255, 255, 255));
  switch (data->state)
  {
  case BS_MOUSE_OUT:
    break;
  case BS_MOUSE_IN:
    for (int i = 0; i < hdc->size.cy; ++i)
    {
      for (int j = 0; j < hdc->size.cx; ++j)
      {
        PColor col = hdc->content[i * hdc->size.cx + j];
		// 
        int t = (int)col.b + 50;
		if (t > 255)
		{
			t = 255;
		}
		col.b = t;

        t = (int)col.r - 50;
		if (t < 0)
		{
			t = 0;
		}
		col.r = t;

        t = (int)col.g - 50;
		if (t < 0)
		{
			t = 0;
		}
		col.g = t;
        hdc->content[i * hdc->size.cx + j] = col;
      }
    }
    break;
  default:
    break;
  }
  int x = (90 - (strlen((char *)data->param)) * FONT_SIZE_CX) / 2;
  int y = (20 - FONT_SIZE_CY) / 2 + 70;
  //printf(1, "%s invalidate\n", (char *)data->param);
  hdc->font.color = RGB(0, 0, 0);
  pvcDrawText(hdc, (char *)data->param, x, y);
  /*hdc->pen.size = 0;
  hdc->pen.color = COLOR_NULL;
  hdc->brush.color = RGB(255, 155, 255);
  pvcDrawRect(hdc, 10, 10, 30, 6);
  pvcDrawRect(hdc, 10, 16, 60, 6);
  hdc->brush.color = RGB(200, 165, 180);
  pvcDrawRect(hdc, 10, 22, 60, 48);
  pvcDrawRect(hdc, 15, 12, 20, 2);
  switch (data->state)
  {
  case BS_MOUSE_OUT:
    break;
  case BS_MOUSE_IN:
    for (int i = 0; i < hdc->size.cy; ++i)
    {
      for (int j = 0; j < hdc->size.cx; ++j)
      {
        PColor c = hdc->content[i * hdc->size.cx + j];
        int t = (int)c.b + 100;
        if (t > 255)
          t = 255;
        c.b = t;
        t = (int)c.r - 100;
        if (t < 0)
          t = 0;
        c.r = t;
        t = (int)c.g - 100;
        if (t < 0)
          t = 0;
        c.g = t;
        hdc->content[i * hdc->size.cx + j] = c;
      }
    }
    break;
  case BS_BUTTON_DOWN:
    for (int i = 0; i < hdc->size.cy; ++i)
    {
      for (int j = 0; j < hdc->size.cx; ++j)
      {
        PColor c = hdc->content[i * hdc->size.cx + j];
        int t = (int)c.b + 180;
        if (t > 255)
          t = 255;
        c.b = t;
        t = (int)c.r - 180;
        if (t < 0)
          t = 0;
        c.r = t;
        t = (int)c.g - 180;
        if (t < 0)
          t = 0;
        c.g = t;
        hdc->content[i * hdc->size.cx + j] = c;
      }
    }
    break;
  default:
    break;
  }
  int x = (80 - (strlen((char *)data->param)) * FONT_SIZE_CX) / 2;
  int y = (20 - FONT_SIZE_CY) / 2 + 70;
  //printf(1, "%s invalidate\n", (char *)data->param);
  hdc->font.color = RGB(0, 0, 0);
  pvcDrawText(hdc, (char *)data->param, x, y);*/
}

char* desktopFmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  buf[strlen(p)] = 0;
  return buf;
}

void desktopLs(char *path, PHwnd hwnd)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(1, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(1, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  int index = 0;

  switch(st.type){
  case T_FILE:
    printf(1, "%s %d %d %d\n", desktopFmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      int l = strlen(desktopFmtname(buf));
      char * temp = desktopFmtname(buf);
      if (l > 3 && temp[0] == 'P' && temp[1] == 'V' && temp[2] == 'C' && strcmp(temp, "PVCDesktop") != 0)
      {
        temp += 3;
        char * title = (char *)malloc(sizeof(char) * (l - 3));
        strcpy(title, temp);
        desktopCreateItem(hwnd, 10 + (index / 5) * 90 , 10 + (index % 5) * 90, 90, 90, title);
        ++index;
      }
    }
    break;
  }
  close(fd);
}

bool taskBarProc(PHControl taskBar, PMessage msg)
{
  TaskBarData * data = (TaskBarData *)taskBar->data;
  switch (msg.type)
  {
  case MSG_PAINT:
  {
    PHdc hdc = &taskBar->dc;
    //pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    memmove(hdc->content, taskBar->background, sizeof(PColor) * hdc->size.cx * hdc->size.cy);
    if (data->num == 0)
      break;
    int width = TASKBAR_WIDTH / data->num;
    if (width > TASKBAR_ITEM_MAX_WIDTH)
      width = TASKBAR_ITEM_MAX_WIDTH;
    TaskBarItem * p = data->head;
    int sx = 0;
    const int d = 4;
    int textLen = (width - d * 4) / FONT_SIZE_CX;
    hdc->font.color = RGB(65, 65, 85);
    while (p)
    {
      if (p == data->select)
        hdc->brush.color = RGB(225, 225, 235);
      else if (p == data->focus)
        hdc->brush.color = RGB(235, 245, 245);
      else
        hdc->brush.color = RGB(225, 235, 225);

      if (p == data->mouse)
      {
        hdc->pen.size = 3;
        hdc->pen.color = RGB(225, 225, 235);
      }
      else
      {
        hdc->pen.size = 0;
        hdc->pen.color = COLOR_NULL;
      }

      pvcDrawRect(hdc, sx + d, d, width - 2 * d, hdc->size.cy - 2 * d);
      char ch = p->title[textLen];
      p->title[textLen] = '\0';
      pvcDrawText(hdc, p->title, sx + d * 2, (hdc->size.cy - FONT_SIZE_CY) / 2);
      p->title[textLen] = ch;
      sx += width;
      p = p->next;
    }
  }
    break;
  case MSG_MOUSE_MOVE:
  case MSG_LBUTTON_DOWN:
  case MSG_LBUTTON_UP:
  {
    if (data->num == 0)
      break;
    int width = TASKBAR_WIDTH / data->num;
    if (width > TASKBAR_ITEM_MAX_WIDTH)
      width = TASKBAR_ITEM_MAX_WIDTH;
    TaskBarItem * p = data->head;
    int x = (msg.param >> 16) & 0xffff;
    while (p)
    {
      if (x < width)
      {
        if (msg.type == MSG_LBUTTON_UP)
        {
          if (p == data->select)
          {
            PMessage m;
            m.type = MSG_STATECHAGE;
            m.param = WSS_SHOW;
            sendMessage(p->wndId, &m);
          }
          data->select = 0;
        }
        else if (msg.type == MSG_LBUTTON_DOWN)
          data->select = p;
        else if (msg.type == MSG_MOUSE_MOVE)
          data->mouse = p;
        pvcCtrlInvalidate(taskBar);
        return SUBMIT;
      }
      x -= width;
      p = p->next;
    }
  }
    return SUBMIT;
  case MSG_MOUSE_OUT:
  case MSG_LOSE_FOCUS:
    data->select = data->mouse = 0;
    return FINISH;
  }
  return pvcCtrlProc(taskBar, msg);
}

bool wndProc(PHwnd hwnd, PMessage msg)
{
  static PDc timeDc;
  #ifdef BITMAP
    static PHdc backgroundDC;
  #endif
  switch (msg.type)
  {
  case MSG_NC_LBUTTON_DOWN:
    printf(1, "Desktop MSG_NC_LBUTTON_DOWN\n");
  case MSG_NC_LBUTTON_UP:
    return FINISH;
  case MSG_HAS_FOCUS:
  case MSG_LOSE_FOCUS:
    return FINISH;
  case MSG_CREATE:
  {
    #ifdef BITMAP
    PBitmap bmp = pvcLoadBitmap("desktop.bmp");
    backgroundDC = pvcCreateCompatibleDCFromBitmap(bmp);
    #endif
    taskBar = pvcCreateControl(hwnd, 0, hwnd->dc.size.cy, TASKBAR_WIDTH,
       hwnd->wholeDc.size.cy - hwnd->dc.size.cy, &taskBarData, taskBarProc);
    timeDc.content = (PColor *)malloc(sizeof(PColor) * 8 * 8 * 16);
    timeDc.font.color = RGB(68, 64, 63);
    timeDc.pen.color = COLOR_NULL;
    timeDc.pen.size = 0;
    timeDc.brush.color = RGB(205, 205, 215);
    timeDc.size.cx = 8 * 8;
    timeDc.size.cy = 16;
    desktopLs(".", hwnd);
    setTimer(hwnd, 1, 500);
    hwnd->wholeDc.pen.size = 0;
    hwnd->wholeDc.pen.color = COLOR_NULL;
    hwnd->wholeDc.brush.color = timeDc.brush.color;
    pvcDrawRect(&hwnd->wholeDc, 0, SCREEN_HEIGHT - WND_TITLE_HEIGHT, SCREEN_WIDTH, WND_TITLE_HEIGHT);
    msg.type = MSG_PAINT;
    msg.param = WPT_WHOLEDC;
    pvcSendMessage(hwnd, msg);
  }
    return FINISH;
  case MSG_TIMEOUT:
  {
    uint t = getTime();
    char buf[20];
    memset(timeDc.content, 0x0, sizeof(PColor) * timeDc.size.cx * timeDc.size.cy);
    sprintf(buf, "%2d:%2d:%2d\n", ((t >> 16) & 0xffff), ((t >> 8) & 0xff), (t & 0xff));
    pvcDrawRect(&timeDc, 0, 0, timeDc.size.cx, timeDc.size.cy);
    //printf(1, "%s\n", buf);
    pvcDrawText(&timeDc, buf, 0, 0);
    paintWindow(hwnd, 726, 600 - WND_TITLE_HEIGHT + 6, &timeDc, 0, 0, timeDc.size.cx, timeDc.size.cy);
  }
    break;
  case MSG_RBUTTON_UP:
  {
    int x = (msg.param >> 16) & 0xffff;
    int y = msg.param & 0xffff;
    PHMenu menu = pvcCreateMenu(hwnd, true);
    pvcMenuAddItem(menu, "Refresh", 0, pvcRefresh);
    pvcShowMenuAt(menu, x, y);
  }
    break;
  case MSG_KEY_DOWN:
    if (msg.param == VK_ESC || msg.param == VK_F4)
      return FINISH;
  case MSG_PAINT:
  {
    PHdc hdc = pvcGetDC(hwnd);
    #ifdef BITMAP
    pvcBltbit(hdc, 0, 0, backgroundDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    #else
    hdc->pen.color = COLOR_NULL;
    hdc->pen.size = 0;
    hdc->font.color = RGB(68, 64, 63);
    hdc->brush.color = RGB(85, 175, 135);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    hdc->pen.size = 20;
    hdc->pen.color = RGB(195, 245, 205);
    pvcDrawLine(hdc, 150, 100, 150, 470);
    pvcDrawLine(hdc, 150, 100, 250, 100);
    pvcDrawLine(hdc, 250, 100, 250, 300);
    pvcDrawLine(hdc, 250, 300, 150, 300);

    pvcDrawLine(hdc, 350, 100, 400, 470);
    pvcDrawLine(hdc, 400, 470, 450, 100);

    pvcDrawLine(hdc, 650, 100, 550, 100);
    pvcDrawLine(hdc, 550, 100, 550, 470);
    pvcDrawLine(hdc, 550, 470, 650, 470);
    #endif

    //hwnd->wholeDc.brush.color = RGB(195, 245, 205);
    //pvcDrawRect(&hwnd->wholeDc, 4, hwnd->dc.size.cy + 4, 42, hwnd->wholeDc.size.cy - hwnd->dc.size.cy - 8);
  }
    break;
  case MSG_OTHER_WINDOW_ACTION:
  {
    int id = msg.param & 0xffff;
    int action = (msg.param >> 16) & 0xffff;
    PWndListNode node;
    getWindowInfo(id, &node);
    switch (action)
    {
    case OTHER_WINDOW_OPEN:
    {
      TaskBarItem * p = (TaskBarItem *)malloc(sizeof(TaskBarItem));
      p->next = 0;
      p->wndId = id;
      strcpy(p->title, node.title);
      if (taskBarData.head == 0)
        taskBarData.head = taskBarData.tail = p;
      else
      {
        taskBarData.tail->next = p;
        taskBarData.tail = p;
      }
      ++taskBarData.num;
      pvcCtrlInvalidate(taskBar);
    }
      break;
    case OTHER_WINDOW_LOSE_FOCUS:
    case OTHER_WINDOW_CLOSE:
    case OTHER_WINDOW_HAS_FOCUS:
    {
      TaskBarItem * p = taskBarData.head;
      TaskBarItem * q = 0;
      while (p)
      {
        if (p->wndId == id)
        {
          if (action == OTHER_WINDOW_CLOSE)
          {
            if (q == 0)
              taskBarData.head = p->next;
            else
              q->next = p->next;
            if (p == taskBarData.tail)
              taskBarData.tail = q;
            free(p);
            --taskBarData.num;
          }
          else if (action == OTHER_WINDOW_HAS_FOCUS)
          {
            taskBarData.focus = p;
          }
          else if (action == OTHER_WINDOW_LOSE_FOCUS)
          {
            if (taskBarData.focus == p)
              taskBarData.focus = 0;
            else
              return FINISH;
          }
          break;
        }
        q = p;
        p = p->next;
      }
      pvcCtrlInvalidate(taskBar);
    }
      return FINISH;
    }
    //printf(1, "%s action\n", node.title);
  }
    return FINISH;
  default:
    break;
  }
  return pvcWndProc(hwnd, msg);
}

int main(void)
{
  desktopInitStringFigure();
  PHwnd r = (PHwnd)malloc(sizeof(PWindow));
  if (r == 0)
    pvcError(0);
  strcpy(r->title, "desktop");

  r->pos.x = 0;
  r->pos.y = 0;
  r->wholeDc.size.cx = SCREEN_WIDTH;
  r->wholeDc.size.cy = SCREEN_HEIGHT;
  r->wholeDc.content = (PColor *)malloc(sizeof(PColor) * r->wholeDc.size.cx * r->wholeDc.size.cy);
  if (r->wholeDc.content == 0)
    pvcError(0);
  memset(r->wholeDc.content, 0x0c, sizeof(PColor) * r->wholeDc.size.cx * r->wholeDc.size.cy);

  r->clientPos.x = 0;
  r->clientPos.y = 0;
  r->dc.size.cx = SCREEN_WIDTH;
  r->dc.size.cy = SCREEN_HEIGHT - WND_TITLE_HEIGHT;
  r->dc.content = (PColor *)malloc(sizeof(PColor) * r->dc.size.cx * r->dc.size.cy);
  if (r->dc.content == 0)
    pvcError(0);
  memset(r->dc.content, 0x0, sizeof(PColor) * r->dc.size.cx * r->dc.size.cy);

  r->msg.type = MSG_NULL;
  r->state = 0;
  r->pid = getpid();
  r->msgQueueId = -1;

  r->parentId = -1;

  pvcCtrlLstInit(&r->ctrlLst);

  taskBarData.num = 0;
  taskBarData.head = taskBarData.tail = taskBarData.select
   = taskBarData.focus = taskBarData.mouse = 0;
  PHwnd hwnd = r;
  pvcWndExec(hwnd, wndProc);
  exit();
}
