#include "PVCNotePad.h"

void startFile(PHwnd hwnd)
{
	if (strlen(file_tobe_open) != 0)
	{
		showFile(hwnd, file_tobe_open);
		file_tobe_open[0] = 0;
	}

	return;
}

void showFile(PHwnd hwnd, char * buf)
{
	int file = open(buf, O_RDONLY);
	if (file < 0)
	{
		pvcMessageBox(hwnd, "Error", "Open File failed!");
		return;
	}

	saved = false;
	PMultiLineEditData * data = (PMultiLineEditData *)multiLineEdit->data;

	for (int i = 0; i <= data->rows_size; ++i)
		data->buf[i].length = 0;

	data->rows = 0;
	data->cursor_x = data->cursor_y = 0;
	data->position_x = data->position_y = 0;
	data->state = LES_INACTIVE;
	data->sy = 0;
	data->rows_all = 0;

	while (1)
	{
		int l = read(file, buf, sizeof(char) * 512);
		if (l <= 0)
			break;
		for (int i = 0; i < 512; ++i)
		{
			if (buf[i] == '\n')
			{
				++data->rows;
				if (data->rows + 1 >= data->rows_size)
				{
					printf(1, "file too long\n");
					close(file);
					return;
				}
				continue;
			}
			if (data->buf[data->rows].length >= data->buf[data->rows].size)
			{
				data->buf[data->rows].size *= 2;
				char * temp = (char *)malloc(sizeof(char) * data->buf[data->rows].size);
				for (int j = 0; j < data->buf[data->rows].length; ++j)
					temp[j] = data->buf[data->rows].buf[j];
				free(data->buf[data->rows].buf);
				data->buf[data->rows].buf = temp;
			}
			data->buf[data->rows].buf[data->buf[data->rows].length++] = buf[i];
		}
		//printf(1, "%s", buf);
	}
	close(file);
}

void deleteFile(void * param)
{
  unlink((char *)param);
  ListItem(".");
}
void AddrReturn()
{
  char buf[1000];
  pvcGetLineEditString(addrEdit, buf);
  ListItem(buf);
}

char* FmtName(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  strcpy(buf, p);
  return buf;
}

//create scan list£¬user can scan the text file in XV6
void ListItem(char *path)
{
	char buf[MAX_ADDRESS_LEN], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if ((fd = open(path, 0)) < 0) {
		printf(2, "ls: cannot open %s\n", path);
		return;
	}

	if (fstat(fd, &st) < 0) {
		printf(2, "ls: cannot stat %s\n", path);
		close(fd);
		return;
	}

	switch (st.type) {
	case T_FILE:
	{
		char * temp = FmtName(path);
		int l = strlen(temp);
		printf(1, "%d %s\n", l, temp);
		if (l > 3 &&
			temp[0] == 'P' &&
			temp[1] == 'V' &&
			temp[2] == 'C' &&
			strcmp(temp, "PVCDesktop") != 0)
		{
			if (fork() > 0)
			{
				char * argv[] = { temp };
				exec(temp, argv);
				exit();
			}
		}
		else if (l != 0)
		{
			char* cmds[] = { temp, 0 };
			// Run notepad with temp as filename.
			runAppWithCmds("NotePad", cmds);
		}
		else
		{
			sprintf(buf, "Cannot Open %s", temp);
			//printf(1, "%s\n", buf);
			pvcMessageBox(itemListCtrl->hwnd, "Error", buf);
		}
		return;
	}
	//printf(1, "%s %d %d %d\n", FmtName(path), st.type, st.ino, st.size);
	break;

	case T_DIR:
		if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
			printf(1, "ls: path too long\n");
			break;
		}
		ItemListDestroy(&itemList);
		strcpy(buf, path);
		p = buf + strlen(buf);
		*p++ = '/';
		while (read(fd, &de, sizeof(de)) == sizeof(de)) {
			if (de.inum == 0)
				continue;
			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;
			if (stat(buf, &st) < 0) {
				printf(1, "ls: cannot stat %s\n", buf);
				continue;
			}
			ItemListNode t;
			strcpy(t.name, FmtName(buf));
			if (t.name[0] == '.')
				continue;
			t.type = 0;
			ItemListAdd(&itemList, &t);
			//printf(1, "%s %d %d %d\n", FmtName(buf), st.type, st.ino, st.size);
		}
		break;
	}
	close(fd);
	int showNum = itemListCtrl->dc.size.cy / ITEM_SIZE_CY;
	double scale;
	if (itemList.num <= showNum)
		scale = 1;
	else
		scale = showNum / (double)itemList.num;
	pvcSliderBarSetRange(sliderBar, 0, itemList.num - showNum);
	pvcSliderBarSetScale(sliderBar, scale);
	pvcSliderBarSetPos(sliderBar, 0);
	pvcCtrlInvalidate(sliderBar);
	pvcSetLineEditString(addrEdit, path);
	pvcCtrlInvalidate(itemListCtrl);
}

//init list
void ItemListInit(ItemList * list)
{
  list->len = 0;
  list->num = 0;
  list->data = 0;
  list->mouse = -1;
  list->select = -1;
  list->startIndex = 0;
}
///Listnode add
void ItemListAdd(ItemList * list, ItemListNode * data)
{
  if (list->num >= list->len)
  {
    ItemListNode * t = list->data;
    if (list->len == 0)
      list->len = 1;
    else
      list->len *= 2;
    list->data = (ItemListNode *)malloc(sizeof(ItemListNode) * list->len);
    for (int i = 0; i < list->num; ++i)
    {
      strcpy(list->data[i].name, t[i].name);
      list->data[i].type = t[i].type;
    }
    if (t != 0)
      free(t);
  }
  strcpy(list->data[list->num].name, data->name);
  list->data[list->num].type = data->type;
  ++list->num;
}
//destroy list
void ItemListDestroy(ItemList * list)
{
  if (list->data)
    free(list->data);
  list->data = 0;
  list->num = list->len = 0;
  list->mouse = list->select = -1;
  list->startIndex = 0;
}
//paint item
void ItemListPaintItem(ItemList * itemList, int index, PHControl itemListCtrl)
{
  if (index < 0 || index >= itemList->num)
    return;
  if (index - itemList->startIndex < 0 || index - itemList->startIndex >= itemListCtrl->dc.size.cy / ITEM_SIZE_CY)
    return;
  PHdc hdc = &itemListCtrl->dc;
  memset(hdc->content + (index - itemList->startIndex) * ITEM_SIZE_CY * hdc->size.cx,
    241, sizeof(PColor) * ITEM_SIZE_CY * hdc->size.cx);
  hdc->pen.size = 2;
  hdc->pen.color = RGB(125, 125, 235);
  hdc->brush.color = COLOR_NULL;
  hdc->font.color = RGB(65, 65, 85);
  pvcDrawText(hdc, itemList->data[index].name, ITEM_SIZE_CY, (index - itemList->startIndex) * ITEM_SIZE_CY + (ITEM_SIZE_CY - FONT_SIZE_CY) / 2);
  if (index == itemList->mouse)
    pvcDrawRect(hdc, 0, (index - itemList->startIndex) * ITEM_SIZE_CY, hdc->size.cx, ITEM_SIZE_CY);
  if (index == itemList->select)
  {
    for (int y = 0; y < ITEM_SIZE_CY; ++y)
    {
      for (int x = 0; x < hdc->size.cx; ++x)
      {
        PColor c = hdc->content[(y + (index - itemList->startIndex) * ITEM_SIZE_CY) * hdc->size.cx + x];
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
        hdc->content[(y + (index - itemList->startIndex) * ITEM_SIZE_CY) * hdc->size.cx + x] = c;
      }
    }
  }


    for (int y = 0; y < ITEM_SIZE_CY; ++y)
    {
      for (int x = 0; x < itemListCtrl->dc.size.cx; ++x)
      {
        PControlList * list = &itemListCtrl->hwnd->ctrlLst;
        PControlListNode * p = list->tail;
        while (p && p->ctrl != itemListCtrl)
        {
          if (ctrlContain(p->ctrl, itemListCtrl->pos.x + x, itemListCtrl->pos.y + y))
            break;
          p = p->prev;
        }
        if (p->ctrl == itemListCtrl)
        {
          pvcSetPixel(&itemListCtrl->hwnd->wholeDc, itemListCtrl->pos.x + x, itemListCtrl->pos.y + ITEM_SIZE_CY * (index - itemList->startIndex) + y,
            pvcPixel(&itemListCtrl->dc, x, ITEM_SIZE_CY * (index - itemList->startIndex) + y));
        }
        else
        {
          //printf(1, "1");
        }
      }
    }

  paintWindow(itemListCtrl->hwnd, itemListCtrl->pos.x, itemListCtrl->pos.y + ITEM_SIZE_CY * (index - itemList->startIndex),
    &itemListCtrl->hwnd->wholeDc, itemListCtrl->pos.x, itemListCtrl->pos.y + ITEM_SIZE_CY * (index - itemList->startIndex),
    itemListCtrl->dc.size.cx, ITEM_SIZE_CY);
}

bool ItemListProc(PHControl itemListCtrl, PMessage msg)
{
  ItemList * itemList = (ItemList *)itemListCtrl->data;
  switch (msg.type)
  {
  case MSG_PAINT:
  {
    if (msg.param == WPT_WHOLEDC)
      return SUBMIT;
    PHdc hdc = &itemListCtrl->dc;
    memset(hdc->content, 241, sizeof(PColor) * hdc->size.cx * hdc->size.cy);
    hdc->pen.size = 2;
    hdc->pen.color = RGB(125, 125, 235);
    hdc->brush.color = COLOR_NULL;
    hdc->font.color = RGB(65, 65, 85);
    for (int i = itemList->startIndex; i < itemList->num && i < itemList->startIndex + hdc->size.cy / ITEM_SIZE_CY; ++i)
    {
      pvcDrawText(hdc, itemList->data[i].name, ITEM_SIZE_CY, (i - itemList->startIndex) * ITEM_SIZE_CY + (ITEM_SIZE_CY - FONT_SIZE_CY) / 2);
      if (i == itemList->mouse)
        pvcDrawRect(hdc, 0, (i - itemList->startIndex) * ITEM_SIZE_CY, hdc->size.cx, ITEM_SIZE_CY);
      if (i == itemList->select)
      {
        for (int y = 0; y < ITEM_SIZE_CY; ++y)
        {
          for (int x = 0; x < hdc->size.cx; ++x)
          {
            PColor c = hdc->content[(y + (i - itemList->startIndex) * ITEM_SIZE_CY) * hdc->size.cx + x];
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
            hdc->content[(y + (i - itemList->startIndex) * ITEM_SIZE_CY) * hdc->size.cx + x] = c;
          }
        }
      }
    }
  }
    break;
  case MSG_MOUSE_MOVE:
  {
    int y = msg.param & 0xffff;
    y /= ITEM_SIZE_CY;
    y += itemList->startIndex;
    if (y < itemList->num)
    {
      if (itemList->mouse != y)
      {
        int t = itemList->mouse;
        itemList->mouse = y;
        ItemListPaintItem(itemList, t, itemListCtrl);
        ItemListPaintItem(itemList, y, itemListCtrl);
      }
    }
    else
    {
      if (itemList->mouse != -1)
      {
        int t = itemList->mouse;
        itemList->mouse = -1;
        ItemListPaintItem(itemList, t, itemListCtrl);
      }
    }
  }
    return SUBMIT;
  case MSG_LBUTTON_DOWN:
  {
    int y = msg.param & 0xffff;
    y /= ITEM_SIZE_CY;
    y += itemList->startIndex;
    if (y < itemList->num)
    {
      if (itemList->select != y)
      {
        int t = itemList->select;
        itemList->select = y;
        ItemListPaintItem(itemList, t, itemListCtrl);
        ItemListPaintItem(itemList, y, itemListCtrl);
      }
      else
      {
        //ListItem(itemList->data[itemList->select].name);
      }
    }
    else
    {
      if (itemList->select != -1)
      {
        int t = itemList->select;
        itemList->select = -1;
        ItemListPaintItem(itemList, t, itemListCtrl);
      }
    }
  }
    return FINISH;
  case MSG_LBUTTON_DCLK:
  {
    int y = msg.param & 0xffff;
    y /= ITEM_SIZE_CY;
    y += itemList->startIndex;
    if (y < itemList->num)
    {
      char buf[MAX_ADDRESS_LEN];
      char r[MAX_ADDRESS_LEN];
      pvcGetLineEditString(addrEdit, buf);
      sprintf(r, "%s/%s", buf, itemList->data[y].name);
      ListItem(r);
    }
  }
    return FINISH;
  case MSG_RBUTTON_UP:
  {
    int y = msg.param & 0xffff;
    y /= ITEM_SIZE_CY;
    y += itemList->startIndex;
    if (y < itemList->num)
    {
      PHMenu menu = pvcCreateMenu(itemListCtrl->hwnd, true);
      pvcMenuAddItem(menu, "delete", itemList->data[y].name, deleteFile);
      y = msg.param & 0xffff;
      int x = (msg.param >> 16) & 0xffff;
      pvcShowMenuAt(menu, x + itemListCtrl->pos.x, y + itemListCtrl->pos.y);
    }
  }
    return FINISH;
  /*case MSG_MOUSE_OUT:
  case MSG_LOSE_FOCUS:
  {
    int t = itemList->mouse;
    itemList->mouse = -1;
    ItemListPaintItem(itemList, t, itemListCtrl);
  }
    return FINISH;*/
  }
  return pvcCtrlProc(itemListCtrl, msg);
}


void allFile(void * param)
{
    PHwnd hwnd = (PHwnd)param;
    addrEdit = pvcCreateLineEdit(hwnd, hwnd->clientPos.x + 60, WND_MENUBAR_HEIGHT + hwnd->clientPos.y + 2, hwnd->dc.size.cx - 62, 30,
      MAX_ADDRESS_LEN, AddrReturn, pvcPaintLineEdit);
    ItemListInit(&itemList);
    const int sliderBarWidth = 16;
    itemListCtrl = pvcCreateControl(hwnd, hwnd->clientPos.x, hwnd->clientPos.y + WND_MENUBAR_HEIGHT + 34, hwnd->dc.size.cx - sliderBarWidth, hwnd->dc.size.cy - 34 - WND_MENUBAR_HEIGHT,
      &itemList, ItemListProc);
    sliderBar = pvcCreateSliderBar(hwnd, SLIDER_ID, hwnd->clientPos.x + hwnd->dc.size.cx - sliderBarWidth, hwnd->clientPos.y + 40 + WND_MENUBAR_HEIGHT, sliderBarWidth, hwnd->dc.size.cy - 40 - WND_MENUBAR_HEIGHT,
      0, 1, 1, SBT_VECTICAL, pvcPaintSliderBar);
    ListItem(".");
    pvcInvalidate(hwnd);    
}

void openFile(void * param)
{
  PHwnd hwnd = (PHwnd)param;
  char buf[512];
  PMultiLineEditData * data = (PMultiLineEditData *)multiLineEdit->data;
  if (MB_ACCEPT != pvcGetInputDialog(hwnd, "Input name", buf, 512))
    return;
  int file = open(buf, O_RDONLY);
  if (file < 0)
  {
    pvcMessageBox(hwnd, "Error", "Open File failed!");
    return;
  }

  for (int i = 0; i <= data->rows_size; ++i)
    data->buf[i].length = 0;

  data->rows = 0;
  data->cursor_x = data->cursor_y = 0;
  data->position_x = data->position_y = 0;
  data->state = LES_INACTIVE;
  data->sy = 0;
  data->rows_all = 0;

  while (1)
  {
    int l = read(file, buf, sizeof(char) * 512);
    if (l <= 0)
      break;
    for (int i = 0; i < 512; ++i)
    {
      if (buf[i] == '\n')
      {
        ++data->rows;
        if (data->rows + 1 >= data->rows_size)
        {
          printf(1, "file too long\n");
          close(file);
          return;
        }
        continue;
      }
      if (data->buf[data->rows].length >= data->buf[data->rows].size)
      {
        data->buf[data->rows].size *= 2;
        char * temp = (char *)malloc(sizeof(char) * data->buf[data->rows].size);
        for (int j = 0; j < data->buf[data->rows].length; ++j)
          temp[j] = data->buf[data->rows].buf[j];
        free(data->buf[data->rows].buf);
        data->buf[data->rows].buf = temp;
      }
      data->buf[data->rows].buf[data->buf[data->rows].length++] = buf[i];
    }
    //printf(1, "%s", buf);
  }
  close(file);
}

void newFile(void * param)
{
  PHwnd hwnd = (PHwnd)param;
  multiLineEdit = pvcCreateMultiLineEdit(hwnd, hwnd->clientPos.x, hwnd->clientPos.y + WND_MENUBAR_HEIGHT,
    hwnd->dc.size.cx, hwnd->dc.size.cy - WND_MENUBAR_HEIGHT, pvcPaintMultiLineEdit);
}

void saveFile(void * param)
{
  PHwnd hwnd = (PHwnd)param;
  PMultiLineEditData * data = (PMultiLineEditData *)multiLineEdit->data;
  char buf[512];
  if (MB_ACCEPT != pvcGetInputDialog(hwnd, "Input name", buf, 512))
    return;
  int file = open(buf, O_WRONLY | O_CREATE);
  if (file < 0)
  {
    pvcMessageBox(hwnd, "Error", "Save File failed!");
    return;
  }
  //printf(1, "%d\n", data->rows);
  for (int i = 0; i <= data->rows; ++i)
  {
    write(file, data->buf[i].buf, sizeof(char) * data->buf[i].length);
    write(file, "\n", sizeof(char));
  }

  saved = true;
  write(file, "\0", sizeof(char));
  close(file);
}

//void closeWindow(void * hwnd)
//{
//	if (saved || MB_ACCEPT == pvcMessageBox(hwnd, "NotePad", "Quit before saving?"))
//	{
//		if (MB_ACCEPT == pvcMessageBox(hwnd, "NotePad", "Do you want to quit?"))
//		{
//			pvcCloseWindow(hwnd);
//			return;
//		}
//	}
//}

bool wndProc(PHwnd hwnd, PMessage msg)
{
  switch (msg.type)
  {
  case MSG_CREATE:
  {
    PHMenu menu = pvcCreateMenu(hwnd, false);
    PHMenu openmenu = pvcCreateMenu(hwnd, false);
    PHMenu allTextmenu = pvcCreateMenu(hwnd, false);

    pvcMenuAddMenu(menu, "open", openmenu);
    pvcMenuAddMenu(openmenu, "allText", allTextmenu);

    pvcMenuAddItem(menu, "save", (void *)hwnd, saveFile);
    pvcMenuAddItem(menu, "close", (void *)hwnd, pvcCloseWindow);
    pvcMenuAddItem(openmenu,"open", (void *)hwnd, openFile);
    pvcMenuAddItem(allTextmenu,"scanTextInXV6", (void *)hwnd, allFile);
    pvcMenuAddItem(allTextmenu,"return", (void *)hwnd, newFile);

    PHMenuBar menuBar = pvcCreateMenuBar(hwnd);
    pvcMenuBarAddMenu(menuBar, "file", menu);

    multiLineEdit = pvcCreateMultiLineEdit(hwnd, hwnd->clientPos.x, hwnd->clientPos.y + WND_MENUBAR_HEIGHT,
      hwnd->dc.size.cx, hwnd->dc.size.cy - WND_MENUBAR_HEIGHT, pvcPaintMultiLineEdit);

	startFile(hwnd);
  }
		break;
  case MSG_DESTROY:
  {
	  if (saved || MB_ACCEPT == pvcMessageBox(hwnd, "NotePad", "Quit before saving?"))
	  {
		  if (MB_ACCEPT == pvcMessageBox(hwnd, "NotePad", "Do you want to quit?"))
		  {
			  break;
		  }
	  }
	  return FINISH;
  }
  case MSG_PAINT:
  {
    PHdc hdc = pvcGetDC(hwnd);
    if (msg.param == WPT_WHOLEDC)
    {
      memset(hdc->content, 0x0c, hdc->size.cx * hdc->size.cy * sizeof(PColor));
      break;
    }
    memset(hdc->content + hdc->size.cx * WND_MENUBAR_HEIGHT, 241, sizeof(PColor) * hdc->size.cx * 50);
    hdc->font.color = RGB(0, 0, 0);
    pvcDrawText(hdc, "Text in XV6", 3, WND_MENUBAR_HEIGHT + 8);
  }
    break;
  case MSG_SLIDER_POS_CHANGE:
  {
    int id = (msg.param >> 16) & 0xffff;
    int pos = msg.param & 0xffff;
    if (id == SLIDER_ID)
    {
      if (pos != itemList.startIndex)
      {
        itemList.startIndex = pos;
        pvcCtrlInvalidate(itemListCtrl);
      }
    }
    break;
  }
  case MSG_KEY_DOWN:
  {
    switch (msg.param)
    {
    case VK_UP:
    {
      if (itemList.select > 0)
      {
        --itemList.select;
        if (itemList.select < itemList.startIndex)
        {
          itemList.startIndex = itemList.select;
          pvcSliderBarSetPos(sliderBar, itemList.startIndex);
          pvcCtrlInvalidate(sliderBar);
        }
      }
      pvcCtrlInvalidate(itemListCtrl);
    }
      break;
    case VK_DOWN:
    {
      if (itemList.select < itemList.num - 1)
      {
        ++itemList.select;
        if (itemList.select - itemList.startIndex >= itemListCtrl->dc.size.cy / ITEM_SIZE_CY - 1)
        {
          itemList.startIndex = itemList.select - itemListCtrl->dc.size.cy / ITEM_SIZE_CY + 1;
          pvcSliderBarSetPos(sliderBar, itemList.startIndex);
          pvcCtrlInvalidate(sliderBar);
        }
        pvcCtrlInvalidate(itemListCtrl);
      }
    }
      break;
    }
  }
  default:
    break;
  }
  return pvcWndProc(hwnd, msg);
}

int main(int argc, char* argv[])
{
  PHwnd hwnd = pvcCreateWindow("NotePad", 0, 150, 150, NOTEPAD_WIDTH, NOTEPAD_HEIGHT);

  // Passing the second argument as file name
  if (argc >= 2)
  {
	  int i = 0;
	  for (; argv[1][i] != 0; i++)
	  {
		  file_tobe_open[i] = argv[1][i];
	  }
	  file_tobe_open[i] = 0;
  }

  pvcWndExec(hwnd, wndProc);
  exit();
}
