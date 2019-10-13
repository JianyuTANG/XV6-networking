#include "PVCDecodeJPEG.h"
 
#include "user.h"

void loadJPEG(char * filename, int  imgsize,
    unsigned char *  oimg, int  width, int  height)
{
    char ZZ[64] = { 0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18,
        11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
        42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45,
        38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63 };
    Context ctx;
    memset(&ctx, 0, sizeof(Context));

    // 文件读写需要修改
    int f;
    f = open(filename, O_RDONLY);
    if (f<0)
    {
        return;
    }
    unsigned char * buf = (unsigned char *)malloc(MAX_JPEG_FILE_SIZE/*size*/);

    read(f, buf, MAX_JPEG_FILE_SIZE/*size*/);
    printf(1,"load success");
    close(f);

    _Decode(&ctx, ZZ, buf, MAX_JPEG_FILE_SIZE/*size*/);

    imgsize = GetImageSize(&ctx);
    oimg = GetImage(&ctx);
    width = GetWidth(&ctx);
    height = GetHeight(&ctx);
}

unsigned char * GetImage(Context* ctx)
{
    return (ctx->ncomp == 1) ? ctx->comp[0].pixels : ctx->rgb;
}

 int GetWidth(Context* ctx)
{
    return ctx->width;
}

 int GetHeight(Context* ctx)
{
    return ctx->height;
}

uint GetImageSize(Context* ctx)
{
    return ctx->width * ctx->height * ctx->ncomp;
}

unsigned char _Clip(const int x)
{
    return (x < 0) ? 0 : ((x > 0xFF) ? 0xFF : (unsigned char)x);
}

enum {
    W1 = 2841,
    W2 = 2676,
    W3 = 2408,
    W5 = 1609,
    W6 = 1108,
    W7 = 565,
};

void _RowIDCT(int * blk)
{
    int x0, x1, x2, x3, x4, x5, x6, x7, x8;
    if (!((x1 = blk[4] << 11)
        | (x2 = blk[6])
        | (x3 = blk[2])
        | (x4 = blk[1])
        | (x5 = blk[7])
        | (x6 = blk[5])
        | (x7 = blk[3])))
    {
        blk[0] = blk[1] = blk[2] = blk[3] = blk[4] = blk[5] = blk[6] = blk[7] = blk[0] << 3;
        return;
    }
    x0 = (blk[0] << 11) + 128;
    x8 = W7 * (x4 + x5);
    x4 = x8 + (W1 - W7) * x4;
    x5 = x8 - (W1 + W7) * x5;
    x8 = W3 * (x6 + x7);
    x6 = x8 - (W3 - W5) * x6;
    x7 = x8 - (W3 + W5) * x7;
    x8 = x0 + x1;
    x0 -= x1;
    x1 = W6 * (x3 + x2);
    x2 = x1 - (W2 + W6) * x2;
    x3 = x1 + (W2 - W6) * x3;
    x1 = x4 + x6;
    x4 -= x6;
    x6 = x5 + x7;
    x5 -= x7;
    x7 = x8 + x3;
    x8 -= x3;
    x3 = x0 + x2;
    x0 -= x2;
    x2 = (181 * (x4 + x5) + 128) >> 8;
    x4 = (181 * (x4 - x5) + 128) >> 8;
    blk[0] = (x7 + x1) >> 8;
    blk[1] = (x3 + x2) >> 8;
    blk[2] = (x0 + x4) >> 8;
    blk[3] = (x8 + x6) >> 8;
    blk[4] = (x8 - x6) >> 8;
    blk[5] = (x0 - x4) >> 8;
    blk[6] = (x3 - x2) >> 8;
    blk[7] = (x7 - x1) >> 8;
}

void _ColIDCT(const int * blk, unsigned char * out, int stride)
{
    int x0, x1, x2, x3, x4, x5, x6, x7, x8;
    if (!((x1 = blk[8 * 4] << 8)
        | (x2 = blk[8 * 6])
        | (x3 = blk[8 * 2])
        | (x4 = blk[8 * 1])
        | (x5 = blk[8 * 7])
        | (x6 = blk[8 * 5])
        | (x7 = blk[8 * 3])))
    {
        x1 = _Clip(((blk[0] + 32) >> 6) + 128);
        for (x0 = 8; x0; --x0) {
            *out = (unsigned char)x1;
            out += stride;
        }
        return;
    }
    x0 = (blk[0] << 8) + 8192;
    x8 = W7 * (x4 + x5) + 4;
    x4 = (x8 + (W1 - W7) * x4) >> 3;
    x5 = (x8 - (W1 + W7) * x5) >> 3;
    x8 = W3 * (x6 + x7) + 4;
    x6 = (x8 - (W3 - W5) * x6) >> 3;
    x7 = (x8 - (W3 + W5) * x7) >> 3;
    x8 = x0 + x1;
    x0 -= x1;
    x1 = W6 * (x3 + x2) + 4;
    x2 = (x1 - (W2 + W6) * x2) >> 3;
    x3 = (x1 + (W2 - W6) * x3) >> 3;
    x1 = x4 + x6;
    x4 -= x6;
    x6 = x5 + x7;
    x5 -= x7;
    x7 = x8 + x3;
    x8 -= x3;
    x3 = x0 + x2;
    x0 -= x2;
    x2 = (181 * (x4 + x5) + 128) >> 8;
    x4 = (181 * (x4 - x5) + 128) >> 8;
    *out = _Clip(((x7 + x1) >> 14) + 128);  out += stride;
    *out = _Clip(((x3 + x2) >> 14) + 128);  out += stride;
    *out = _Clip(((x0 + x4) >> 14) + 128);  out += stride;
    *out = _Clip(((x8 + x6) >> 14) + 128);  out += stride;
    *out = _Clip(((x8 - x6) >> 14) + 128);  out += stride;
    *out = _Clip(((x0 - x4) >> 14) + 128);  out += stride;
    *out = _Clip(((x3 - x2) >> 14) + 128);  out += stride;
    *out = _Clip(((x7 - x1) >> 14) + 128);
}

int _ShowBits(Context* ctx, int bits)
{
    unsigned char newbyte;
    if (!bits) return 0;
    while (ctx->bufbits < bits) {
        if (ctx->size <= 0) {
            ctx->buf = (ctx->buf << 8) | 0xFF;
            ctx->bufbits += 8;
            continue;
        }
        newbyte = *ctx->pos++;
        ctx->size--;
        ctx->bufbits += 8;
        ctx->buf = (ctx->buf << 8) | newbyte;
        if (newbyte == 0xFF) {
            if (ctx->size) {
                unsigned char marker = *ctx->pos++;
                ctx->size--;
                switch (marker) {
                case 0:    break;
                case 0xD9: ctx->size = 0; break;
                default:
                    if ((marker & 0xF8) != 0xD0)
                        ctx->error = SyntaxError;
                    else {
                        ctx->buf = (ctx->buf << 8) | marker;
                        ctx->bufbits += 8;
                    }
                }
            }
            else
                ctx->error = SyntaxError;
        }
    }
    return (ctx->buf >> (ctx->bufbits - bits)) & ((1 << bits) - 1);
}

void _SkipBits(Context* ctx, int bits)
{
    if (ctx->bufbits < bits)
        (void) _ShowBits(ctx, bits);
    ctx->bufbits -= bits;
}

int _GetBits(Context* ctx, int bits)
{
    int res = _ShowBits(ctx, bits);
    _SkipBits(ctx, bits);
    return res;
}

void _ByteAlign(Context* ctx)
{
    ctx->bufbits &= 0xF8;
}

void _Skip(Context* ctx, int count)
{
    ctx->pos += count;
    ctx->size -= count;
    ctx->length -= count;
    if (ctx->size < 0) ctx->error = SyntaxError;
}

void _DecodeLength(Context* ctx)
{
    if (ctx->size < 2) JPEG_DECODER_THROW(SyntaxError);
    ctx->length = _Decode16(ctx->pos);
    if (ctx->length > ctx->size) JPEG_DECODER_THROW(SyntaxError);
    _Skip(ctx, 2);
}

unsigned short _Decode16(const unsigned char * pos) 
{
    return (pos[0] << 8) | pos[1];
}

void _DecodeBlock(Context* ctx, char * ZZ, Component * c, unsigned char * out)
{
    unsigned char code;
    int value, coef = 0;
    memset(ctx->block, 0, sizeof(ctx->block));
    c->dcpred += _GetVLC(ctx, &ctx->vlctab[c->dctabsel][0], 0);
    ctx->block[0] = (c->dcpred) * ctx->qtab[c->qtsel][0];
    do {
        value = _GetVLC(ctx, &ctx->vlctab[c->actabsel][0], &code);
        if (!code) break;  // EOB
        if (!(code & 0x0F) && (code != 0xF0)) JPEG_DECODER_THROW(SyntaxError);
        coef += (code >> 4) + 1;
        if (coef > 63) JPEG_DECODER_THROW(SyntaxError);
        ctx->block[(int)ZZ[coef]] = value * ctx->qtab[c->qtsel][coef];
    } while (coef < 63);
    for (coef = 0; coef < 64; coef += 8)
        _RowIDCT(&ctx->block[coef]);
    for (coef = 0; coef < 8; ++coef)
        _ColIDCT(&ctx->block[coef], &out[coef], c->stride);
}

int _GetVLC(Context* ctx, VlcCode * vlc, unsigned char * code)
{
    int value = _ShowBits(ctx, 16);
    int bits = vlc[value].bits;
    if (!bits) { ctx->error = SyntaxError; return 0; }
    _SkipBits(ctx, bits);
    value = vlc[value].code;
    if (code) *code = (unsigned char)value;
    bits = value & 15;
    if (!bits) return 0;
    value = _GetBits(ctx, bits);
    if (value < (1 << (bits - 1)))
        value += ((-1) << bits) + 1;
    return value;
}

void _SkipMarker(Context* ctx)
{
    _DecodeLength(ctx);
    _Skip(ctx, ctx->length);
}

enum {
    CF4A = (-9),
    CF4B = (111),
    CF4C = (29),
    CF4D = (-3),
    CF3A = (28),
    CF3B = (109),
    CF3C = (-9),
    CF3X = (104),
    CF3Y = (27),
    CF3Z = (-3),
    CF2A = (139),
    CF2B = (-11),
};

unsigned char CF(const int x)
{
    return _Clip((x + 64) >> 7);
}

void _UpsampleH(Context* ctx, Component * c)
{
    const int xmax = c->width - 3;
    unsigned char *out, *lin, *lout;
    int x, y;
    out = (unsigned char*)malloc((c->width * c->height) << 1);
    if (!out) JPEG_DECODER_THROW(OutOfMemory);
    lin = c->pixels;
    lout = out;
    for (y = c->height; y; --y) {
        lout[0] = CF(CF2A * lin[0] + CF2B * lin[1]);
        lout[1] = CF(CF3X * lin[0] + CF3Y * lin[1] + CF3Z * lin[2]);
        lout[2] = CF(CF3A * lin[0] + CF3B * lin[1] + CF3C * lin[2]);
        for (x = 0; x < xmax; ++x) {
            lout[(x << 1) + 3] = CF(CF4A * lin[x] + CF4B * lin[x + 1] + CF4C * lin[x + 2] + CF4D * lin[x + 3]);
            lout[(x << 1) + 4] = CF(CF4D * lin[x] + CF4C * lin[x + 1] + CF4B * lin[x + 2] + CF4A * lin[x + 3]);
        }
        lin += c->stride;
        lout += c->width << 1;
        lout[-3] = CF(CF3A * lin[-1] + CF3B * lin[-2] + CF3C * lin[-3]);
        lout[-2] = CF(CF3X * lin[-1] + CF3Y * lin[-2] + CF3Z * lin[-3]);
        lout[-1] = CF(CF2A * lin[-1] + CF2B * lin[-2]);
    }
    c->width <<= 1;
    c->stride = c->width;
    free(c->pixels);
    c->pixels = out;
}

void _UpsampleV(Context* ctx, Component * c)
{
    const int w = c->width, s1 = c->stride, s2 = s1 + s1;
    unsigned char *out, *cin, *cout;
    int x, y;
    out = (unsigned char*)malloc((c->width * c->height) << 1);
    if (!out) JPEG_DECODER_THROW(OutOfMemory);
    for (x = 0; x < w; ++x) {
        cin = &c->pixels[x];
        cout = &out[x];
        *cout = CF(CF2A * cin[0] + CF2B * cin[s1]);  cout += w;
        *cout = CF(CF3X * cin[0] + CF3Y * cin[s1] + CF3Z * cin[s2]);  cout += w;
        *cout = CF(CF3A * cin[0] + CF3B * cin[s1] + CF3C * cin[s2]);  cout += w;
        cin += s1;
        for (y = c->height - 3; y; --y) {
            *cout = CF(CF4A * cin[-s1] + CF4B * cin[0] + CF4C * cin[s1] + CF4D * cin[s2]);  cout += w;
            *cout = CF(CF4D * cin[-s1] + CF4C * cin[0] + CF4B * cin[s1] + CF4A * cin[s2]);  cout += w;
            cin += s1;
        }
        cin += s1;
        *cout = CF(CF3A * cin[0] + CF3B * cin[-s1] + CF3C * cin[-s2]);  cout += w;
        *cout = CF(CF3X * cin[0] + CF3Y * cin[-s1] + CF3Z * cin[-s2]);  cout += w;
        *cout = CF(CF2A * cin[0] + CF2B * cin[-s1]);
    }
    c->height <<= 1;
    c->stride = c->width;
    free(c->pixels);
    c->pixels = out;
}

int _Decode(Context* ctx, char * ZZ, const unsigned char * jpeg, const int size)
{
    ctx->pos = (const unsigned char*)jpeg;
    ctx->size = size & 0x7FFFFFFF;
    if (ctx->size < 2) return NotAJpeg;
    if ((ctx->pos[0] ^ 0xFF) | (ctx->pos[1] ^ 0xD8)) return NotAJpeg;
    _Skip(ctx, 2);
    while (!ctx->error) {
        if ((ctx->size < 2) || (ctx->pos[0] != 0xFF)) return SyntaxError;
        _Skip(ctx, 2);
        switch (ctx->pos[-1]) {
        case 0xC0: _DecodeSOF(ctx);  break;
        case 0xC4: _DecodeDHT(ctx);  break;
        case 0xDB: _DecodeDQT(ctx);  break;
        case 0xDD: _DecodeDRI(ctx);  break;
        case 0xDA: _DecodeScan(ctx, ZZ); break;
        case 0xFE: _SkipMarker(ctx); break;
        default:
            if ((ctx->pos[-1] & 0xF0) == 0xE0)
                _SkipMarker(ctx);
            else
                return Unsupported;
        }
    }
    if (ctx->error != Internal_Finished) return ctx->error;
    ctx->error = OK;
    _Convert(ctx);
    return ctx->error;
}

void _DecodeSOF(Context* ctx)
{
    int i, ssxmax = 0, ssymax = 0;
    Component* c;
    _DecodeLength(ctx);
    if (ctx->length < 9) JPEG_DECODER_THROW(SyntaxError);
    if (ctx->pos[0] != 8) JPEG_DECODER_THROW(Unsupported);
    ctx->height = _Decode16(ctx->pos + 1);
    ctx->width = _Decode16(ctx->pos + 3);
    ctx->ncomp = ctx->pos[5];
    _Skip(ctx, 6);
    switch (ctx->ncomp) {
    case 1:
    case 3:
        break;
    default:
        JPEG_DECODER_THROW(Unsupported);
    }
    if (ctx->length < (ctx->ncomp * 3)) JPEG_DECODER_THROW(SyntaxError);
    for (i = 0, c = ctx->comp; i < ctx->ncomp; ++i, ++c) {
        c->cid = ctx->pos[0];
        if (!(c->ssx = ctx->pos[1] >> 4)) JPEG_DECODER_THROW(SyntaxError);
        if (c->ssx & (c->ssx - 1)) JPEG_DECODER_THROW(Unsupported);  // non-power of two
        if (!(c->ssy = ctx->pos[1] & 15)) JPEG_DECODER_THROW(SyntaxError);
        if (c->ssy & (c->ssy - 1)) JPEG_DECODER_THROW(Unsupported);  // non-power of two
        if ((c->qtsel = ctx->pos[2]) & 0xFC) JPEG_DECODER_THROW(SyntaxError);
        _Skip(ctx, 3);
        ctx->qtused |= 1 << c->qtsel;
        if (c->ssx > ssxmax) ssxmax = c->ssx;
        if (c->ssy > ssymax) ssymax = c->ssy;
    }
    ctx->mbsizex = ssxmax << 3;
    ctx->mbsizey = ssymax << 3;
    ctx->mbwidth = (ctx->width + ctx->mbsizex - 1) / ctx->mbsizex;
    ctx->mbheight = (ctx->height + ctx->mbsizey - 1) / ctx->mbsizey;
    for (i = 0, c = ctx->comp; i < ctx->ncomp; ++i, ++c) {
        c->width = (ctx->width * c->ssx + ssxmax - 1) / ssxmax;
        c->stride = (c->width + 7) & 0x7FFFFFF8;
        c->height = (ctx->height * c->ssy + ssymax - 1) / ssymax;
        c->stride = ctx->mbwidth * ctx->mbsizex * c->ssx / ssxmax;
        if (((c->width < 3) && (c->ssx != ssxmax)) || ((c->height < 3) && (c->ssy != ssymax))) JPEG_DECODER_THROW(Unsupported);
        if (!(c->pixels = (unsigned char*)malloc(c->stride * (ctx->mbheight * ctx->mbsizey * c->ssy / ssymax)))) JPEG_DECODER_THROW(OutOfMemory);
    }
    if (ctx->ncomp == 3) {
        ctx->rgb = (unsigned char*)malloc(ctx->width * ctx->height * ctx->ncomp);
        if (!ctx->rgb) JPEG_DECODER_THROW(OutOfMemory);
    }
    _Skip(ctx, ctx->length);
}

void _DecodeDHT(Context* ctx)
{
    int codelen, currcnt, remain, spread, i, j;
    VlcCode *vlc;
    unsigned char counts[16];
    _DecodeLength(ctx);
    while (ctx->length >= 17) {
        i = ctx->pos[0];
        if (i & 0xEC) JPEG_DECODER_THROW(SyntaxError);
        if (i & 0x02) JPEG_DECODER_THROW(Unsupported);
        i = (i | (i >> 3)) & 3;  // combined DC/AC + tableid value
        for (codelen = 1; codelen <= 16; ++codelen)
            counts[codelen - 1] = ctx->pos[codelen];
        _Skip(ctx, 17);
        vlc = &ctx->vlctab[i][0];
        remain = spread = 65536;
        for (codelen = 1; codelen <= 16; ++codelen) {
            spread >>= 1;
            currcnt = counts[codelen - 1];
            if (!currcnt) continue;
            if (ctx->length < currcnt) JPEG_DECODER_THROW(SyntaxError);
            remain -= currcnt << (16 - codelen);
            if (remain < 0) JPEG_DECODER_THROW(SyntaxError);
            for (i = 0; i < currcnt; ++i) {
                register unsigned char code = ctx->pos[i];
                for (j = spread; j; --j) {
                    vlc->bits = (unsigned char)codelen;
                    vlc->code = code;
                    ++vlc;
                }
            }
            _Skip(ctx, currcnt);
        }
        while (remain--) {
            vlc->bits = 0;
            ++vlc;
        }
    }
    if (ctx->length) JPEG_DECODER_THROW(SyntaxError);
}

void _DecodeDQT(Context* ctx)
{
    int i;
    unsigned char *t;
    _DecodeLength(ctx);
    while (ctx->length >= 65) {
        i = ctx->pos[0];
        if (i & 0xFC) JPEG_DECODER_THROW(SyntaxError);
        ctx->qtavail |= 1 << i;
        t = &ctx->qtab[i][0];
        for (i = 0; i < 64; ++i)
            t[i] = ctx->pos[i + 1];
        _Skip(ctx, 65);
    }
    if (ctx->length) JPEG_DECODER_THROW(SyntaxError);
}

void _DecodeDRI(Context* ctx)
{
    _DecodeLength(ctx);
    if (ctx->length < 2) JPEG_DECODER_THROW(SyntaxError);
    ctx->rstinterval = _Decode16(ctx->pos);
    _Skip(ctx, ctx->length);

}

void _DecodeScan(Context* ctx, char * ZZ)
{
    int i, mbx, mby, sbx, sby;
    int rstcount = ctx->rstinterval, nextrst = 0;
    Component* c;
    _DecodeLength(ctx);
    if (ctx->length < (4 + 2 * ctx->ncomp)) JPEG_DECODER_THROW(SyntaxError);
    if (ctx->pos[0] != ctx->ncomp) JPEG_DECODER_THROW(Unsupported);
    _Skip(ctx, 1);
    for (i = 0, c = ctx->comp; i < ctx->ncomp; ++i, ++c) {
        if (ctx->pos[0] != c->cid) JPEG_DECODER_THROW(SyntaxError);
        if (ctx->pos[1] & 0xEE) JPEG_DECODER_THROW(SyntaxError);
        c->dctabsel = ctx->pos[1] >> 4;
        c->actabsel = (ctx->pos[1] & 1) | 2;
        _Skip(ctx, 2);
    }
    if (ctx->pos[0] || (ctx->pos[1] != 63) || ctx->pos[2]) JPEG_DECODER_THROW(Unsupported);
    _Skip(ctx, ctx->length);
    for (mby = 0; mby < ctx->mbheight; ++mby)
        for (mbx = 0; mbx < ctx->mbwidth; ++mbx) {
            for (i = 0, c = ctx->comp; i < ctx->ncomp; ++i, ++c)
                for (sby = 0; sby < c->ssy; ++sby)
                    for (sbx = 0; sbx < c->ssx; ++sbx) {
                        _DecodeBlock(ctx, ZZ, c, &c->pixels[((mby * c->ssy + sby) * c->stride + mbx * c->ssx + sbx) << 3]);
                        if (ctx->error)
                            return;
                    }
            if (ctx->rstinterval && !(--rstcount)) {
                _ByteAlign(ctx);
                i = _GetBits(ctx, 16);
                if (((i & 0xFFF8) != 0xFFD0) || ((i & 7) != nextrst)) JPEG_DECODER_THROW(SyntaxError);
                nextrst = (nextrst + 1) & 7;
                rstcount = ctx->rstinterval;
                for (i = 0; i < 3; ++i)
                    ctx->comp[i].dcpred = 0;
            }
        }
    ctx->error = Internal_Finished;
}

void _Convert(Context* ctx)
{
    int i;
    Component* c;
    for (i = 0, c = ctx->comp; i < ctx->ncomp; ++i, ++c) {
        while ((c->width < ctx->width) || (c->height < ctx->height)) {
            if (c->width < ctx->width) _UpsampleH(ctx, c);
            if (ctx->error) return;
            if (c->height < ctx->height) _UpsampleV(ctx, c);
            if (ctx->error) return;
        }
        if ((c->width < ctx->width) || (c->height < ctx->height)) JPEG_DECODER_THROW(InternalError);
    }
    if (ctx->ncomp == 3) {
        // convert to RGB
        int x, yy;
        unsigned char *prgb = ctx->rgb;
        const unsigned char *py = ctx->comp[0].pixels;
        const unsigned char *pcb = ctx->comp[1].pixels;
        const unsigned char *pcr = ctx->comp[2].pixels;
        for (yy = ctx->height; yy; --yy) {
            for (x = 0; x < ctx->width; ++x) {
                register int y = py[x] << 8;
                register int cb = pcb[x] - 128;
                register int cr = pcr[x] - 128;
                *prgb++ = _Clip((y + 359 * cr + 128) >> 8);
                *prgb++ = _Clip((y - 88 * cb - 183 * cr + 128) >> 8);
                *prgb++ = _Clip((y + 454 * cb + 128) >> 8);
            }
            py += ctx->comp[0].stride;
            pcb += ctx->comp[1].stride;
            pcr += ctx->comp[2].stride;
        }
    }
    else if (ctx->comp[0].width != ctx->comp[0].stride) {
        // grayscale -> only remove stride
        unsigned char *pin = &ctx->comp[0].pixels[ctx->comp[0].stride];
        unsigned char *pout = &ctx->comp[0].pixels[ctx->comp[0].width];
        int y;
        for (y = ctx->comp[0].height - 1; y; --y) {
            memmove(pout, pin, ctx->comp[0].width);
            pin += ctx->comp[0].stride;
            pout += ctx->comp[0].width;
        }
        ctx->comp[0].stride = ctx->comp[0].width;
    }
}
