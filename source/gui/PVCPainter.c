#include "types.h"
#include "user.h"
#include "math.h"
#include "PVCPainter.h"
#include "PVCLib.h"
#include "math.h"
#include "fcntl.h"

#define OFFSET_X_SMALL -1
#define OFFSET_Y_SMALL -2
#define OFFSET_X_BIG   -3
#define OFFSET_Y_BIG   -4

static inline int pointOffset(PHdc hdc, int x, int y)
{
	if (x < 0)
		return OFFSET_X_SMALL;
	if (y < 0)
	 	return OFFSET_Y_SMALL;
	if (x >= hdc->size.cx)
		return OFFSET_X_BIG;
	if (y >= hdc->size.cy)
		return OFFSET_Y_BIG;
	return y * hdc->size.cx + x;;
}

static inline int dist2(int x1, int y1, int x2, int y2)
{
	return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

static inline int absInt(int x)
{
	return x < 0? -x : x;
}

static inline double absDouble(double x)
{
	return x < 0? -x : x;
}

static inline bool isValidColor(PColor color)
{
	return (color.r != COLOR_NULL_ALPHA || color.g != COLOR_NULL_ALPHA || color.b != COLOR_NULL_ALPHA);
}

PHdc pvcGetDC(PHwnd hwnd)
{
	return &(hwnd->dc);
}

PHdc pvcGetWholeDC(PHwnd hwnd)
{
	return &(hwnd->wholeDc);
}

PHdc pvcCreateCompatibleDC(PHdc hdc)
{
	PHdc r = (PHdc)malloc(sizeof(PDc));
	if (r == 0)
		pvcError(0);
	r->size = hdc->size;
	r->pen = hdc->pen;
	r->brush = hdc->brush;
	r->font = hdc->font;
	int s = r->size.cx * r->size.cy;
	if (s < 0)
	{
		s = 0;
		r->content = 0;
	}
	else
	{
		r->content = (PColor *)malloc(sizeof(PColor) * s);
		if (r->content == 0)
			pvcError(0);
		memset(r->content, 0, sizeof(PColor) * s);
	}
	return r;
}

void pvcDeleteDC(PHdc hdc)
{
	free(hdc->content);
	free(hdc);
}

void pvcBltbit(PHdc buf, int x, int y, PHdc src, int sx, int sy, int w, int h)
{
	if (sx < 0 || sy < 0 || x < 0 || y < 0)
		return;
	int i, j;
	int t = src->size.cx - sx;
	w = BOUND(0, w, t);
	t = buf->size.cx - x;
	w = BOUND(0, w, t);
	t = src->size.cy - sy;
	h = BOUND(0, h, t);
	t = buf->size.cy - y;
	h = BOUND(0, h, t);

	for (i = 0; i < h; ++i)
	{
		if (y + i < 0)
		{
			i = 0 - y - 1;
			continue;
		}
		else if (sy + i < 0)
		{
			i = 0 - sy - 1;
			continue;
		}
		int off_buf = (y + i) * buf->size.cx + x;
		int off_src = (sy + i) * src->size.cx + sx;
		for (j = 0; j < w; ++j)
		{
			if (x + j < 0)
			{
				j = 0 - x - 1;
				continue;
			}
			else if (sx + j < 0)
			{
				j = 0 - sx - 1;
				continue;
			}
			PColor c = src->content[off_src + j];
			if (isValidColor(c))
				buf->content[off_buf + j] = c;
		}
	}
}

PPen pvcSetPen(PHdc hdc, PPen pen)
{
	PPen r = hdc->pen;
	hdc->pen = pen;
	return r;
}

PBrush pvcSetBrush(PHdc hdc, PBrush brush)
{
	PBrush r = hdc->brush;
	hdc->brush = brush;
	return r;
}

PFont pvcSetFont(PHdc hdc, PFont font)
{
	PFont r = hdc->font;
	hdc->font = font;
	return r;
}

void pvcSetPixel(PHdc hdc, int x, int y, PColor color)
{
	if (!isValidColor(color))
		return;
	int off = pointOffset(hdc, x, y);
	if (off < 0)
		return;
	hdc->content[off] = color;
}

PColor pvcPixel(PHdc hdc, int x, int y)
{
	int off = pointOffset(hdc, x, y);
	if (off < 0)
		return COLOR_NULL;
	return hdc->content[off];
}

void pvcDrawPoint(PHdc hdc, int x, int y)
{
	if (!isValidColor(hdc->pen.color))
		return;
	int i, j, t;
	int off = hdc->pen.size / 2;
	int d2 = off * off;
	i = y - off;
	if (i < 0)
		i = 0;
	for (; i <= y + off; ++i)
	{
		j = x - off;
		if (j < 0)
			j = 0;
		for (; j <= x + off; ++j)
		{
			t = pointOffset(hdc, j, i);
			if (t == OFFSET_X_BIG)
				break;
			if (t == OFFSET_Y_BIG)
				return;
			if (dist2(j, i, x, y) <= d2)
				hdc->content[t] = hdc->pen.color;
		}
	}
}

void pvcDrawLine(PHdc hdc, int x1, int y1, int x2, int y2)
{
	if (!isValidColor(hdc->pen.color))
		return;
	if (x1 == x2)
	{
		if (x1 < 0 || x1 >= hdc->size.cx)
			return;
		if (y1 > y2)
		{
			int t = y1;
			y1 = y2;
			y2 = t;
		}
		if (y1 < 0)
			y1 = 0;
		if (y2 >= hdc->size.cy)
			y2 = hdc->size.cy - 1;
		int y;
		for (y = y1; y <= y2; ++y)
			pvcDrawPoint(hdc, x1, y);
		return;
	}
	if (y1 == y2)
	{
		if (y1 < 0 || y1 >= hdc->size.cy)
			return;
		if (x1 > x2)
		{
			int t = x1;
			x1 = x2;
			x2 = t;
		}
		if (x1 < 0)
			x1 = 0;
		if (x2 >= hdc->size.cx)
			x2 = hdc->size.cx - 1;
		int x;
		for (x = x1; x <= x2; ++x)
			pvcDrawPoint(hdc, x, y1);
		return;
	}
	if (absInt(x1 - x2) > absInt(y1 - y2))
	{
		double a = (double)(y1 - y2) / (double)(x1 - x2);
		int s, e;
		if (x1 > x2)
		{
			s = x2;
			e = x1;
		}
		else
		{
			s = x1;
			e = x2;
		}
		if (s < 0)
			s = 0;
		if (e >= hdc->size.cx)
			e = hdc->size.cx - 1;
		int x;
		for (x = s; x <= e; ++x)
		{
			int y = (int)(a * (x - x2) + y2);
			pvcDrawPoint(hdc, x, y);
		}
		return;
	}
	double a = (double)(x1 - x2) / (double)(y1 - y2);
	int s, e;
	if (y1 > y2)
	{
		s = y2;
		e = y1;
	}
	else
	{
		s = y1;
		e = y2;
	}
	if (s < 0)
		s = 0;
	if (e >= hdc->size.cy)
		e = hdc->size.cy - 1;
	int y;
	for (y = s; y <= e; ++y)
	{
		int x = (int)(a * (y - y2) + x2);
		pvcDrawPoint(hdc, x, y);
	}
}

void pvcDrawRect(PHdc hdc, int x, int y, int w, int h)
{
	if (isValidColor(hdc->pen.color))
	{
		pvcDrawLine(hdc, x, y, x + w - 1, y);
		pvcDrawLine(hdc, x + w - 1, y, x + w - 1, y + h - 1);
		pvcDrawLine(hdc, x + w - 1, y + h - 1, x, y + h - 1);
		pvcDrawLine(hdc, x, y + h - 1, x, y);
	}

	if (isValidColor(hdc->brush.color))
	{
		w += x;
		h += y;
		x += hdc->pen.size / 2;
		y += hdc->pen.size / 2;
		w -= hdc->pen.size / 2 + 1;
		h -= hdc->pen.size / 2 + 1;
		if (x < 0)
			x = 0;
		if (w >= hdc->size.cx)
			w = hdc->size.cx - 1;
		if (h >= hdc->size.cy)
			h = hdc->size.cy - 1;
		if (y < 0)
			y = 0;
		if (x >= hdc->size.cx || y >= hdc->size.cy)
			return;
		int i, j;
		for (i = y; i <= h; ++i)
		{
			int offBase = i * hdc->size.cx;
			for (j = x; j <= w; ++j)
				hdc->content[offBase + j] = hdc->brush.color;
		}
	}
}

//void pvcFillRect(PHdc hdc, int x, int y, int w, int h);

//void pvcFrameRect(PHdc hdc, int x, int y, int w, int h);

void pvcDrawEllipse(PHdc hdc, int x, int y, int w, int h)
{
	int a = w / 2;
	int b = h / 2;
	int fy = -1;
	for (int sx = -a; sx <= a + 1; ++sx)
	{
		float t = b * b - sx * sx * h * h / (float)(w * w);
		int dy;
		if (t < 1)
			dy = 0;
		else if (t < 4)
			dy = 1;
		else
			dy = sqrt(t);
		int k = MAX(fy, dy);
		for (int i = MIN(fy, dy) + 1; i < k; ++i)
		{
			pvcDrawPoint(hdc, x + a + sx, y + b - i);
			pvcDrawPoint(hdc, x + a + sx, y + b + i);
		}
		k = MIN(fy, dy);
		pvcDrawPoint(hdc, x + a + sx, y + b - k);
		pvcDrawPoint(hdc, x + a + sx, y + b + k);
		k -= 1;
		if (isValidColor(hdc->brush.color))
			for (int sy = -k; sy <= k; ++sy)
				pvcSetPixel(hdc, x + a + sx, y + b + sy, hdc->brush.color);
		fy = dy;
	}
	/*double a = w / 2;
	double b = h / 2;
	const double DL = 0.03;
	int i, j;
	for (i = -b; i <= b; ++i)
	{
		for (j = -a; j <= a; ++j)
		{
			double l = (double)(j * j) / (a * a) + (double)(i * i) / (b * b);
			if (absDouble(l - 1) <= DL)
			{
				if (isValidColor(hdc->pen.color))
				{
					pvcDrawPoint(hdc, x + a + j, y + b + i);
				}
			}
			if (1 - l >= DL)
			{
				if (isValidColor(hdc->brush.color))
				{
					pvcSetPixel(hdc, x + a + j, y + b + i, hdc->brush.color);
				}
			}
		}
	}*/
}

void pvcFloodFill(PHdc hdc, int x, int y, PColor color, PFlag type)
{

}

void pvcDrawText(PHdc hdc, char * str, int x, int y)
{
	int l = strlen(str);
	char * buf = (char *)malloc(sizeof(char) * l * FONT_SIZE_CY);
	getStringFigure(str, buf, l);
	int row = 0;
	int col = 0;
	for (int k = 0; k < l; ++k)
	{
		if (str[k] == '\n')
		{
			row += FONT_SIZE_CY + 2;
			col = 0;
			continue;
		}
		else if (str[k] == '\t')
		{
			col += 4 * FONT_SIZE_CX;
			continue;
		}
		for (int i = 0; i < FONT_SIZE_CY; ++i)
		{
			char ch = buf[(k << 4) + i];
			for (int j = 0; j < FONT_SIZE_CX; ++j)
			{
				if (ch < 0)
					pvcSetPixel(hdc, x + col + j, y + i + row, hdc->font.color);
				ch <<= 1;
			}
		}
		col += FONT_SIZE_CX;
	}
	free(buf);
}

PBitmap pvcLoadBitmap(char * filename)
{
	PBitmap bmp;
	bmp.width = bmp.height = 0;
	bmp.data = 0;
	int fd = open(filename, O_RDONLY);
  if (fd < 0)
  {
    printf(1, "Cannot open %s\n", filename);
    return bmp;
  }
	PBitmapFileHeader fileHeader;
	read(fd, (char *)&fileHeader, sizeof(fileHeader));
	//printf(1, "%x %d %d %d %d\n", fileHeader.bfType, fileHeader.bfSize, fileHeader.bfOffBits, fileHeader.bfReserved1, fileHeader.bfReserved2);
	PBitmapInfoHeader infoHeader;
	read(fd, (char *)&infoHeader, sizeof(infoHeader));
	bmp.width = infoHeader.biWidth;
	bmp.height = infoHeader.biHeight;
	bmp.data = (PColor *)malloc(bmp.width * bmp.height * sizeof(PColor));
	int c = infoHeader.biBitCount;
	//printf(1, "%d %d %d\n", w, h, c);
	int l = (((bmp.width * c) + 31) >> 5) << 2;
	int s = l * bmp.height;
	unsigned char * data = (unsigned char *)malloc(sizeof(unsigned char) * s);
	read(fd, (char *)data, sizeof(unsigned char) * s);
	int bit = infoHeader.biBitCount / 8;
	for (int j = 0; j < bmp.height; ++j)
	{
		int off = (bmp.height - j - 1) * bmp.width;
		for (int i = 0; i < bmp.width; ++i)
		{
			bmp.data[off + i].r = (int)data[j * l + bit * i + bit - 1];
			bmp.data[off + i].g = (int)data[j * l + bit * i + bit - 2];
			bmp.data[off + i].b = (int)data[j * l + bit * i + bit - 3];
		}
	}
	close(fd);
	return bmp;
}

PHdc pvcCreateCompatibleDCFromBitmap(PBitmap bmp)
{
	PHdc hdc = (PHdc)malloc(sizeof(PDc));
	hdc->pen.size = 0;
	hdc->pen.color = hdc->brush.color = hdc->font.color = COLOR_NULL;
	hdc->size.cx = bmp.width;
	hdc->size.cy = bmp.height;
	hdc->content = bmp.data;
	//printf(1, "%d %d\n", hdc->size.cx, hdc->size.cy);
	return hdc;
}

void pvcTransparentBit(PHdc buf, int x, int y, PHdc src, int sx, int sy, int w, int h, PColor tspt)
{
	int i, j;
	int t = src->size.cx - sx;
	w = BOUND(0, w, t);
	t = buf->size.cx - x;
	w = BOUND(0, w, t);
	t = src->size.cy - sy;
	h = BOUND(0, h, t);
	t = buf->size.cy - y;
	h = BOUND(0, h, t);

	for (i = 0; i < h; ++i)
	{
		if (y + i < 0)
		{
			i = 0 - y - 1;
			continue;
		}
		else if (sy + i < 0)
		{
			i = 0 - sy - 1;
			continue;
		}
		int off_buf = (y + i) * buf->size.cx + x;
		int off_src = (sy + i) * src->size.cx + sx;
		for (j = 0; j < w; ++j)
		{
			if (x + j < 0)
			{
				j = 0 - x - 1;
				continue;
			}
			else if (sx + j < 0)
			{
				j = 0 - sx - 1;
				continue;
			}
			PColor c = src->content[off_src + j];
			if (isValidColor(c) && (c.r != tspt.r || c.g != tspt.g || c.b != tspt.b))
				buf->content[off_buf + j] = c;
		}
	}
	/*int i, j;
	for (i = 0; i < h; ++i)
	{
		for (j = 0; j < w; ++j)
		{
			PColor c = pvcPixel(src, sx + j, sy + i);
			if (c.r != tspt.r || c.g != tspt.g || c.b != tspt.b)
				pvcSetPixel(buf, x + j, y + i, c);
		}
	}*/
}

void pvcError(int index)
{
	return;
}
