#ifndef PVCDECODEJPEG_H
#define PVCDECODEJPEG_H

#define JPEG_DECODER_THROW(e) do { ctx->error = e; return; } while (0)
#define MAX_JPEG_FILE_SIZE 1000000

#include"PVC.h"
#include"types.h"
enum DecodeResult
{
    OK,        // decoding successful
    NotAJpeg,      // not a JPEG file
    Unsupported,   // unsupported format
    OutOfMemory,   // out of memory
    InternalError, // internal error
    SyntaxError,   // syntax error
    Internal_Finished, // used internally, will never be reported
};

typedef struct VlcCode {
    unsigned char bits, code;
}VlcCode;

typedef struct Component {
    int cid;
    int ssx, ssy;
    int width, height;
    int stride;
    int qtsel;
    int actabsel, dctabsel;
    int dcpred;
    unsigned char *pixels;
}Component;

typedef struct Context {
    int error;
    const unsigned char *pos;
    int size;
    int length;
    int width, height;
    int mbwidth, mbheight;
    int mbsizex, mbsizey;
    int ncomp;
    Component comp[3];
    int qtused, qtavail;
    unsigned char qtab[4][64];
    VlcCode vlctab[4][65536];
    int buf, bufbits;
    int block[64];
    int rstinterval;
    unsigned char *rgb;
}Context;

void loadJPEG(char * filename, int  imgsize,
    unsigned char *  oimg, int width, int height);
unsigned char * GetImage(Context* ctx);
 int GetWidth(Context* ctx);
 int GetHeight(Context* ctx);
uint GetImageSize(Context* ctx);

unsigned char _Clip(const int x);
void _RowIDCT(int* blk);
void _ColIDCT(const int* blk, unsigned char *out, int stride);
int _ShowBits(Context* ctx, int bits);
void _SkipBits(Context* ctx, int bits);
int _GetBits(Context* ctx, int bits);
void _ByteAlign(Context* ctx);
void _Skip(Context* ctx, int count);
void _DecodeLength(Context* ctx);
unsigned short _Decode16(const unsigned char *pos);
void _DecodeBlock(Context* ctx, char * ZZ, Component* c, unsigned char* out);
int _GetVLC(Context* ctx, VlcCode* vlc, unsigned char* code);
void _SkipMarker(Context* ctx);
unsigned char CF(const int x);
void _UpsampleH(Context* ctx, Component* c);
void _UpsampleV(Context* ctx, Component* c);

int _Decode(Context* ctx, char * ZZ, const unsigned char* jpeg, const int size);
void _DecodeSOF(Context* ctx);
void _DecodeDHT(Context* ctx);
void _DecodeDQT(Context* ctx);
void _DecodeDRI(Context* ctx);
void _DecodeScan(Context* ctx, char * ZZ);
void _Convert(Context* ctx);

#endif
