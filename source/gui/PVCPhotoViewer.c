#include "PVCPhotoViewer.h"
struct photoNode* now;
PHdc hdcBmp;
PBitmap nBmp;
unsigned char * c;
int imgsize;
PBitmap loadJpg(char *filename)
{
	char ZZ[64] = { 0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18,
        11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
        42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45,
        38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63 };
    Context* ctx=malloc(sizeof(struct Context));
    memset(ctx, 0, sizeof(Context));

    int f;
    f = open(filename, O_RDONLY);
    unsigned char * buf = (unsigned char *)malloc(MAX_JPEG_FILE_SIZE/*size*/);

    read(f, buf, MAX_JPEG_FILE_SIZE/*size*/);
    
    close(f);

    _Decode(ctx, ZZ, buf, MAX_JPEG_FILE_SIZE/*size*/);

   
	PBitmap bmp;
	bmp.height=0;
	bmp.width=0;
	bmp.data=0;
	
	int imgsize = GetImageSize(ctx);
    	unsigned char * c = GetImage(ctx);
    	bmp.width = GetWidth(ctx);
    	bmp.height = GetHeight(ctx);
	int n = bmp.width * bmp.height;
    	bmp.data = (PColor *)malloc(sizeof(PColor) * n);
   	for (int i = 0; i < imgsize; i += 3)
   	{
      		  bmp.data[i / 3].r = c[i];
     	 	  bmp.data[i / 3].g = c[i + 1];
      		  bmp.data[i / 3].b = c[i + 2];
   	}
	return bmp;
}
void photoListInit(struct photoList* pl)
{
	pl->head=0;
	pl->nd=0;
	pl->tail=0;
}
void update(PHwnd hwnd)
{
	nBmp=loadImg(now->name);
	hdcBmp = pvcCreateCompatibleDCFromBitmap(nBmp);
}
int type(char *filename)
{
	int len=strlen(filename);
	if(filename[len-1]=='g'&&filename[len-2]=='p'&&filename[len-3]=='j') return JPG;
	if(filename[len-1]=='p'&&filename[len-2]=='m'&&filename[len-3]=='b') return BMP;
	else return NOTIMG;
}

PBitmap loadImg(char* filename)
{
	int t=type(filename);
	PBitmap pm;
	switch(t){
		
	case JPG:
	return loadJpg(filename);
	break;
	
	case BMP:
	return pvcLoadBitmap(filename);
	break;
	
	default :
	return pm;
	}
	
	
}

PHdc pvcRightRotatePdc(PHdc src)
{
	PHdc des = (PHdc)malloc(sizeof(PDc));
	des->pen.size = 0;
	des->pen.color = des->brush.color = des->font.color =COLOR_NULL;
	des->size.cx=src->size.cy;
	des->size.cy=src->size.cx;
	PColor* data = (PColor *)malloc(nBmp.width * nBmp.height * sizeof(PColor));
	for(int i=0;i<des->size.cx*des->size.cy;i++)
	{
		int l=i/src->size.cy;
		int r=i%src->size.cx;
		data[r*des->size.cx+des->size.cy-l].r=nBmp.data[i].r;
		data[r*des->size.cx+des->size.cy-l].g=nBmp.data[i].g;
		data[r*des->size.cx+des->size.cy-l].b=nBmp.data[i].b;
		
	}
	des->content=data;
	nBmp.data=data;
	return des;
}

PHdc pvcLeftRotatePdc(PHdc src)
{
	PHdc des = (PHdc)malloc(sizeof(PDc));
	des->pen.size = 0;
	des->pen.color = des->brush.color = des->font.color =COLOR_NULL;
	des->size.cx=src->size.cy;
	des->size.cy=src->size.cx;
	PColor* data = (PColor *)malloc(nBmp.width * nBmp.height * sizeof(PColor));
	for(int i=0;i<des->size.cx*des->size.cy;i++)
	{
		int l=i/src->size.cy;
		int r=i%src->size.cx;
		data[(des->size.cy-r-1)*des->size.cx+l].r=nBmp.data[i].r;
		data[(des->size.cy-r-1)*des->size.cx+l].g=nBmp.data[i].g;
		data[(des->size.cy-r-1)*des->size.cx+l].b=nBmp.data[i].b;
		
	}
	des->content=data;
	nBmp.data=data;
	return des;
}
PHdc enlarge(PHdc src)
{
	PHdc des= (PHdc) malloc(sizeof(PDc));
	des->pen.size=0;
	des->pen.color=des->brush.color=des->font.color=COLOR_NULL;
	des->size.cx=3*src->size.cx/2;
	des->size.cy=3*src->size.cy/2;	
	PColor* data=(PColor *)malloc(des->size.cx*des->size.cy*sizeof(PColor));
	for(int i=0;i<des->size.cx*des->size.cy;i++)
	{
		int l=i/des->size.cy;
		int r=i%des->size.cx;
		data[i].r=nBmp.data[(2*l/3)*src->size.cx+2*r/3].r;
		data[i].g=nBmp.data[(2*l/3)*src->size.cx+2*r/3].g;
		data[i].b=nBmp.data[(2*l/3)*src->size.cx+2*r/3].b;
	}
	des->content=data;
	nBmp.width=des->size.cx;
	nBmp.height=des->size.cy;
	nBmp.data=(PColor *)malloc(des->size.cx*des->size.cy*sizeof(PColor));
	nBmp.data=data;	
	
	return des;	
}
PHdc reduce(PHdc src)
{
	PHdc des= (PHdc) malloc(sizeof(PDc));
	des->pen.size=0;
	des->pen.color=des->brush.color=des->font.color=COLOR_NULL;
	des->size.cx=2*src->size.cx/3;
	des->size.cy=2*src->size.cy/3;	
	PColor* data=(PColor *)malloc(des->size.cx*des->size.cy*sizeof(PColor));
	for(int i=0;i<des->size.cx*des->size.cy;i++)
	{
		int l=i/des->size.cy;
		int r=i%des->size.cx;
		data[i].r=nBmp.data[(3*l/2)*src->size.cx+3*r/2].r;
		data[i].g=nBmp.data[(3*l/2)*src->size.cx+3*r/2].g;
		data[i].b=nBmp.data[(3*l/2)*src->size.cx+3*r/2].b;
	}
	des->content=data;
	nBmp.width=des->size.cx;
	nBmp.height=des->size.cy;
	nBmp.data=(PColor *)malloc(des->size.cx*des->size.cy*sizeof(PColor));
	nBmp.data=data;	
	
	return des;	
}
void addPhotoByFileName(char *s,struct photoList* pl)
{
	struct photoNode *n=(struct photoNode*)malloc(sizeof(struct photoNode));
	n->name=s;
	if(pl->tail==0)
	{
		pl->head=n;
		pl->tail=n;
		pl->head->next=0;
		pl->head->prev=0;
	}
	else{
		struct photoNode* f=pl->tail;
		pl->tail->next=n;
		pl->tail=n;
		pl->tail->prev=f;
		pl->tail->next=0;
	}
}
bool wndProc(PHwnd hwnd, PMessage msg)
{
	switch(msg.type)
	{
	case MSG_CREATE:
		init(hwnd);
		break;
	case MSG_PAINT:
		draw(hwnd);
		break;
	case MSG_KEY_DOWN:
		keydown(hwnd, msg);
		pvcInvalidate(hwnd);
		break;
	case MSG_WHEEL_UP:
		hdcBmp=enlarge(hdcBmp);
		pvcInvalidate(hwnd);
		break;
	default:
		break;
	}
	return pvcWndProc(hwnd, msg);
}
void keydown(PHwnd hwnd, PMessage msg)
{
	switch(msg.param)
	{
	case VK_UP:
		if(now->prev!=0) now=now->prev;
		update(hwnd);
		break;
		
	case VK_DOWN:
		if(now->next!=0) now=now->next;
		update(hwnd);
		break;
	case VK_LEFT:
		hdcBmp=pvcLeftRotatePdc(hdcBmp);
		break;
	case VK_RIGHT:
		hdcBmp=pvcRightRotatePdc(hdcBmp);
		break;
	case VK_ADD:
		hdcBmp=enlarge(hdcBmp);
		break;
	case VK_MINUS:
		hdcBmp=reduce(hdcBmp);
		break;
	default:
		break;
	}

}
void init(PHwnd hwnd)
{
	PL=(struct photoList* )malloc(sizeof(struct photoList));
	photoListInit(PL);
	addPhotoByFileName("test.jpg", PL);
	addPhotoByFileName("Snake.bmp", PL);
	addPhotoByFileName("Clock.bmp", PL);
	addPhotoByFileName("NotePad.bmp", PL);
	
	now=PL->head;
	nBmp=loadImg(now->name);
	hdcBmp=pvcCreateCompatibleDCFromBitmap(nBmp);
}

void draw(PHwnd hwnd)
{
	PHdc hdc = pvcGetDC(hwnd);
	pvcTransparentBit(
		hdc, 0, 0,
		hdcBmp, 0, 0, hdcBmp->size.cx, hdcBmp->size.cy,
		RGB(255, 255, 255)
		);
}

int main()
{
  PHwnd hwnd = pvcCreateWindow("PhotoViewer", 0, 100, 100, 500, 500);
  pvcWndExec(hwnd, wndProc);
  exit();
}
