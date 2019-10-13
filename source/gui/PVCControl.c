#include "types.h"
#include "user.h"
#include "PVCControl.h"
#include "PVCPainter.h"
#include "PVCWindow.h"

void pvcCtrlLstInit(PControlList * list)
{
  list->head = list->tail = list->entry = list->mouseIn = 0;
  list->mouseCatch = 0;
}

void pvcCtrlLstAddToTail(PControlList * list, PHControl ctrl)
{
  PControlListNode * p = (PControlListNode *)malloc(sizeof(PControlListNode));
  p->ctrl = ctrl;
  p->prev = p->next = 0;

  for (int i = 0; i < ctrl->dc.size.cy; ++i)
  {
    for (int j = 0; j < ctrl->dc.size.cx; ++j)
    {
      p->ctrl->background[i * ctrl->dc.size.cx + j] =
        pvcPixel(&ctrl->hwnd->wholeDc, ctrl->pos.x + j, ctrl->pos.y + i);
    }
  }

  PMessage msg;
  msg.type = MSG_CREATE;
  pvcCtrlSendMessage(ctrl, msg);

  if (list->tail == 0)
    list->head = list->tail = p;
  else
  {
    p->prev = list->tail;
    list->tail->next = p;
    list->tail = p;
  }
}

void pvcCtrlLstRemove(PControlList * list, PHControl ctrl, bool flag)
{
  if (list->entry && list->entry->ctrl == ctrl)
    list->entry = 0;
  if (list->mouseIn && list->mouseIn->ctrl == ctrl)
    list->mouseIn = 0;
  PControlListNode *p;
  p = list->head;
  while (p)
  {
    if (p->ctrl == ctrl)
    {
      if (p == list->head)
      {
        if (p->next == 0)
        {
          list->head = list->tail = 0;
        }
        else
        {
          list->head = list->head->next;
          list->head->prev = 0;
        }
      }
      else if (p == list->tail)
      {
        list->tail = list->tail->prev;
        list->tail->next = 0;
      }
      else
      {
        p->prev->next = p->next;
        p->next->prev = p->prev;
      }
      if (flag == true)
        free(p->ctrl);
      PMessage msg;
      msg.type = MSG_CTRL_REMOVE;
      msg.param = (uint)p->ctrl;
      pvcSendMessage(p->ctrl->hwnd, msg);
      free(p);
      return;
    }
    p = p->next;
  }
}

void pvcCtrlLstDestroy(PControlList * list, bool flag)
{
  if (list->head == 0)
    return;
  list->tail = list->head;
  while (list->tail)
  {
    list->head = list->head->next;
    if (flag)
      free(list->tail->ctrl);
    free(list->tail);
    list->tail = list->head;
  }
  list->tail = list->head = list->entry = list->mouseIn = 0;
}

bool pvcDispatchMsgToCtrlLst(PControlList * list, PMessage msg)
{
  switch (msg.type)
  {
  case MSG_KEY_DOWN:
    if (msg.param == VK_TAB)
    {
      if (list->entry == 0)
        return SUBMIT;
      else
      {
        PMessage t;
        t.type = MSG_LOSE_FOCUS;
        t.param = (uint)(list->entry->next->ctrl);
        list->entry->ctrl->ctrlProc(list->entry->ctrl, t);
        t.param = (uint)(list->entry->ctrl);
        list->entry = list->entry->next;
        if (list->entry)
        {
          t.type = MSG_HAS_FOCUS;
          list->entry->ctrl->ctrlProc(list->entry->ctrl, t);
        }
        return FINISH;
      }
    }
  case MSG_KEY_UP:
    if (list->entry)
      return list->entry->ctrl->ctrlProc(list->entry->ctrl, msg);
    break;
  case MSG_LBUTTON_UP:
  case MSG_RBUTTON_UP:
  case MSG_MBUTTON_UP:
  case MSG_MOUSE_MOVE:
    if (list->mouseCatch != 0)
    {
      int x = (msg.param >> 16) & 0xffff;
      int y = msg.param & 0xffff;
      x -= list->mouseCatch->pos.x;
      y -= list->mouseCatch->pos.y;
      if (x < 0)
       x = 0;
      else if (x >= list->mouseCatch->dc.size.cx)
        x = list->mouseCatch->dc.size.cx - 1;
      if (y < 0)
        y = 0;
      else if (y >= list->mouseCatch->dc.size.cy)
        y = list->mouseCatch->dc.size.cy - 1;
      msg.param = ((x << 16) & 0xffff0000) | (y & 0xffff);
      int r = list->mouseCatch->ctrlProc(list->mouseCatch, msg);
      if (msg.type != MSG_MOUSE_MOVE)
        list->mouseCatch = 0;
      if (r == TERMINATE)
        return FINISH;
      return r;
    }
  case MSG_LBUTTON_DOWN:
  case MSG_LBUTTON_DCLK:
  case MSG_RBUTTON_DOWN:
  case MSG_RBUTTON_DCLK:
  case MSG_MBUTTON_DOWN:
  case MSG_MBUTTON_DCLK:
  case MSG_WHEEL_UP:
  case MSG_WHEEL_DOWN:
  {
    int x = (msg.param >> 16) & 0xffff;
    int y = msg.param & 0xffff;
    /*if (list->mouseIn && !ctrlContain(list->mouseIn->ctrl, x, y))
    {
      PMessage t;
      t.type = MSG_MOUSE_OUT;
      t.param = msg.type;
      list->mouseIn->ctrl->ctrlProc(list->mouseIn->ctrl, t);
    }
    if (list->entry && ctrlContain(list->entry->ctrl, x, y))
    {
      x -= list->entry->ctrl->pos.x;
      y -= list->entry->ctrl->pos.y;
      msg.param = ((x << 16) & 0xffff0000) | (y & 0xffff);
      list->mouseIn = list->entry;
      int r = list->entry->ctrl->ctrlProc(list->entry->ctrl, msg);
      if (r == TERMINATE)
        return FINISH;
      return r;
    }*/
    PControlListNode * p = list->tail;
    while (p)
    {
      if (ctrlContain(p->ctrl, x, y))
      {
        if (list->mouseIn && p != list->mouseIn)
        {
          PMessage t;
          t.type = MSG_MOUSE_OUT;
          t.param = msg.type;
          list->mouseIn->ctrl->ctrlProc(list->mouseIn->ctrl, t);
        }
        if (p == list->entry)
        {
          x -= list->entry->ctrl->pos.x;
          y -= list->entry->ctrl->pos.y;
          msg.param = ((x << 16) & 0xffff0000) | (y & 0xffff);
          list->mouseIn = list->entry;
          int r = list->entry->ctrl->ctrlProc(list->entry->ctrl, msg);
          if (r == TERMINATE)
            return FINISH;
          return r;
        }
        x -= p->ctrl->pos.x;
        y -= p->ctrl->pos.y;
        msg.param = ((x << 16) & 0xffff0000) | (y & 0xffff);
        list->mouseIn = p;
        int r = p->ctrl->ctrlProc(p->ctrl, msg);
        if (r == SUBMIT)
          return SUBMIT;
        else if (r == FINISH)
        {
          if (list->entry)
          {
            PMessage t;
            t.type = MSG_LOSE_FOCUS;
            t.param = (uint)(p->ctrl);
            list->entry->ctrl->ctrlProc(list->entry->ctrl, t);
          }
          PMessage t;
          t.type = MSG_HAS_FOCUS;
          t.param = (uint)(list->entry->ctrl);
          list->entry = p;
          list->entry->ctrl->ctrlProc(list->entry->ctrl, t);
          return FINISH;
        }
        else if (r == TERMINATE)
          return FINISH;
      }
      p = p->prev;
    }
    if (msg.type != MSG_WHEEL_UP && msg.type != MSG_WHEEL_DOWN && msg.type != MSG_MOUSE_MOVE)
    {
      if (list->entry)
      {
        PMessage t;
        t.type = MSG_LOSE_FOCUS;
        t.param = msg.type;
        list->entry->ctrl->ctrlProc(list->entry->ctrl, t);
      }
      list->entry = 0;
    }

    if (list->mouseIn && msg.type == MSG_MOUSE_MOVE)
    {
      PMessage t;
      t.type = MSG_MOUSE_OUT;
      t.param = msg.type;
      list->mouseIn->ctrl->ctrlProc(list->mouseIn->ctrl, t);
      list->mouseIn = 0;
    }
  }
    break;
  case MSG_MOUSE_OUT:
  case MSG_TIMEOUT:
  case MSG_LOSE_FOCUS:
  case MSG_CTRL_REMOVE:
  {
    PControlListNode * p = list->tail;
    while (p)
    {
      p->ctrl->ctrlProc(p->ctrl, msg);
      p = p->prev;
    }
  }
    break;
  default:
    break;
  }
  return SUBMIT;
}

void pvcPaintControl(PHControl ctrl)
{
  //pvcBltbit(&ctrl->hwnd->wholeDc, ctrl->pos.x, ctrl->pos.y, &ctrl->dc, 0, 0, ctrl->dc.size.cx, ctrl->dc.size.cy);
  for (int y = 0; y < ctrl->dc.size.cy; ++y)
  {
    for (int x = 0; x < ctrl->dc.size.cx; ++x)
    {
      PControlList * list = &ctrl->hwnd->ctrlLst;
      PControlListNode * p = list->tail;
      while (p && p->ctrl != ctrl)
      {
        if (ctrlContain(p->ctrl, ctrl->pos.x + x, ctrl->pos.y + y))
          break;
        p = p->prev;
      }
      if (p->ctrl == ctrl)
      {
        pvcSetPixel(&ctrl->hwnd->wholeDc, ctrl->pos.x + x, ctrl->pos.y + y,
          pvcPixel(&ctrl->dc, x, y));
      }
    }
  }
  paintWindow(ctrl->hwnd, ctrl->pos.x, ctrl->pos.y, &ctrl->hwnd->wholeDc, ctrl->pos.x, ctrl->pos.y, ctrl->dc.size.cx, ctrl->dc.size.cy);
}

bool pvcCtrlProc(PHControl ctrl, PMessage msg)
{
  switch (msg.type)
  {
  case MSG_PAINT:
    pvcPaintControl(ctrl);
    break;
  default:
    break;
  }
  return SUBMIT;
}

PHControl pvcCreateControl(PHwnd hwnd, int x, int y, int w, int h, void * data, bool (*ctrlProc)(PHControl, PMessage))
{
  PHControl ctrl = (PHControl)malloc(sizeof(PControl));
  ctrl->pos.x = x;
  ctrl->pos.y = y;
  ctrl->dc.size.cx = w;
  ctrl->dc.size.cy = h;
  ctrl->dc.content = (PColor *)malloc(sizeof(PColor) * w * h);
  ctrl->background = (PColor *)malloc(sizeof(PColor) * w * h);
  ctrl->dc.pen.size = 0;
  ctrl->dc.pen.color = ctrl->dc.brush.color = ctrl->dc.font.color = COLOR_NULL;
  ctrl->data = data;
  ctrl->state = CS_SHOW;
  ctrl->ctrlProc = ctrlProc;
  ctrl->hwnd = hwnd;
  if (hwnd)
    pvcCtrlLstAddToTail(&hwnd->ctrlLst, ctrl);
  return ctrl;
}

void pvcCtrlSendMessage(PHControl ctrl, PMessage msg)
{
  ctrl->ctrlProc(ctrl, msg);
}

void pvcCtrlInvalidate(PHControl ctrl)
{
  PMessage msg;
  msg.type = MSG_PAINT;
  msg.param = WPT_DC;
  pvcCtrlSendMessage(ctrl, msg);
}

void pvcPaintButton(PButtonData * data, PHdc hdc)
{
  switch (data->state)
  {
  case BS_MOUSE_OUT:
    hdc->pen.size = 2;
    hdc->pen.color = RGB(215, 215, 235);
    hdc->brush.color = RGB(215, 225, 215);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    break;
  case BS_MOUSE_IN:
    hdc->pen.size = 2;
    hdc->pen.color = RGB(165, 165, 185);
    hdc->brush.color = RGB(220, 230, 220);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    break;
  case BS_BUTTON_DOWN:
    hdc->pen.size = 2;
    hdc->pen.color = RGB(215, 215, 235);
    hdc->brush.color = RGB(225, 235, 225);
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    break;
  }
  for (int i = 0; i < hdc->size.cx; i++)
  {
    for (int j = 0; j < hdc->size.cy; j++)
    {
      PColor color = pvcPixel(hdc, i, j);
      int d = (hdc->size.cx - i) * 20 / hdc->size.cx;
      color.r += d;
      color.g += d;
      color.b += d;
      pvcSetPixel(hdc, i, j, color);
    }
  }
  if (data->name)
  {
    int nameSize = strlen(data->name);
    hdc->font.color = RGB(65, 65, 85);
    pvcDrawText(hdc, data->name, (hdc->size.cx - FONT_SIZE_CX * nameSize) / 2, (hdc->size.cy - FONT_SIZE_CY) / 2);
  }
}

bool pvcButtonProc(PHButton btn, PMessage msg)
{
  PButtonData * data = (PButtonData *)btn->data;
  switch (msg.type)
  {
  case MSG_PAINT:
    memmove(btn->dc.content, btn->background, sizeof(PColor) * btn->dc.size.cx * btn->dc.size.cy);
    data->paintButton(data, &btn->dc);
    break;
  case MSG_MOUSE_OUT:
  case MSG_LOSE_FOCUS:
    if (data->state != BS_MOUSE_OUT)
    {
      //printf(1, "%s MSG_MOUSE_OUT\n", (char *)data->param);
      data->state = BS_MOUSE_OUT;
      pvcCtrlInvalidate(btn);
    }
    return FINISH;
  case MSG_MOUSE_MOVE:
  case MSG_HAS_FOCUS:
    if (data->state == BS_MOUSE_OUT)
    {
      //printf(1, "%s MSG_MOUSE_IN\n", (char *)data->param);
      data->state = BS_MOUSE_IN;
      pvcCtrlInvalidate(btn);
    }
    return SUBMIT;
  case MSG_LBUTTON_DOWN:
    data->state = BS_BUTTON_DOWN;
    pvcCtrlInvalidate(btn);
    return FINISH;
  case MSG_LBUTTON_UP:
    if (data->state == BS_BUTTON_DOWN)
    {
      data->state = BS_MOUSE_IN;
      data->call(data->param);
      pvcCtrlInvalidate(btn);
      /*pvcCtrlLstRemove(&btn->hwnd->ctrlLst, btn, false);
      pvcInvalidate(btn->hwnd);
      free(data);
      //menu->dc.size.cx = menu->dc.size.cy = 0;
      free(btn->dc.content);
      free(btn->background);
      free(btn);
      return TERMINATE;*/
    }
    return FINISH;
  default:
    break;
  }
  return pvcCtrlProc(btn, msg);
}

PHButton pvcCreateButton(PHwnd hwnd, char * name, int x, int y, int w, int h,
   void * param, void (*call)(void *), void (*paintButton)(PButtonData *, PHdc))
{
  PHButton btn = (PHButton)pvcCreateControl(hwnd, x, y, w, h, 0, pvcButtonProc);
  PButtonData * data = (PButtonData *)malloc(sizeof(PButtonData));
  data->call = call;
  data->paintButton = paintButton;
  data->state = BS_MOUSE_OUT;
  data->param = param;
  data->name = name;
  btn->data = (void *)data;
  return btn;
}

void pvcPaintMenu(PMenuData * data, PHdc hdc)
{
  hdc->pen.size = 0;
  hdc->pen.color = COLOR_NULL;
  hdc->brush.color = RGB(240, 241, 242);
  hdc->font.color = RGB(62, 50, 40);
  for (int i = 0; i < data->num; ++i)
  {
    if (i == data->select)
    {
      hdc->pen.color = RGB(0, 0, 0);
      hdc->pen.size = 2;
    }
    int l = (strlen(data->data[i].name) + 1) * FONT_SIZE_CX;
    pvcDrawRect(hdc, 0, i * data->h, data->w, data->h);
    pvcDrawText(hdc, data->data[i].name, (data->w - l) / 2, i * data->h + (data->h - FONT_SIZE_CY) / 2);
    if (data->data[i].call == NULL_FUNCTION)
    {
      hdc->pen.size = 1;
      hdc->pen.color = RGB(62, 50, 40);
      pvcDrawLine(hdc, data->w - FONT_SIZE_CX, i * data->h + (data->h - FONT_SIZE_CY / 2) / 2,
        data->w - FONT_SIZE_CX, i * data->h + (data->h - FONT_SIZE_CY / 2) / 2 + FONT_SIZE_CY / 2);
      pvcDrawLine(hdc, data->w - FONT_SIZE_CX, i * data->h + (data->h - FONT_SIZE_CY / 2) / 2,
        data->w - FONT_SIZE_CX / 2, i * data->h + (data->h - FONT_SIZE_CY / 2) / 2 + FONT_SIZE_CY / 4);
      pvcDrawLine(hdc, data->w - FONT_SIZE_CX / 2, i * data->h + (data->h - FONT_SIZE_CY / 2) / 2 + FONT_SIZE_CY / 4,
        data->w - FONT_SIZE_CX, i * data->h + (data->h - FONT_SIZE_CY / 2) / 2 + FONT_SIZE_CY / 2);
      //pvcDrawRect(hdc, data->w - FONT_SIZE_CX, i * data->h + (data->h - FONT_SIZE_CY / 2) / 2, FONT_SIZE_CX / 2, FONT_SIZE_CY / 2);
      hdc->pen.size = 0;
      hdc->pen.color = COLOR_NULL;
    }
    if (i == data->select)
    {
      hdc->pen.size = 0;
      hdc->pen.color = COLOR_NULL;
    }
  }
}

bool pvcMenuLoseFocus(PHMenu menu, uint param)
{
  PMenuData * data = (PMenuData *)menu->data;
  if ((uint)menu == param)
    return false;
  for (int i = 0; i < data->num; ++i)
  {
    if (data->data[i].call == NULL_FUNCTION)
    {
      PSubMenuData * sub = data->data[i].param;
      if ((uint)(sub->subMenu) == param)
        return false;
      bool r = pvcMenuLoseFocus(sub->subMenu, param);
      if (r == false)
        return false;
    }
  }
  return true;
}

bool pvcMenuProc(PHMenu menu, PMessage msg)
{
  PMenuData * data = (PMenuData *)menu->data;
  switch (msg.type)
  {
  case MSG_PAINT:
    pvcPaintMenu(data, &menu->dc);
    break;
  case MSG_LBUTTON_DOWN:
    return FINISH;
  case MSG_LBUTTON_UP:
  {
    if (data->select >= 0 && data->data[data->select].call != NULL_FUNCTION)
    {
      data->data[data->select].call(data->data[data->select].param);
      pvcDestroyMenu(menu, ((PMenuData *)menu->data)->flag);
      return TERMINATE;
    }
  }
    return FINISH;
  case MSG_MOUSE_MOVE:
  {
    int y = msg.param & 0xffff;
    y /= data->h;
    if (y != data->select && y >= 0 && y < data->num)
    {
      if (data->select >= 0 && data->data[data->select].call == NULL_FUNCTION)
      {
        PSubMenuData * subMenudata = (PSubMenuData *)data->data[data->select].param;
        pvcDestroySubMenu(subMenudata->subMenu, false);
        menu->state = MS_SHOW;
      }
      data->select = y;
      if (data->data[data->select].call == NULL_FUNCTION)
      {
        pvcShowSubMenu(data->data[data->select].param);
        menu->state = MS_CHILD;
      }
      pvcCtrlInvalidate(menu);
    }
  }
    return FINISH;
  case MSG_LOSE_FOCUS:
  {
    while (1)
    {
      PMenuData * p = (PMenuData *)menu->data;
      if (p->parent == 0)
        break;
      menu = p->parent;
    }
    if (pvcMenuLoseFocus(menu, msg.param))
      pvcDestroyMenu(menu, false);
  }
    return TERMINATE;
  }
  return pvcCtrlProc(menu, msg);
}

void pvcShowMenuAt(PHMenu menu, int x, int y)
{
  menu->pos.x = x;
  menu->pos.y = y;
  pvcShowMenu(menu);
}

void pvcShowMenu(PHMenu menu)
{
  menu->state = CS_SHOW;
  ((PMenuData *)menu->data)->select = -1;
  pvcCtrlLstAddToTail(&menu->hwnd->ctrlLst, menu);
  pvcCtrlInvalidate(menu);
}

void pvcHideMenu(PHMenu menu)
{
  memmove(menu->dc.content, menu->background, sizeof(PColor) * menu->dc.size.cx * menu->dc.size.cy);
  pvcPaintControl(menu);
  pvcCtrlLstRemove(&menu->hwnd->ctrlLst, menu, false);
  menu->state = CS_HIDE;
}

void * pvcShowSubMenu(void * param)
{
  PSubMenuData * subMenuData = (PSubMenuData *)param;
  subMenuData->subMenu->pos.x = subMenuData->menu->pos.x + ((PMenuData *)subMenuData->menu->data)->w;
  subMenuData->subMenu->pos.y = subMenuData->menu->pos.y + ((PMenuData *)subMenuData->menu->data)->h * subMenuData->index;
  subMenuData->subMenu->state = CS_SHOW;
  pvcShowMenu(subMenuData->subMenu);
  return 0;
}

PHMenu pvcCreateMenu(PHwnd hwnd, bool flag)
{
  PMenuData * data = (PMenuData *)malloc(sizeof(PMenuData));
  data->h = FONT_SIZE_CY + 4;
  data->w = 0;
  data->num = 0;
  data->len = 2;
  data->select = -1;
  data->parent = 0;
  data->flag = flag;
  data->data = (PMenuUnit *)malloc(sizeof(PMenuUnit) * data->len);
  PHMenu menu = (PHMenu)malloc(sizeof(PControl));
  menu->pos.x = 0;
  menu->pos.y = 0;
  menu->dc.size.cx = 0;
  menu->dc.size.cy = 0;
  menu->dc.content = 0;
  menu->background = 0;
  menu->dc.pen.size = 0;
  menu->dc.pen.color = menu->dc.brush.color = menu->dc.font.color = COLOR_NULL;
  menu->data = data;
  menu->state = CS_HIDE;
  menu->ctrlProc = pvcMenuProc;
  menu->hwnd = hwnd;
  return menu;
}

void pvcDestroySubMenu(PHMenu menu, bool flag)
{
  PMenuData * data = (PMenuData *)menu->data;
  for (int i = 0; i < data->num; ++i)
  {
    if (data->data[i].call == NULL_FUNCTION)
    {
      PSubMenuData * sub = (PSubMenuData *)data->data[i].param;
      pvcDestroySubMenu(sub->subMenu, flag);
      if (flag)
        free(sub);
    }
  }
  if (menu->state != MS_HIDE)
    pvcHideMenu(menu);
  if (flag)
  {
    free(data->data);
    free(data);
    free(menu->dc.content);
    free(menu->background);
    free(menu);
  }
}

void pvcDestroyMenu(PHMenu menu, bool flag)
{
  while (1)
  {
    PMenuData * data = (PMenuData *)menu->data;
    if (data->parent == 0)
      break;
    menu = data->parent;
  };
  pvcDestroySubMenu(menu, flag);
}

void pvcMenuAddItem(PHMenu menu, char * name, void * param, void (*call)(void *))
{
  PMenuData * data = (PMenuData *)menu->data;
  if (data->num + 1 >= data->len)
  {
    data->len *= 2;
    PMenuUnit * t = data->data;
    data->data = (PMenuUnit *)malloc(sizeof(PMenuUnit) * data->len);
    for (int i = 0; i < data->num; ++i)
      data->data[i] = t[i];
    free(t);
  }
  data->data[data->num].name = name;
  int w = (strlen(name) + 2) * FONT_SIZE_CX;
  if (w > data->w)
    data->w = w;
  data->data[data->num].param = param;
  data->data[data->num].call = call;
  ++data->num;
  menu->dc.size.cx = data->w;
  menu->dc.size.cy = data->h * data->num;
  if (menu->background)
    free(menu->background);
  menu->background = (PColor *)malloc(sizeof(PColor) * menu->dc.size.cx * menu->dc.size.cy);
  if (menu->dc.content)
    free(menu->dc.content);
  menu->dc.content = (PColor *)malloc(sizeof(PColor) * menu->dc.size.cx * menu->dc.size.cy);
}

void pvcMenuAddMenu(PHMenu menu, char * name, PHMenu subMenu)
{
  PSubMenuData * data = (PSubMenuData *)malloc(sizeof(PSubMenuData));
  data->menu = menu;
  data->subMenu = subMenu;
  ((PMenuData *)subMenu->data)->parent = menu;
  data->index = ((PMenuData *)menu->data)->num;
  pvcMenuAddItem(menu, name, (void *)data, NULL_FUNCTION);
}

void pvcPaintMenuBar(PMenuBarData * data, PHdc hdc)
{
  hdc->pen.size = 0;
  hdc->pen.color = COLOR_NULL;
  hdc->brush.color = RGB(240, 241, 242);
  hdc->font.color = RGB(62, 50, 40);
  pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
  int sx = 0;
  for (int i = 0; i < data->num; ++i)
  {
    if (i == data->select)
    {
      hdc->pen.size = 2;
      hdc->pen.color = RGB(0, 0, 0);
    }
    pvcDrawRect(hdc, sx, 0, data->data[i].w, data->h);
    int l = strlen(data->data[i].name) * FONT_SIZE_CX;
    pvcDrawText(hdc, data->data[i].name, sx + (data->data[i].w - l) / 2, (data->h - FONT_SIZE_CY) / 2);
    sx += data->data[i].w;
    if (i == data->select)
    {
      hdc->pen.size = 0;
      hdc->pen.color = COLOR_NULL;
    }
  }
}

bool pvcMenuBarProc(PHMenuBar menuBar, PMessage msg)
{
  PMenuBarData * data = (PMenuBarData *)menuBar->data;
  switch (msg.type)
  {
  case MSG_PAINT:
    pvcPaintMenuBar(data, &menuBar->dc);
    break;
  case MSG_LBUTTON_UP:
    return FINISH;
  case MSG_LBUTTON_DOWN:
  case MSG_MOUSE_MOVE:
  {
    int x = (msg.param >> 16) & 0xffff;
    int k = -1;
    int sx = 0;
    for (int i = 0; i < data->num; ++i)
    {
      if (x > 0 && x < data->data[i].w)
      {
        k = i;
        break;
      }
      x -= data->data[i].w;
      sx += data->data[i].w;
    }
    if (k != data->select && k >= 0 && k < data->num)
    {
      uint t = k;
      k = data->select;
      data->select = t;
      if (k >= 0)
        pvcDestroySubMenu(data->data[k].menu, false);
      pvcShowMenuAt(data->data[data->select].menu, menuBar->pos.x + sx, menuBar->pos.y + data->h);
    }
    else if (msg.type == MSG_LBUTTON_DOWN)
    {
      if (data->select >= 0)
      {
        pvcDestroySubMenu(data->data[data->select].menu, false);
        data->select = -1;
      }
    }
    pvcCtrlInvalidate(menuBar);
  }
    return FINISH;
  case MSG_CTRL_REMOVE:
    for (int i = 0; i < data->num; ++i)
    {
      if (i == data->select && (uint)(data->data[i].menu) == msg.param)
      {
        data->select = -1;
        pvcCtrlInvalidate(menuBar);
        return FINISH;
      }
    }
    return FINISH;
  case MSG_LOSE_FOCUS:
    if (data->select >= 0)
    {
      if (msg.param != (uint)(data->data[data->select].menu))
      {
        if (pvcMenuLoseFocus(data->data[data->select].menu, msg.param))
        {
          pvcDestroySubMenu(data->data[data->select].menu, false);
          data->select = -1;
          pvcCtrlInvalidate(menuBar);
        }
      }
    }
    return FINISH;
  }
  return pvcCtrlProc(menuBar, msg);
}

PHMenuBar pvcCreateMenuBar(PHwnd hwnd)
{
  PMenuBarData * data = (PMenuBarData *)malloc(sizeof(PMenuBarData));
  data->h = FONT_SIZE_CY + 4;
  data->num = 0;
  data->len = 2;
  data->select = -1;
  data->data = (PMenuBarUnit *)malloc(sizeof(PMenuBarUnit) * data->len);
  PHMenuBar menuBar = pvcCreateControl(hwnd, hwnd->clientPos.x, hwnd->clientPos.y,
    hwnd->dc.size.cx, data->h, (void *)data, pvcMenuBarProc);
  return menuBar;
}

void pvcMenuBarAddMenu(PHMenuBar menuBar, char * name, PHMenu menu)
{
  PMenuBarData * data = (PMenuBarData *)menuBar->data;
  
  if (data->num + 1 >= data->len)
  {
    data->len *= 2;
    PMenuBarUnit * t = data->data;
    data->data = (PMenuBarUnit *)malloc(sizeof(PMenuBarUnit) * data->len);
    for (int i = 0; i < data->num; ++i)
      data->data[i] = t[i];
    free(t);
  }
  data->data[data->num].name = name;
  data->data[data->num].w = (strlen(name) + 1) * FONT_SIZE_CX;
  data->data[data->num].menu = menu;
  ++data->num;
}

void pvcSliderBarSetPos(PHSliderBar sliderBar, int pos)
{
  PSliderBarData * data = (PSliderBarData *)sliderBar->data;
  pos -= data->min;
  double scale = pos / (double)(data->max - data->min);
  if (data->type == SBT_VECTICAL)
  {
    data->pos = scale * (sliderBar->dc.size.cy - data->blockSize);
    if (data->pos < 0)
      data->pos = 0;
    else if (data->pos > sliderBar->dc.size.cy - data->blockSize)
      data->pos = sliderBar->dc.size.cy - data->blockSize;
  }
  else if (data->type == SBT_HORIZONAL)
  {
    data->pos = scale * (sliderBar->dc.size.cx - data->blockSize);
    if (data->pos < 0)
      data->pos = 0;
    else if (data->pos > sliderBar->dc.size.cx - data->blockSize)
      data->pos = sliderBar->dc.size.cx - data->blockSize;
  }
  //pvcCtrlInvalidate(sliderBar);
}

void pvcSliderBarSetScale(PHSliderBar sliderBar, double scale)
{
  PSliderBarData * data = (PSliderBarData *)sliderBar->data;
  data->scale = scale;
  if (data->type == SBT_VECTICAL)
  {
    data->blockSize = scale * sliderBar->dc.size.cy;
    if (data->pos < 0)
      data->pos = 0;
    else if (data->pos > sliderBar->dc.size.cy - data->blockSize)
      data->pos = sliderBar->dc.size.cy - data->blockSize;
  }
  else if (data->type == SBT_HORIZONAL)
  {
    data->blockSize = scale * sliderBar->dc.size.cx;
    if (data->pos < 0)
      data->pos = 0;
    else if (data->pos > sliderBar->dc.size.cx - data->blockSize)
      data->pos = sliderBar->dc.size.cx - data->blockSize;
  }
  //pvcCtrlInvalidate(sliderBar);
}

void pvcSliderBarSetRange(PHSliderBar sliderBar, int min, int max)
{
  PSliderBarData * data = (PSliderBarData *)sliderBar->data;
  data->min = min;
  data->max = max;
}

void pvcPaintSliderBar(PHSliderBar sliderBar, PHdc hdc)
{
  PSliderBarData * data = (PSliderBarData *)(sliderBar->data);
  memset(hdc->content, 241, sizeof(PColor) * hdc->size.cx * hdc->size.cy);
  /*if (data->pressed)
    hdc->pen.color = RGB(0, 0, 0);
  else
    hdc->pen.color = RGB(60, 60, 60);
  hdc->pen.size = 4;
  hdc->brush.color = COLOR_NULL;
  pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);*/
  if (data->pressed)
    hdc->brush.color = RGB(0, 0, 0);
  else
    hdc->brush.color = RGB(60, 60, 60);
  hdc->pen.size = 0;
  hdc->pen.color = COLOR_NULL;
  if (data->type == SBT_VECTICAL)
  {
    if (data->pressed)
      pvcDrawRect(hdc, 0, data->pos, hdc->size.cx, data->blockSize);
    else
      pvcDrawRect(hdc, hdc->size.cx / 4, data->pos, hdc->size.cx / 2, data->blockSize);
  }
  else if (data->type == SBT_HORIZONAL)
  {
    if (0)
      pvcDrawRect(hdc, data->pos, 0, data->blockSize, hdc->size.cy);
    else
      pvcDrawRect(hdc, data->pos, hdc->size.cy / 4, data->blockSize, hdc->size.cy / 2);
  }
}

bool pvcSliderBarProc(PHSliderBar sliderBar, PMessage msg)
{
  PSliderBarData * data = (PSliderBarData *)sliderBar->data;
  switch (msg.type)
  {
  case MSG_CREATE:
    pvcCtrlInvalidate(sliderBar);
    break;
  case MSG_PAINT:
    if (msg.param == WPT_WHOLEDC)
      return SUBMIT;
    data->paintSliderBar(sliderBar, &sliderBar->dc);
    break;
  case MSG_LBUTTON_DOWN:
  {
    sliderBar->hwnd->ctrlLst.mouseCatch = sliderBar;
    if (data->type == SBT_VECTICAL)
    {
      int y = msg.param & 0xffff;
      if (y >= data->pos  && y < data->pos + data->blockSize)
      {
        data->dPos = y - data->pos;
        data->pressed = true;
        pvcCtrlInvalidate(sliderBar);
      }
    }
    else if (data->type == SBT_HORIZONAL)
    {
      int x = (msg.param >> 16) & 0xffff;
      if (x >= data->pos && x < data->pos + data->blockSize)
      {
        data->dPos = x - data->pos;
        data->pressed = true;
        pvcCtrlInvalidate(sliderBar);
      }
    }
  }
    break;
  case MSG_MOUSE_OUT:
  case MSG_LOSE_FOCUS:
  case MSG_LBUTTON_UP:
    if (data->pressed == true)
    {
      data->pressed = false;
      pvcCtrlInvalidate(sliderBar);
    }
    break;
  case MSG_MOUSE_MOVE:
  {
    if (data->pressed == false)
      return FINISH;
    if (data->type == SBT_VECTICAL)
    {
      int y = msg.param & 0xffff;
      if (y - data->dPos == data->pos)
        return FINISH;
      data->pos = y - data->dPos;
      if (data->pos < 0)
        data->pos = 0;
      else if (data->pos > sliderBar->dc.size.cy - data->blockSize)
        data->pos = sliderBar->dc.size.cy - data->blockSize;
    }
    else if (data->type == SBT_HORIZONAL)
    {
      int x = (msg.param >> 16) & 0xffff;
      if (x - data->dPos == data->pos)
        return FINISH;
      data->pos = x - data->dPos;
      if (data->pos < 0)
        data->pos = 0;
      else if (data->pos > sliderBar->dc.size.cx - data->blockSize)
        data->pos = sliderBar->dc.size.cx - data->blockSize;
    }
    PMessage m;
    m.type = MSG_SLIDER_POS_CHANGE;
    double scale;
    if (data->type == SBT_VECTICAL)
      scale = data->pos / (double)(sliderBar->dc.size.cy - data->blockSize);
    else if (data->type == SBT_HORIZONAL)
      scale = data->pos / (double)(sliderBar->dc.size.cx - data->blockSize);
    int pos = scale * (data->max - data->min) + data->min;
    m.param = ((data->id & 0xffff) << 16) | (pos & 0xffff);
    pvcSendMessage(sliderBar->hwnd, m);
    pvcCtrlInvalidate(sliderBar);
  }
    break;
  }
  return pvcCtrlProc(sliderBar, msg);
}

PHSliderBar pvcCreateSliderBar(PHwnd hwnd, int id, int x, int y, int w, int h,
  int min, int max, double scale, int type, void (*paintSliderBar)(PHSliderBar, PHdc))
{
  PSliderBarData * data = (PSliderBarData *)malloc(sizeof(PSliderBarData));
  data->type = type;
  data->pos = 0;
  data->paintSliderBar = paintSliderBar;
  data->pressed = false;
  data->id = id;
  data->min = min;
  data->max = max;
  data->scale = scale;
  if (type == SBT_VECTICAL)
  {
    data->blockSize = scale * h;
  }
  else if (type == SBT_HORIZONAL)
  {
    data->blockSize = scale * w;
  }
  PHSliderBar sliderBar = pvcCreateControl(hwnd, x, y, w, h, data, pvcSliderBarProc);
  return sliderBar;
}

void pvcGetLineEditString(PHLineEdit lineEdit, char * buf)
{
  PLineEditData * data = (PLineEditData *)lineEdit->data;
  data->buf[data->index] = '\0';
  strcpy(buf, data->buf);
}

void pvcSetLineEditString(PHLineEdit lineEdit, char * buf)
{
  PLineEditData * data = (PLineEditData *)lineEdit->data;
  strcpy(data->buf, buf);
  data->sx = data->cursor = 0;
  data->index = strlen(buf);
  pvcCtrlInvalidate(lineEdit);
}

void pvcPaintLineEdit(PLineEditData * data, PHdc hdc)
{
  if (data->state == LES_INACTIVE)
  {
    hdc->pen.color = COLOR_NULL;
    hdc->pen.size = 0;
  }
  else
  {
    hdc->pen.color = RGB(53, 95, 125);
    hdc->pen.size = LE_ACTIVE_EDGE_SIZE;
  }
  hdc->brush.color = RGB(235, 235, 245);
  pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
  data->buf[data->index] = 0;
  hdc->font.color = RGB(132, 132, 132);
  int size = data->num;
  if (size > data->index - data->sx)
    size = data->index - data->sx;
  char * temp = (char *)malloc(sizeof(char) * (size + 1));
  memmove(temp, data->buf + data->sx, sizeof(char) * size);
  temp[size] = '\0';
  pvcDrawText(hdc, temp, LE_ACTIVE_EDGE_SIZE, (hdc->size.cy - FONT_SIZE_CY) / 2 - 1);

  if (data->state == LES_ACTIVE_CURSORSHOW)
  {
    hdc->pen.size = LES_CURSOR_SIZE;
    hdc->pen.color = RGB(132, 132, 132);
    pvcDrawLine(hdc, LE_ACTIVE_EDGE_SIZE + FONT_SIZE_CX * (data->cursor - data->sx),
      (hdc->size.cy - FONT_SIZE_CY) / 2 - 1, LE_ACTIVE_EDGE_SIZE + FONT_SIZE_CX *
        (data->cursor - data->sx), (hdc->size.cy - FONT_SIZE_CY) / 2 - 1 + FONT_SIZE_CY);
  }
  free(temp);
}


uchar bits = 0;

static char shiftMap[128] =
{
  ['`'] '~', ['1'] '!', ['2'] '@', ['3'] '#', ['4'] '$', ['5'] '%',
  ['6'] '^', ['7'] '&', ['8'] '*', ['9'] '(', ['0'] ')', ['-'] '_',
  ['='] '+', ['['] '{', [']'] '}', ['\\'] '|', [';'] ':', ['\''] '\"',
  [','] '<', ['.'] '>', ['/'] '?'
};

static char numberPadMap[256] =
{
  [VK_NUMPAD0] '0', [VK_NUMPAD1] '1', [VK_NUMPAD2] '2', [VK_NUMPAD3] '3', [VK_NUMPAD4] '4',
  [VK_NUMPAD5] '5', [VK_NUMPAD6] '6', [VK_NUMPAD7] '7', [VK_NUMPAD8] '8', [VK_NUMPAD9] '9',
  [VK_DIVIDE] '/', [VK_MULTIPLY] '*', [VK_MINUS] '-', [VK_ADD] '+', [VK_DOT] '.'
};

static inline char editGetChar(uint key)
{
  if ((bits & LE_CAPSLOCK_BIT) && key >= 'a' && key <= 'z')
    key += 'A' - 'a';
  if (bits & LE_SHIFT_BIT)
  {
    if (key >= 'a' && key <= 'z')
      key += 'A' - 'a';
    else if (key >= 'A' && key <= 'z')
      key += 'a' - 'A';
    else
      key = shiftMap[key];
    return key;
  }
  if (key >= 32 && key <= 126)
  {
    return key;
  }
  if (key == VK_SPACE)
    return ' ';
  else if(key == VK_ENTER)
    return '\n';
  else
    return numberPadMap[key];
  return 0;
}

bool pvcLineEditProc(PHLineEdit lineEdit, PMessage msg)
{
  PLineEditData * data = (PLineEditData *)lineEdit->data;
  switch (msg.type)
  {
  case MSG_PAINT:
    data->paintLineEdit(data, &lineEdit->dc);
    break;
  case MSG_KEY_DOWN:
  {
    switch (msg.param)
    {
    case VK_LSHIFT:
    case VK_RSHIFT:
      bits ^= LE_SHIFT_BIT;
      return FINISH;
    case VK_LCTRL:
    case VK_RCTRL:
      bits ^= LE_CTRL_BIT;
      return FINISH;
    case VK_LALT:
    case VK_RALT:
      bits ^= LE_ALT_BIT;
      return FINISH;
    case VK_CAPSLOCK:
      bits ^= LE_CAPSLOCK_BIT;
      return FINISH;
    case VK_NUMLOCK:
      bits ^= LE_NUMLOCK_BIT;
      return FINISH;
    case VK_ENTER:
    case VK_RETURN:
      data->call();
      return FINISH;
    case VK_LEFT:
      --data->cursor;
      if (data->cursor < 0)
        data->cursor = 0;
      if (data->cursor < data->sx)
        data->sx = data->cursor;
      pvcCtrlInvalidate(lineEdit);
      return FINISH;
    case VK_RIGHT:
      ++data->cursor;
      if (data->cursor > data->index)
        data->cursor = data->index;
      if (data->cursor > data->sx + data->num)
        data->sx = data->cursor - data->num;
      pvcCtrlInvalidate(lineEdit);
      return FINISH;
    case VK_HOME:
      data->cursor = 0;
      data->sx = 0;
      pvcCtrlInvalidate(lineEdit);
      return FINISH;
    case VK_END:
      data->cursor = data->index;
      data->sx = data->index - data->num;
      if (data->sx < 0)
        data->sx = 0;
      pvcCtrlInvalidate(lineEdit);
      return FINISH;
    case VK_DELETE:
      if (data->cursor >= data->index)
        return FINISH;
      ++data->cursor;
    case VK_BACKSPACE:
      if (data->index > 0 && data->cursor > 0)
      {
        for (int i = data->cursor - 1; i < data->index - 1; ++i)
          data->buf[i] = data->buf[i + 1];
        --data->index;
        --data->cursor;
        if (data->sx > 0)
          --data->sx;
      }
      pvcCtrlInvalidate(lineEdit);
      return FINISH;
    }
    if ((bits & LE_ALT_BIT) || (bits & LE_CTRL_BIT))
      return FINISH;
    char c = editGetChar(msg.param);
    if (c)
    {
      if (data->index < data->len)
      {
          for (int i = data->index; i > data->cursor; --i)
            data->buf[i] = data->buf[i - 1];
          ++data->index;
          data->buf[data->cursor++] = c;
          if (data->cursor - data->sx > data->num)
            data->sx = data->cursor - data->num;
      }
    }
    pvcCtrlInvalidate(lineEdit);
  }
    return FINISH;
  case MSG_KEY_UP:
    switch (msg.param)
    {
    case VK_LSHIFT:
    case VK_RSHIFT:
      bits ^= LE_SHIFT_BIT;
      return FINISH;
    case VK_LCTRL:
    case VK_RCTRL:
      bits ^= LE_CTRL_BIT;
      return FINISH;
    case VK_LALT:
    case VK_RALT:
      bits ^= LE_ALT_BIT;
      return FINISH;
    }
    return FINISH;
  case MSG_HAS_FOCUS:
    if (data->state == LES_INACTIVE)
    {
      data->state = LES_ACTIVE_CURSORSHOW;
      setTimer(lineEdit->hwnd, LE_TIMER_ID^ (uint)lineEdit, 500);
      bits &= ~LE_SHIFT_BIT & ~LE_CTRL_BIT & ~LE_ALT_BIT;
    }
    pvcCtrlInvalidate(lineEdit);
    break;
  case MSG_TIMEOUT:
  {
    if(msg.param >> 16 == ((LE_TIMER_ID ^ (uint)lineEdit) & 0xffff))
    {
      if(data->state == 1)
        data->state = 2;
      else
      data->state = 1;
    }
    pvcCtrlInvalidate(lineEdit);
  }
    break;
  case MSG_LBUTTON_DOWN:
  {
    int x = (msg.param >> 16) & 0xffff;
    data->cursor = (x - LE_ACTIVE_EDGE_SIZE) / FONT_SIZE_CX;
    if (data->cursor < 0)
      data->cursor = data->sx;
    else if (data->cursor > data->num)
      data->cursor = data->num;
    data->cursor += data->sx;
    if (data->cursor > data->index)
      data->cursor = data->index;
  }
    pvcCtrlInvalidate(lineEdit);
    return FINISH;
  case MSG_LOSE_FOCUS:
    if (data->state != LES_INACTIVE)
    {
      data->state = LES_INACTIVE;
      killTimer(lineEdit->hwnd, LE_TIMER_ID ^ (uint)lineEdit);
      //printf(1, "killTimer\n");
      pvcCtrlInvalidate(lineEdit);
    }
    return FINISH;
  default:
    break;
  }
  return pvcCtrlProc(lineEdit, msg);
}

PHLineEdit pvcCreateLineEdit(PHwnd hwnd, int x, int y, int w, int h, int len,
  void (*call)(void), void (*paintLineEdit)(PLineEditData *, PHdc))
{
  PHLineEdit lineEdit = (PHLineEdit)pvcCreateControl(hwnd, x, y, w, h, 0, pvcLineEditProc);
  PLineEditData * data = (PLineEditData *)malloc(sizeof(PLineEditData));
  data->call = call;
  data->paintLineEdit = paintLineEdit;
  data->len = len;
  data->buf = (char *)malloc(sizeof(char) * len);
  data->index = data->cursor = 0;
  data->state = LES_INACTIVE;
  data->sx = 0;
  data->num = (w - 2 * LE_ACTIVE_EDGE_SIZE) / FONT_SIZE_CX;
  lineEdit->data = (void *)data;
  return lineEdit;
}

PHMultiLineEdit pvcCreateMultiLineEdit(PHwnd hwnd, int x, int y, int w, int h,
   void (*paintMultiLineEdit)(PMultiLineEditData *, PHdc))
{
  PHMultiLineEdit multiLineEdit = (PHMultiLineEdit)pvcCreateControl(hwnd, x, y, w, h, 0, pvcMultiLineEditProc);
  PMultiLineEditData * data = (PMultiLineEditData *)malloc(sizeof(PMultiLineEditData));
  data->paintMultiLineEdit = paintMultiLineEdit;
  data->num = w / FONT_SIZE_CX - 1;
  data->rows_size = h / FONT_SIZE_CY;
  data->buf = (PSingleLine*)malloc(sizeof(PSingleLine) * data->rows_size);
  for (int i = 0; i < data->rows_size; i++)
  {
    data->buf[i].size = data->num;
    data->buf[i].buf = (char*)malloc(sizeof(char) * data->buf[i].size);
    data->buf[i].length = 0;
  }
  data->rows = 0;
  data->cursor_x = data->cursor_y = 0;
  data->position_x = data->position_y = 0;
  data->state = LES_INACTIVE;
  data->sy = 0;
  data->row_num = h / FONT_SIZE_CY;
  data->rows_all = 0;
  multiLineEdit->data = (void *)data;
  return multiLineEdit;
}

bool pvcMultiLineEditProc(PHMultiLineEdit multiLineEdit, PMessage msg)
{
  PMultiLineEditData * data = (PMultiLineEditData *)multiLineEdit->data;
  switch (msg.type)
  {
  case MSG_PAINT:
    data->paintMultiLineEdit(data, &multiLineEdit->dc);
    break;
  case MSG_KEY_DOWN:
  {
    switch (msg.param)
    {
    case VK_LSHIFT:
    case VK_RSHIFT:
      bits ^= LE_SHIFT_BIT;
      return FINISH;
    case VK_LCTRL:
    case VK_RCTRL:
      bits ^= LE_CTRL_BIT;
      return FINISH;
    case VK_LALT:
    case VK_RALT:
      bits ^= LE_ALT_BIT;
      return FINISH;
    case VK_CAPSLOCK:
      bits ^= LE_CAPSLOCK_BIT;
      return FINISH;
    case VK_NUMLOCK:
      bits ^= LE_NUMLOCK_BIT;
      return FINISH;
    case VK_UP:
      --data->cursor_y;
      if (data->cursor_y < 0)
        data->cursor_y = 0;
      if (data->cursor_y < data->sy)
        data->sy = data->cursor_y;
      if (data->position_x < data->num && data->position_y > 0)
        --data->position_y;
      if (data->position_x >= data->num)
        data->position_x -= data->num;
      pvcCtrlInvalidate(multiLineEdit);
      return FINISH;
    case VK_DOWN:
      if (data->cursor_y < data->rows_all)
        ++data->cursor_y;
      if ((data->position_x + data->num) / data->num * data->num >
        data->buf[data->position_y].length && data->position_y < data->rows)
        ++data->position_y;
      if (data->cursor_x > data->buf[data->position_y].length % data->num)
      {
        data->position_x -= (data->cursor_x - data->buf[data->position_y].length % data->num);
        data->cursor_x = data->buf[data->position_y].length % data->num;
      }
      //printf(1, "%d %d %d\n", data->position_x, data->position_y, data->buf[data->position_y].length);
      pvcCtrlInvalidate(multiLineEdit);
      return FINISH;
    case VK_LEFT:
      --data->cursor_x;
      if (data->cursor_x < 0)
        if (data->cursor_y > 0)
        {
          data->cursor_x = data->num;
          --data->cursor_y;
        }
      if (data->position_x > 0)
      --data->position_x;
      else if (data->position_y > 0)
        {
          data->position_x = 0;
          --data->position_y;
        }
      /*--data->cursor_x;
          else data->cursor_x = 0;*/
      pvcCtrlInvalidate(multiLineEdit);
      return FINISH;
    case VK_RIGHT:
      if (data->position_x < data->buf[data->position_y].length)
      {
        ++data->cursor_x;
        ++data->position_x;
      }
      if (data->cursor_x == data->num)
      {
        data->cursor_x = 0;
        ++data->cursor_y;
      }
      pvcCtrlInvalidate(multiLineEdit);
      return FINISH;
    case VK_HOME:
      data->cursor_x = 0;
      data->sy = 0;
      data->position_y = 0;
      data->position_x = 0;
      pvcCtrlInvalidate(multiLineEdit);
      return FINISH;
    case VK_END:
      //int t_cursor = data->cursor_x;
      if (data->buf[data->position_y].length >= data->num)
      {
        data->position_x += data->num - data->cursor_x;
        data->cursor_x = data->num;
      }
      else
      {
        data->position_x = data->buf[data->position_y].length;
        data->cursor_x = data->buf[data->position_y].length;
      }
      pvcCtrlInvalidate(multiLineEdit);
      return FINISH;
    case VK_DELETE:
      if (data->cursor_x >= data->buf[data->position_y].length && data->position_y == data->rows)
        return FINISH;
      if (data->position_x == data->buf[data->position_y].length)
      {
        if(data->position_y < data->rows)
        {
          PSingleLine temp;
          temp.length = data->buf[data->position_y].length + data->buf[data->position_y + 1].length;
          if (data->buf[data->position_y].length > data->buf[data->position_y + 1].length)
            temp.size = data->buf[data->position_y].size * 2;
          else
            temp.size = data->buf[data->position_y + 1].size * 2;
          temp.buf = (char*)malloc(sizeof(char) * temp.size);
          for (int i = 0; i < data->buf[data->position_y].length; i++)
            temp.buf[i] = data->buf[data->position_y].buf[i];
          for (int i = 0; i < data->buf[data->position_y + 1].length; i++)
            temp.buf[i + data->buf[data->position_y].length] = data->buf[data->position_y + 1].buf[i];
          for (int i = data->position_y + 1; i < data->rows - 1; i++)
            data->buf[i] = data->buf[i + 1];
          --data->rows;
          data->buf[data->position_y] = temp;
        }
        pvcCtrlInvalidate(multiLineEdit);
        return FINISH;
      }
      ++data->cursor_x;
      ++data->position_x;
    case VK_BACKSPACE:
      if (data->position_x > 0)
      {
        for (int i = data->position_x - 1; i <= data->buf[data->position_y].length; i++)
        {
          data->buf[data->position_y].buf[i] = data->buf[data->position_y].buf[i + 1];
        }
        --data->position_x;
        --data->cursor_x;
        --data->buf[data->position_y].length;
        if (data->cursor_x < 0)
        {
          data->cursor_x = data->num;
          --data->cursor_y;
        }
      }
      else
      {
        if (data->position_y > 0)
        {
          --data->position_y;
          data->position_x = data->buf[data->position_y].length;
          --data->cursor_y;
          data->cursor_x = data->position_x % data->num;
          PSingleLine temp;
          temp.length = data->buf[data->position_y].length + data->buf[data->position_y + 1].length;
          if (data->buf[data->position_y].length > data->buf[data->position_y + 1].length)
            temp.size = data->buf[data->position_y].size * 2;
          else
            temp.size = data->buf[data->position_y + 1].size * 2;
          temp.buf = (char*)malloc(sizeof(char) * temp.size);
          for (int i = 0; i < data->buf[data->position_y].length; i++)
            temp.buf[i] = data->buf[data->position_y].buf[i];
          for (int i = 0; i < data->buf[data->position_y + 1].length; i++)
            temp.buf[i + data->buf[data->position_y].length] = data->buf[data->position_y + 1].buf[i];
          for (int i = data->position_y + 1; i < data->rows - 1; i++)
            data->buf[i] = data->buf[i + 1];
          --data->rows;
          data->buf[data->position_y] = temp;
        }
      }
      pvcCtrlInvalidate(multiLineEdit);
      return FINISH;
    }
    if ((bits & LE_ALT_BIT) || (bits & LE_CTRL_BIT))
    return FINISH;
    char c = editGetChar(msg.param);
    if (c)
    {
      data->rows_all = 0;
      for (int i = 0; i < data->rows; i++)
      {
        data->rows_all += (data->buf[i].length / data->num);
        ++data->rows_all;
      }
      if (c == '\n')
      {
        if (data->rows + 1 == data->rows_size)
        {
          data->rows_size *= 2;
          PSingleLine * buf = (PSingleLine*)malloc(sizeof(PSingleLine*) * data->rows_size);
          for (int i = 0; i < data->rows_size; i++)
          {
            buf[i].size =  data->num;
            buf[i].length = 0;
            buf[i].buf = (char*)malloc(sizeof(char) * buf[i].size);
          }
          for (int i = 0; i < data->rows; i++)
          {
            buf[i] = data->buf[i];
          }
          data->buf = buf;
        }
        PSingleLine temp;
        temp.length = 0;
        temp.size = data->buf[data->position_y].size;
        temp.buf = (char*)malloc(sizeof(char) * temp.size);
        for (int i = data->position_x; i < data->buf[data->position_y].length;i++)
        {
          temp.buf[i - data->position_x] = data->buf[data->position_y].buf[i];
          ++temp.length;
        }
        temp.buf[temp.length-1] = '\0';
        data->buf[data->position_y].buf[data->position_x] = '\0';
        for (int i = data->rows;i > data->position_y;i--)
        {
          data->buf[i + 1] = data->buf[i];
        }
        data->buf[data->position_y + 1] = temp;
        ++data->rows;
        ++data->position_y;
        data->position_x = 0;
        data->cursor_x = 0;
        ++data->cursor_y;
      }
      else
      {
        if (data->buf[data->position_y].length == data->buf[data->position_y].size)
        {
          data->buf[data->position_y].size *= 2;
          char* buf = (char*)malloc(sizeof(char) * data->buf[data->position_y].size);
          for (int i = 0; i < data->buf[data->position_y].size / 2; i++)
            buf[i] = data->buf[data->position_y].buf[i];
          data->buf[data->position_y].buf = buf;
        }
        for (int i = data->buf[data->position_y].length; i > data->position_x; i--)
          data->buf[data->position_y].buf[i] = data->buf[data->position_y].buf[i - 1];
        data->buf[data->position_y].buf[data->position_x] = c;
      //  data->buf[data->position_y].buf[data->position_x + 1] = '\0';
        ++data->buf[data->position_y].length;
      //  printf(1, "%c", c);
        ++data->position_x;
        ++data->cursor_x;
        if (data->position_x % data->num == 0)
        {
          data->cursor_x = 0;
          ++data->cursor_y;
        }
        //printf(1, "%d %d", data->position_y, data->position_x);
        //printf(1, "%s\n", data->buf[data->position_y].buf);
      }
    }
    pvcCtrlInvalidate(multiLineEdit);
    return FINISH;
  }
  case MSG_KEY_UP:
    switch (msg.param)
    {
    case VK_LSHIFT:
    case VK_RSHIFT:
      bits ^= LE_SHIFT_BIT;
      return FINISH;
    case VK_LCTRL:
    case VK_RCTRL:
      bits ^= LE_CTRL_BIT;
      return FINISH;
    case VK_LALT:
    case VK_RALT:
      bits ^= LE_ALT_BIT;
      return FINISH;
    }
    return FINISH;
  case MSG_HAS_FOCUS:
    if (data->state == LES_INACTIVE)
    {
      data->state = LES_ACTIVE_CURSORSHOW;
      setTimer(multiLineEdit->hwnd, LE_TIMER_ID ^ (uint)multiLineEdit, 1000);
      bits &= ~LE_SHIFT_BIT & ~LE_CTRL_BIT & ~LE_ALT_BIT;
    }
    pvcCtrlInvalidate(multiLineEdit);
    break;
  case MSG_TIMEOUT:
  {
    if(msg.param >> 16 == ((LE_TIMER_ID ^ (uint)multiLineEdit) & 0xffff))
    {

      if(data->state == 1)
      {
        data->state = 2;
      }
      else if(data->state == 2)
      {
        data->state = 1;
      }


    multiLineEdit->dc.pen.size = LES_CURSOR_SIZE;
    if (data->state == LES_ACTIVE_CURSORSHOW)
      multiLineEdit->dc.pen.color = RGB(132, 132, 132);
    else
      multiLineEdit->dc.pen.color = RGB(195, 235, 215);
      //pvcDrawLine(&multiLineEdit->dc, 10, 10, 100, 100);
    pvcDrawLine(&multiLineEdit->dc, LE_ACTIVE_EDGE_SIZE + FONT_SIZE_CX * (data->cursor_x),
      data->cursor_y * FONT_SIZE_CY + LE_ACTIVE_EDGE_SIZE, LE_ACTIVE_EDGE_SIZE +
        FONT_SIZE_CX * (data->cursor_x), data->cursor_y * FONT_SIZE_CY + FONT_SIZE_CY);
    }
    pvcPaintControl(multiLineEdit);
  }
    break;
  case MSG_LBUTTON_DOWN:
  {
    data->rows_all = 0;
    for (int i = 0; i < data->rows; i++)
    {
      data->rows_all += (data->buf[i].length / data->num);
      ++data->rows_all;
    }
    int x = (msg.param >> 16) & 0xffff;
    int y = (msg.param) & 0xffff;
    data->cursor_x = (x - LE_ACTIVE_EDGE_SIZE) / FONT_SIZE_CX;
    data->cursor_y = y / FONT_SIZE_CY;
    if (data->cursor_y > data->rows_all - data->sy)
    {
      data->cursor_y = data->rows_all - data->sy;
    }
    data->position_y = data->sy;
    for (int i = 0; i < data->cursor_y; i++)
    {
      if (data->buf[data->position_y].length < data->num)
        ++data->position_y;
      i += data->buf[data->position_y].length / data->num;
    }
    if (data->cursor_x > data->buf[data->position_y].length)
      data->cursor_x = data->buf[data->position_y].length;
    data->position_x = data->cursor_x;
  //  printf(1, "%d %d %d\n", data->position_x, data->position_y, data->buf[data->position_y].length);
  }
    pvcCtrlInvalidate(multiLineEdit);
    return FINISH;
  case MSG_LOSE_FOCUS:
    if (data->state != LES_INACTIVE)
    {
      data->state = LES_INACTIVE;
      killTimer(multiLineEdit->hwnd, LE_TIMER_ID);
      //printf(1, "killTimer\n");
      pvcCtrlInvalidate(multiLineEdit);
    }
    return FINISH;
  default:
    break;
  }
  return pvcCtrlProc(multiLineEdit, msg);
}

void pvcPaintMultiLineEdit(PMultiLineEditData * data, PHdc hdc)
{
  if (data->state == LES_INACTIVE)
  {
    hdc->pen.color = COLOR_NULL;
    hdc->pen.size = 0;
  }
  else
  {
    hdc->pen.color = RGB(53, 95, 125);
    hdc->pen.size = LE_ACTIVE_EDGE_SIZE;
  }
  hdc->brush.color = RGB(225, 235, 235);
  pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
  //data->buf[data->index_y][data->index_x] = 0;
  hdc->font.color = RGB(132, 132, 132);
  int pos_row = 0;
  for (int i = data->sy; i <= data->rows; i++)
  {
    int per = 0;
    int l = data->buf[i].length;
    int t = l;
    while (l > 0)
    {
      int size;
      if (l > data->num)
      {
        size = data->num;
      }
      else
        size = l;
      l -= data->num;
      char * temp = (char *)malloc(sizeof(char) * (size + 1));
      for (int j = 0; j < size; j++)
      {
        temp[j] = data->buf[i].buf[j + per * data->num];
      }
      temp[size] = '\0';
      if(size)
        pvcDrawText(hdc, temp, LE_ACTIVE_EDGE_SIZE, FONT_SIZE_CY *
           (pos_row + (t - l) / data->num - 1));
      //printf(1, "%s %d\n", temp, pos_row);
      //++pos_row;
      ++per;
      free(temp);
    }
    ++pos_row;
  }
  hdc->pen.size = LES_CURSOR_SIZE;
  if (data->state == LES_ACTIVE_CURSORSHOW)
    hdc->pen.color = RGB(132, 132, 132);
  else
    hdc->pen.color = RGB(195, 235, 215);
    //pvcDrawLine(&multiLineEdit->dc, 10, 10, 100, 100);

  pvcDrawLine(hdc, LE_ACTIVE_EDGE_SIZE + FONT_SIZE_CX * (data->cursor_x),
      data->cursor_y * FONT_SIZE_CY + LE_ACTIVE_EDGE_SIZE, LE_ACTIVE_EDGE_SIZE + FONT_SIZE_CX *
        (data->cursor_x), data->cursor_y * FONT_SIZE_CY + FONT_SIZE_CY);
}
