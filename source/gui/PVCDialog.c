#include "types.h"
#include "user.h"
#include "PVCDialog.h"
#include "PVCWindow.h"
#include "PVCControl.h"
#include "PVCPainter.h"

PHdlg pvcCreateDialog(PHwnd caller, char * title, PHwnd parent, int x, int y, int width, int height, void * data)
{
  PHdlg r = (PHdlg)malloc(sizeof(PDialog));
  r->caller = caller;
  r->hwnd = pvcCreateWindow(title, parent, x, y, width, height);
  r->data = data;
  r->hwnd->msgQueueId = caller->msgQueueId;
  r->shakeCount = -1;
  return r;
}

bool pvcDlgProc(PHdlg hdlg, PMessage msg)
{
  switch (msg.type)
  {
  case MSG_WNDID_WRONG:
  {
    switch (msg.wndId)
    {
    case MSG_RECTPAINT:
    case MSG_LOSE_FOCUS:
    case MSG_PAINT:
      msg.type = msg.wndId;
      msg.wndId = hdlg->caller->id;
      if (pvcWndPreTranslateMessage(hdlg->caller, &msg))
        if (pvcDispatchMsgToCtrlLst(&hdlg->caller->ctrlLst, msg))
          if (pvcWndTranslateMessage(hdlg->caller, &msg))
            hdlg->caller->wndProc(hdlg->caller, msg);
      break;
    case MSG_LBUTTON_UP:
    case MSG_LBUTTON_DOWN:
    case MSG_LBUTTON_DCLK:
    case MSG_RBUTTON_UP:
    case MSG_RBUTTON_DCLK:
    case MSG_MBUTTON_UP:
    case MSG_MBUTTON_DOWN:
    case MSG_MBUTTON_DCLK:
    case MSG_WHEEL_UP:
    case MSG_WHEEL_DOWN:
    case MSG_KEY_UP:
    case MSG_KEY_DOWN:
      if (hdlg->shakeCount < 0)
        setTimer(hdlg->hwnd, DLG_SHAKE_TIMER_ID, 500);
      hdlg->shakeCount = 2;
      break;
    }
  }
    break;
  case MSG_TIMEOUT:
  {
    if (((msg.param >> 16) & 0xffff) == DLG_SHAKE_TIMER_ID)
    {
      printf(1, "MSG_TIMEOUT %d\n", hdlg->shakeCount);
      --hdlg->shakeCount;
      if (hdlg->shakeCount < 0)
      {
        killTimer(hdlg->hwnd, DLG_SHAKE_TIMER_ID);
        msg.type = MSG_HAS_FOCUS;
        pvcSendMessage(hdlg->hwnd, msg);
      }
      if (hdlg->hwnd->focusState == WFS_HAS_FOCUS)
      {
        msg.type = MSG_LOSE_FOCUS;
        pvcSendMessage(hdlg->hwnd, msg);
      }
      else if (hdlg->hwnd->focusState == WFS_LOSE_FOCUS)
      {
        msg.type = MSG_HAS_FOCUS;
        pvcSendMessage(hdlg->hwnd, msg);
      }
    }
  }
    break;
  }
  return pvcWndProc(hdlg->hwnd, msg);
}

void pvcDlgExec(PHdlg hdlg, bool (*dlgProc)(PHdlg, PMessage))
{
  hdlg->caller->focusState = WFS_CHILD_FOCUS;

  registWindow(hdlg->hwnd);
  PMessage msg;
  msg.type = MSG_CREATE;
  pvcSendMessage(hdlg->hwnd, msg);
  msg.type = MSG_PAINT;
  msg.param = WPT_WHOLEDC;
  pvcSendMessage(hdlg->hwnd, msg);
  while (1)
  {
    getMessage(hdlg->hwnd);
    if (pvcWndPreTranslateMessage(hdlg->hwnd, &hdlg->hwnd->msg))
      if (pvcDispatchMsgToCtrlLst(&hdlg->hwnd->ctrlLst, hdlg->hwnd->msg))
        if (pvcWndTranslateMessage(hdlg->hwnd, &hdlg->hwnd->msg))
          if (dlgProc(hdlg, hdlg->hwnd->msg))
          {
            break;
          }
    hdlg->hwnd->msg.type = MSG_NULL;
  }

  hdlg->caller->focusState = WFS_LOSE_FOCUS;
  msg.type = MSG_HAS_FOCUS;
  pvcSendMessage(hdlg->caller, msg);
}

void pvcDestroyDialog(PHdlg hdlg)
{
  free(hdlg);
}

void pvcAcceptCall(void * param)
{
  PHdlg hdlg = (PHdlg)param;
  PMessageBoxData * data = (PMessageBoxData *)hdlg->data;
  data->result = MB_ACCEPT;
  PMessage msg;
  msg.type = MSG_DESTROY;
  pvcSendMessage(hdlg->hwnd, msg);
}

void pvcRejectCall(void * param)
{
  PHdlg hdlg = (PHdlg)param;
  PMessageBoxData * data = (PMessageBoxData *)hdlg->data;
  data->result = MB_REJECT;
  PMessage msg;
  msg.type = MSG_DESTROY;
  pvcSendMessage(hdlg->hwnd, msg);
}

bool pvcMessageBoxProc(PHdlg hdlg, PMessage msg)
{
  PHwnd hwnd = hdlg->hwnd;
  PMessageBoxData * data = (PMessageBoxData *)hdlg->data;
  switch (msg.type)
  {
  case MSG_CREATE:
    pvcCreateButton(hwnd, "Yes", (hwnd->wholeDc.size.cx - 2 * MB_BUTTON_WIDTH) / 4,
      hwnd->wholeDc.size.cy - MB_VEC_LEFTOUT - MB_BUTTON_HEIGHT, MB_BUTTON_WIDTH,
      MB_BUTTON_HEIGHT, (void *)hdlg, pvcAcceptCall, pvcPaintButton);
    pvcCreateButton(hwnd, "No", hwnd->wholeDc.size.cx / 2 + (hwnd->wholeDc.size.cx - 2 * MB_BUTTON_WIDTH) / 4,
      hwnd->wholeDc.size.cy - MB_VEC_LEFTOUT - MB_BUTTON_HEIGHT, MB_BUTTON_WIDTH,
      MB_BUTTON_HEIGHT, (void *)hdlg, pvcRejectCall, pvcPaintButton);
      break;
  case MSG_PAINT:
  {
    PHdc hdc = pvcGetDC(hdlg->hwnd);
    hdc->brush.color = RGB(235, 235, 245);
    hdc->pen.size = 0;
    hdc->pen.color = COLOR_NULL;
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
    hdc->font.color = RGB(10, 10, 10);
    pvcDrawText(hdc, data->content, (hwnd->dc.size.cx - data->col * FONT_SIZE_CX) / 2,
      (hwnd->dc.size.cy - MB_BUTTON_HEIGHT - data->row * FONT_SIZE_CY) / 2);
  }
    break;
  default:
    break;
  }
  return pvcDlgProc(hdlg, msg);
}

int pvcMessageBox(PHwnd caller, char * title, char * content)
{
  PMessageBoxData data;
  data.content = content;
  data.row = 1;
  data.col = 0;
  int col = 0;
  for (int i = 0; content[i]; ++i)
  {
    if (content[i] == '\n')
    {
      ++data.row;
      if (col > data.col)
        data.col = col;
      col = 0;
    }
    ++col;
  }
  if (col > data.col)
    data.col = col;
  int w = MB_HOR_LEFTOUT * 2 + data.col * FONT_SIZE_CX;
  if (w < 2 * MB_BUTTON_WIDTH)
    w = 2 * MB_BUTTON_WIDTH;
  int h = MB_VEC_LEFTOUT * 3 + MB_BUTTON_HEIGHT + data.row * FONT_SIZE_CY + WND_TITLE_HEIGHT;
  int x = caller->pos.x + caller->wholeDc.size.cx / 2 - w / 2;
  int y = caller->pos.y + caller->wholeDc.size.cy / 2 - h / 2;
  PHdlg hdlg = pvcCreateDialog(caller, title, 0, x, y, w, h, (void *)&data);
  pvcDlgExec(hdlg, pvcMessageBoxProc);
  pvcDestroyDialog(hdlg);
  return data.result;
}

void pvcGetInputDialogLineEditReturn()
{

}

void pvcGetInputDialogAcceptCall(void * param)
{
  PHdlg hdlg = (PHdlg)param;
  PGetInputDialogData * data = (PGetInputDialogData *)hdlg->data;
  data->result = MB_ACCEPT;
  PMessage msg;
  msg.type = MSG_DESTROY;
  pvcSendMessage(hdlg->hwnd, msg);
}

void pvcGetInputDialogRejectCall(void * param)
{
  PHdlg hdlg = (PHdlg)param;
  PGetInputDialogData * data = (PGetInputDialogData *)hdlg->data;
  data->result = MB_REJECT;
  PMessage msg;
  msg.type = MSG_DESTROY;
  pvcSendMessage(hdlg->hwnd, msg);
}

bool pvcGetInputDialogProc(PHdlg hdlg, PMessage msg)
{
  static PHLineEdit lineEdit = 0;
  PHwnd hwnd = hdlg->hwnd;
  PGetInputDialogData * data = (PGetInputDialogData *)hdlg->data;
  switch (msg.type)
  {
  case MSG_PAINT:
  {
    PHdc hdc = pvcGetDC(hwnd);
    hdc->brush.color = RGB(235, 235, 235);
    hdc->pen.size = 0;
    hdc->pen.color = COLOR_NULL;
    pvcDrawRect(hdc, 0, 0, hdc->size.cx, hdc->size.cy);
  }
    break;
  case MSG_CREATE:
    pvcCreateButton(hwnd, "Yes", (hwnd->wholeDc.size.cx - 2 * MB_BUTTON_WIDTH) / 4,
      hwnd->wholeDc.size.cy - MB_VEC_LEFTOUT - MB_BUTTON_HEIGHT, MB_BUTTON_WIDTH,
      MB_BUTTON_HEIGHT, (void *)hdlg, pvcGetInputDialogAcceptCall, pvcPaintButton);
    pvcCreateButton(hwnd, "No", hwnd->wholeDc.size.cx / 2 + (hwnd->wholeDc.size.cx - 2 * MB_BUTTON_WIDTH) / 4,
      hwnd->wholeDc.size.cy - MB_VEC_LEFTOUT - MB_BUTTON_HEIGHT, MB_BUTTON_WIDTH,
      MB_BUTTON_HEIGHT, (void *)hdlg, pvcGetInputDialogRejectCall, pvcPaintButton);
    lineEdit = pvcCreateLineEdit(hwnd, MB_VEC_LEFTOUT, MB_VEC_LEFTOUT + WND_TITLE_HEIGHT, hwnd->wholeDc.size.cx - 2 * MB_VEC_LEFTOUT, 30,
      data->len, pvcGetInputDialogLineEditReturn, pvcPaintLineEdit);
      break;
  case MSG_DESTROY:
    pvcGetLineEditString(lineEdit, data->buf);
    break;
  default:
    break;
  }
  return pvcDlgProc(hdlg, msg);
}

int pvcGetInputDialog(PHwnd caller, char * title, char * buf, int len)
{
  PGetInputDialogData data;
  data.buf = buf;
  data.len = len;
  int w = MB_HOR_LEFTOUT * 2 + 200;
  int h = MB_VEC_LEFTOUT * 3 + MB_BUTTON_HEIGHT + 30 + WND_TITLE_HEIGHT;
  int x = caller->pos.x + caller->wholeDc.size.cx / 2 - w / 2;
  int y = caller->pos.y + caller->wholeDc.size.cy / 2 - h / 2;
  PHdlg hdlg = pvcCreateDialog(caller, title, 0, x, y, w, h, (void *)&data);
  pvcDlgExec(hdlg, pvcGetInputDialogProc);
  pvcDestroyDialog(hdlg);
  return data.result;
}
