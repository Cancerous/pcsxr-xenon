/***************************************************************************
                          texture.c  -  description
                             -------------------
    begin                : Sun Mar 08 2009
    copyright            : (C) 1999-2009 by Pete Bernert
    web                  : www.pbernert.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////////
// Texture related functions are here !
//
// The texture handling is heart and soul of this gpu. The plugin was developed
// 1999, by this time no shaders were available. Since the psx gpu is making
// heavy use of CLUT (="color lookup tables", aka palettized textures), it was
// an interesting task to get those emulated at good speed on NV TNT cards
// (which was my major goal when I created the first "gpuPeteTNT"). Later cards
// (Geforce256) supported texture palettes by an OGL extension, but at some point
// this support was dropped again by gfx card vendors.
// Well, at least there is a certain advatage, if no texture palettes extension can
// be used: it is possible to modify the textures in any way, allowing "hi-res"
// textures and other tweaks.
//
// My main texture caching is kinda complex: the plugin is allocating "n" 256x256 textures,
// and it places small psx texture parts inside them. The plugin keeps track what
// part (with what palette) it had placed in which texture, so it can re-use this
// part again. The more ogl textures it can use, the better (of course the managing/
// searching will be slower, but everything is faster than uploading textures again
// and again to a gfx card). My first card (TNT1) had 16 MB Vram, and it worked
// well with many games, but I recommend nowadays 64 MB Vram to get a good speed.
//
// Sadly, there is also a second kind of texture cache needed, for "psx texture windows".
// Those are "repeated" textures, so a psx "texture window" needs to be put in
// a whole texture to use the GL_TEXTURE_WRAP_ features. This cache can get full very
// fast in games which are having an heavy "texture window" usage, like RRT4. As an
// alternative, this plugin can use the OGL "palette" extension on texture windows,
// if available. Nowadays also a fragment shader can easily be used to emulate
// texture wrapping in a texture atlas, so the main cache could hold the texture
// windows as well (that's what I am doing in the OGL2 plugin). But currently the
// OGL1 plugin is a "shader-free" zone, so heavy "texture window" games will cause
// much texture uploads.
//
// Some final advice: take care if you change things in here. I've removed my ASM
// handlers (they didn't cause much speed gain anyway) for readability/portability,
// but still the functions/data structures used here are easy to mess up. I guess it
// can be a pain in the ass to port the plugin to another byte order :)
//
////////////////////////////////////////////////////////////////////////////////////
void XeTexImage(int internalformat, int width, int height, int format,const void *pixels);
#define xeSetTexture()
#define _IN_TEXTURE

#define XE_TEXF_POINT 0
#define XE_TEXF_LINEAR 1

#include "externals.h"
#include "texture.h"
#include "gpu.h"
#include "prim.h"
#include "xe.h"


#define CLUTCHK   0x00060000
#define CLUTSHIFT 17

// crappy
#define glTexImage2D
#define glColorTableEXTEx 0
int glGetError(){
    return 0;
}

struct XenosSurface *T_CreateTexture(struct XenosDevice *xe, unsigned int width, unsigned int height, unsigned int levels, int format, int tiled);

#define Xe_CreateTexture T_CreateTexture

struct PSXUV gPsxUV;//offset
void XeTexSubImage(int xoffset, int yoffset, int width, int height, const void * buffer){
    struct XenosSurface * surf = gTexName;
    if(surf){
        //copy data
       // printf("xeGfx_setTextureData\n");
        uint8_t * surfbuf =(uint8_t*)Xe_Surface_LockRect(xe,surf,0,0,0,0,XE_LOCK_WRITE);
        uint8_t * srcdata = (uint8_t*)buffer;
        uint8_t * dstdata = surfbuf;
        int srcbytes = 4;
	int dstbytes = 4;
        int y,x;
        int j,i = 0;
/*

        surf->width = width;
        surf->height = height;
*/

        int pitch = surf->wpitch;

        for (y = yoffset; y < (yoffset + height); y++)
	{
            //dstdata = surfbuf + (y * width + xoffset) * dstbytes;
            //srcdata = buffer + ((surf->width)*(y))+x;
            //dstdata = surfbuf + (y-yoffset)*(surf->wpitch);// ok
            dstdata = surfbuf + (y*pitch)+(xoffset*dstbytes);// ok
            //srcdata = buffer + ((surf->width)*(y))+xoffset;
            for (x = xoffset; x < (xoffset + width); x++)
            {
                // 0 a r
                // 1 r g
                // 2 g b
                // 3 b a
/*
                dstdata[0] = srcdata[3];
                dstdata[1] = srcdata[0];
                dstdata[2] = srcdata[1];
                dstdata[3] = srcdata[2];
*/

                dstdata[0] = srcdata[0];
                dstdata[1] = srcdata[1];
                dstdata[2] = srcdata[2];
                dstdata[3] = srcdata[3];


                srcdata += srcbytes;
                dstdata += dstbytes;
            }
        }

        Xe_Surface_Unlock(xe,surf);
    }
    else{
        printf("not found\r\n");
    }
}

////////////////////////////////////////////////////////////////////////
// texture conversion buffer ..
////////////////////////////////////////////////////////////////////////

int iHiResTextures = 0;
GLubyte ubPaletteBuffer[256][4];
struct XenosSurface * gTexMovieName = 0;
struct XenosSurface * gTexBlurName = 0;
struct XenosSurface * gTexFrameName = 0;
int iTexGarbageCollection = 1;
uint32_t dwTexPageComp = 0;
int iVRamSize = 0;
//int iClampType = GL_CLAMP;
int iClampType = XE_TEXADDR_CLAMP;

void (*LoadSubTexFn) (int, int, short, short);
uint32_t(*PalTexturedColourFn) (uint32_t);

////////////////////////////////////////////////////////////////////////
// defines
////////////////////////////////////////////////////////////////////////

#define PALCOL(x) PalTexturedColourFn (x)

#define CSUBSIZE  2048
#define CSUBSIZEA 8192
#define CSUBSIZES 4096

#define OFFA 0
#define OFFB 2048
#define OFFC 4096
#define OFFD 6144

#define XOFFA 0
#define XOFFB 512
#define XOFFC 1024
#define XOFFD 1536

#define SOFFA 0
#define SOFFB 1024
#define SOFFC 2048
#define SOFFD 3072

#define MAXWNDTEXCACHE 128

#define XCHECK(pos1,pos2) ((pos1.c[0]>=pos2.c[1])&&(pos1.c[1]<=pos2.c[0])&&(pos1.c[2]>=pos2.c[3])&&(pos1.c[3]<=pos2.c[2]))
#define INCHECK(pos2,pos1) ((pos1.c[0]<=pos2.c[0]) && (pos1.c[1]>=pos2.c[1]) && (pos1.c[2]<=pos2.c[2]) && (pos1.c[3]>=pos2.c[3]))

////////////////////////////////////////////////////////////////////////

unsigned char * CheckTextureInSubSCache(int TextureMode, uint32_t GivenClutId, unsigned short *pCache);
void LoadSubTexturePageSort(int pageid, int mode, short cx, short cy);
void LoadPackedSubTexturePageSort(int pageid, int mode, short cx, short cy);
void DefineSubTextureSort(void);

////////////////////////////////////////////////////////////////////////
// some globals
////////////////////////////////////////////////////////////////////////

GLint giWantedRGBA = 4;
GLint giWantedFMT = XE_FMT_BGRA;
GLint giWantedTYPE = XE_FMT_8888;
int GlobalTexturePage;
GLint XTexS;
GLint YTexS;
GLint DXTexS;
GLint DYTexS;
int iSortTexCnt = 32;
BOOL bUseFastMdec = FALSE;
BOOL bUse15bitMdec = FALSE;
int iFrameTexType = 0;
int iFrameReadType = 0;

uint32_t(*TCF[2]) (uint32_t);
unsigned short (*PTCF[2]) (unsigned short);

////////////////////////////////////////////////////////////////////////
// texture cache implementation
////////////////////////////////////////////////////////////////////////

// "texture window" cache entry
typedef struct textureWndCacheEntryTag {
    uint32_t ClutID;
    short pageid;
    short textureMode;
    short Opaque;
    short used;
    EXLong pos;
    struct XenosSurface * texname;
} textureWndCacheEntry;

// "standard texture" cache entry (12 byte per entry, as small as possible... we need lots of them)

typedef struct textureSubCacheEntryTagS {
    uint32_t ClutID;
    EXLong pos;
    unsigned char posTX;
    unsigned char posTY;
    unsigned char cTexID;
    unsigned char Opaque;
} textureSubCacheEntryS;

//---------------------------------------------

#define MAXTPAGES_MAX  64
#define MAXSORTTEX_MAX 196

//---------------------------------------------

textureWndCacheEntry wcWndtexStore[MAXWNDTEXCACHE];
textureSubCacheEntryS * pscSubtexStore[3][MAXTPAGES_MAX];
EXLong * pxSsubtexLeft [MAXSORTTEX_MAX];
struct XenosSurface * uiStexturePage[MAXSORTTEX_MAX];

unsigned short usLRUTexPage = 0;

int iMaxTexWnds = 0;
int iTexWndTurn = 0;
int iTexWndLimit = MAXWNDTEXCACHE / 2;

u8 * texturepart = NULL;
u8 * texturebuffer = NULL;
uint32_t g_x1, g_y1, g_x2, g_y2;
unsigned char ubOpaqueDraw = 0;

unsigned short MAXTPAGES = 32;
unsigned short CLUTMASK = 0x7fff;
unsigned short CLUTYMASK = 0x1ff;
unsigned short MAXSORTTEX = 196;

////////////////////////////////////////////////////////////////////////
// Texture color conversions... all my ASM funcs are removed for easier
// porting... and honestly: nowadays the speed gain would be pointless
////////////////////////////////////////////////////////////////////////

uint32_t XP8RGBA(uint32_t BGR) {
#if 1
    if (!(BGR & 0xffff)) return 0x50000000;
    if (DrawSemiTrans && !(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff);
    }
    return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
#else
    return BGR;
#endif
}

uint32_t XP8RGBAEx(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0x03000000;
    if (DrawSemiTrans && !(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff);
    }
    return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
}

uint32_t CP8RGBA(uint32_t BGR) {
    uint32_t l;
    if (!(BGR & 0xffff)) return 0x50000000;
    if (DrawSemiTrans && !(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff);
    }
    l = ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
    if (l == 0xffffff00) l = 0xff000000;
    return l;
}

uint32_t CP8RGBAEx(uint32_t BGR) {
    uint32_t l;
    if (!(BGR & 0xffff)) return 0x03000000;
    if (DrawSemiTrans && !(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff);
    }
    l = ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
    if (l == 0xffffff00) l = 0xff000000;
    return l;
}

uint32_t XP8RGBA_0(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0x50000000;
    return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
}

uint32_t XP8RGBAEx_0(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0x03000000;
    return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
}

uint32_t XP8BGRA_0(uint32_t BGR) {
#if 1
    if (!(BGR & 0xffff)) return 0x50000000;
    return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff) | 0xff000000;
#else
    return BGR;
#endif
}

uint32_t XP8BGRAEx_0(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0x03000000;
    return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff) | 0xff000000;
}

uint32_t CP8RGBA_0(uint32_t BGR) {
    uint32_t l;

    if (!(BGR & 0xffff)) return 0x50000000;
    l = ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
    if (l == 0xfff8f800) l = 0xff000000;
    return l;
}

uint32_t CP8RGBAEx_0(uint32_t BGR) {
    uint32_t l;

    if (!(BGR & 0xffff)) return 0x03000000;
    l = ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
    if (l == 0xfff8f800) l = 0xff000000;
    return l;
}

uint32_t CP8BGRA_0(uint32_t BGR) {
    uint32_t l;

    if (!(BGR & 0xffff)) return 0x50000000;
    l = ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff) | 0xff000000;
    if (l == 0xff00f8f8) l = 0xff000000;
    return l;
}

uint32_t CP8BGRAEx_0(uint32_t BGR) {
    uint32_t l;

    if (!(BGR & 0xffff)) return 0x03000000;
    l = ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff) | 0xff000000;
    if (l == 0xff00f8f8) l = 0xff000000;
    return l;
}

uint32_t XP8RGBA_1(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0x50000000;
    if (!(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff);
    }
    return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
}

uint32_t XP8RGBAEx_1(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0x03000000;
    if (!(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff);
    }
    return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
}

uint32_t XP8BGRA_1(uint32_t BGR) {
#if 1
    if (!(BGR & 0xffff)) return 0x50000000;
    if (!(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff);
    }
    return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff) | 0xff000000;
#else
    return BGR;
#endif
}

uint32_t XP8BGRAEx_1(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0x03000000;
    if (!(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff);
    }
    return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff) | 0xff000000;
}

uint32_t P8RGBA(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0;
    return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
}

uint32_t P8BGRA(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0;
    return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff) | 0xff000000;
}

unsigned short XP5RGBA(unsigned short BGR) {
    if (!BGR) return 0;
    if (DrawSemiTrans && !(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 11)) | ((BGR >> 9)&0x3e) | ((BGR << 1)&0x7c0)));
    }
    return ((((BGR << 11)) | ((BGR >> 9)&0x3e) | ((BGR << 1)&0x7c0))) | 1;
}

unsigned short XP5RGBA_0(unsigned short BGR) {
    if (!BGR) return 0;

    return ((((BGR << 11)) | ((BGR >> 9)&0x3e) | ((BGR << 1)&0x7c0))) | 1;
}

unsigned short CP5RGBA_0(unsigned short BGR) {
    unsigned short s;

    if (!BGR) return 0;

    s = ((((BGR << 11)) | ((BGR >> 9)&0x3e) | ((BGR << 1)&0x7c0))) | 1;
    if (s == 0x07ff) s = 1;
    return s;
}

unsigned short XP5RGBA_1(unsigned short BGR) {
    if (!BGR) return 0;
    if (!(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 11)) | ((BGR >> 9)&0x3e) | ((BGR << 1)&0x7c0)));
    }
    return ((((BGR << 11)) | ((BGR >> 9)&0x3e) | ((BGR << 1)&0x7c0))) | 1;
}

unsigned short P5RGBA(unsigned short BGR) {
    if (!BGR) return 0;
    return ((((BGR << 11)) | ((BGR >> 9)&0x3e) | ((BGR << 1)&0x7c0))) | 1;
}

unsigned short XP4RGBA(unsigned short BGR) {
    if (!BGR) return 6;
    if (DrawSemiTrans && !(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 11)) | ((BGR >> 9)&0x3e) | ((BGR << 1)&0x7c0)));
    }
    return (((((BGR & 0x1e) << 11)) | ((BGR & 0x7800) >> 7) | ((BGR & 0x3c0) << 2))) | 0xf;
}

unsigned short XP4RGBA_0(unsigned short BGR) {
    if (!BGR) return 6;
    return (((((BGR & 0x1e) << 11)) | ((BGR & 0x7800) >> 7) | ((BGR & 0x3c0) << 2))) | 0xf;
}

unsigned short CP4RGBA_0(unsigned short BGR) {
    unsigned short s;
    if (!BGR) return 6;
    s = (((((BGR & 0x1e) << 11)) | ((BGR & 0x7800) >> 7) | ((BGR & 0x3c0) << 2))) | 0xf;
    if (s == 0x0fff) s = 0x000f;
    return s;
}

unsigned short XP4RGBA_1(unsigned short BGR) {
    if (!BGR) return 6;
    if (!(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 11)) | ((BGR >> 9)&0x3e) | ((BGR << 1)&0x7c0)));
    }
    return (((((BGR & 0x1e) << 11)) | ((BGR & 0x7800) >> 7) | ((BGR & 0x3c0) << 2))) | 0xf;
}

unsigned short P4RGBA(unsigned short BGR) {
    if (!BGR) return 0;
    return (((((BGR & 0x1e) << 11)) | ((BGR & 0x7800) >> 7) | ((BGR & 0x3c0) << 2))) | 0xf;
}

////////////////////////////////////////////////////////////////////////
// CHECK TEXTURE MEM (on plugin startup)
////////////////////////////////////////////////////////////////////////

int iFTexA = 512;
int iFTexB = 512;

void CheckTextureMemory(void) {
    iSortTexCnt = 8;
}

////////////////////////////////////////////////////////////////////////
// Main init of textures
////////////////////////////////////////////////////////////////////////

void InitializeTextureStore() {
    int i, j;

    if (iGPUHeight == 1024) {
        MAXTPAGES = 64;
        CLUTMASK = 0xffff;
        CLUTYMASK = 0x3ff;
        MAXSORTTEX = 128;
        iTexGarbageCollection = 0;
        TR;
    } else {
        MAXTPAGES = 32;
        CLUTMASK = 0x7fff;
        CLUTYMASK = 0x1ff;
        MAXSORTTEX = 196;
        TR;
    }

    memset(vertex, 0, 4 * sizeof (OGLVertex)); // init vertices

    gTexName = NULL; // init main tex name

    iTexWndLimit = MAXWNDTEXCACHE;
    if (!iUsePalTextures) iTexWndLimit /= 2;

    memset(wcWndtexStore, 0, sizeof (textureWndCacheEntry) *MAXWNDTEXCACHE);
    texturepart = (GLubyte *) malloc(256 * 256 * 4);
    memset(texturepart, 0, 256 * 256 * 4);
    if (iHiResTextures)
        texturebuffer = (GLubyte *) malloc(512 * 512 * 4);
    else texturebuffer = NULL;

    for (i = 0; i < 3; i++) // -> info for 32*3
        for (j = 0; j < MAXTPAGES; j++) {
            pscSubtexStore[i][j] = (textureSubCacheEntryS *) malloc(CSUBSIZES * sizeof (textureSubCacheEntryS));
            memset(pscSubtexStore[i][j], 0, CSUBSIZES * sizeof (textureSubCacheEntryS));
        }
    for (i = 0; i < MAXSORTTEX; i++) // -> info 0..511
    {
        pxSsubtexLeft[i] = (EXLong *) malloc(CSUBSIZE * sizeof (EXLong));
        memset(pxSsubtexLeft[i], 0, CSUBSIZE * sizeof (EXLong));
        uiStexturePage[i] = NULL;
    }
}

////////////////////////////////////////////////////////////////////////
// Clean up on exit
////////////////////////////////////////////////////////////////////////

void CleanupTextureStore() {
    TR;
    int i, j;
    textureWndCacheEntry * tsx;
    //----------------------------------------------------//
    //XeBindTexture(0);
    //----------------------------------------------------//
    free(texturepart); // free tex part
    texturepart = 0;
    if (texturebuffer) {
        free(texturebuffer);
        texturebuffer = 0;
    }
    //----------------------------------------------------//
    tsx = wcWndtexStore; // loop tex window cache
    for (i = 0; i < MAXWNDTEXCACHE; i++, tsx++) {
        if (tsx->texname) // -> some tex?
        {
            //XeDeleteTextures(1, &tsx->texname); // --> delete it
            Xe_DestroyTexture(xe, tsx->texname);
        }
    }
    iMaxTexWnds = 0; // no more tex wnds
    //----------------------------------------------------//
    if (gTexMovieName != 0) // some movie tex?
    {
        Xe_DestroyTexture(xe, gTexMovieName);
    }
    gTexMovieName = 0; // no more movie tex
    //----------------------------------------------------//
    if (gTexFrameName != 0) // some 15bit framebuffer tex?
    {
        Xe_DestroyTexture(xe, gTexFrameName);
    }
    gTexFrameName = 0; // no more movie tex
    //----------------------------------------------------//
    if (gTexBlurName != 0) // some 15bit framebuffer tex?
    {
        Xe_DestroyTexture(xe, gTexBlurName); // -> delete it
    }
    gTexBlurName = 0; // no more movie tex
    //----------------------------------------------------//
    for (i = 0; i < 3; i++) // -> loop
        for (j = 0; j < MAXTPAGES; j++) // loop tex pages
        {
            free(pscSubtexStore[i][j]); // -> clean mem
        }
    for (i = 0; i < MAXSORTTEX; i++) {
        if (uiStexturePage[i]) // --> tex used ?
        {
            Xe_DestroyTexture(xe, uiStexturePage[i]);
            uiStexturePage[i] = 0; // --> delete it
        }
        free(pxSsubtexLeft[i]); // -> clean mem
    }
    //----------------------------------------------------//
}

////////////////////////////////////////////////////////////////////////
// Reset textures in game...
////////////////////////////////////////////////////////////////////////

void ResetTextureArea(BOOL bDelTex) {
    int i, j;
    textureSubCacheEntryS * tss;
    EXLong * lu;
    textureWndCacheEntry * tsx;
    //----------------------------------------------------//

    dwTexPageComp = 0;

    //----------------------------------------------------//
    if (bDelTex) {
       // XeBindTexture(0);
        gTexName = 0;
    }
    //----------------------------------------------------//
    tsx = wcWndtexStore;
    for (i = 0; i < MAXWNDTEXCACHE; i++, tsx++) {
        tsx->used = 0;
        if (bDelTex && tsx->texname) {
            Xe_DestroyTexture(xe, tsx->texname);
            tsx->texname = 0;
        }
    }
    iMaxTexWnds = 0;
    //----------------------------------------------------//

    for (i = 0; i < 3; i++)
        for (j = 0; j < MAXTPAGES; j++) {
            tss = pscSubtexStore[i][j];
            (tss + SOFFA)->pos.l = 0;
            (tss + SOFFB)->pos.l = 0;
            (tss + SOFFC)->pos.l = 0;
            (tss + SOFFD)->pos.l = 0;
        }

    for (i = 0; i < iSortTexCnt; i++) {
        lu = pxSsubtexLeft[i];
        lu->l = 0;
        if (bDelTex && uiStexturePage[i]) {
            Xe_DestroyTexture(xe, uiStexturePage[i]);
            uiStexturePage[i] = 0;
        }
    }
}


////////////////////////////////////////////////////////////////////////
// Invalidate tex windows
////////////////////////////////////////////////////////////////////////

void InvalidateWndTextureArea(int X, int Y, int W, int H) {
    int i, px1, px2, py1, py2, iYM = 1;
    textureWndCacheEntry * tsw = wcWndtexStore;

    W += X - 1;
    H += Y - 1;
    if (X < 0) X = 0;
    if (X > 1023) X = 1023;
    if (W < 0) W = 0;
    if (W > 1023) W = 1023;
    if (Y < 0) Y = 0;
    if (Y > iGPUHeightMask) Y = iGPUHeightMask;
    if (H < 0) H = 0;
    if (H > iGPUHeightMask) H = iGPUHeightMask;
    W++;
    H++;

    if (iGPUHeight == 1024) iYM = 3;

    py1 = min(iYM, Y >> 8);
    py2 = min(iYM, H >> 8); // y: 0 or 1

    px1 = max(0, (X >> 6));
    px2 = min(15, (W >> 6));

    if (py1 == py2) {
        py1 = py1 << 4;
        px1 += py1;
        px2 += py1; // change to 0-31
        for (i = 0; i < iMaxTexWnds; i++, tsw++) {
            if (tsw->used) {
                if (tsw->pageid >= px1 && tsw->pageid <= px2) {
                    tsw->used = 0;
                }
            }
        }
    } else {
        py1 = px1 + 16;
        py2 = px2 + 16;
        for (i = 0; i < iMaxTexWnds; i++, tsw++) {
            if (tsw->used) {
                if ((tsw->pageid >= px1 && tsw->pageid <= px2) ||
                        (tsw->pageid >= py1 && tsw->pageid <= py2)) {
                    tsw->used = 0;
                }
            }
        }
    }

    // adjust tex window count
    tsw = wcWndtexStore + iMaxTexWnds - 1;
    while (iMaxTexWnds && !tsw->used) {
        iMaxTexWnds--;
        tsw--;
    }
}



////////////////////////////////////////////////////////////////////////
// same for sort textures
////////////////////////////////////////////////////////////////////////

void MarkFree(textureSubCacheEntryS * tsx) {
    EXLong * ul, * uls;
    int j, iMax;
    unsigned char x1, y1, dx, dy;

    uls = pxSsubtexLeft[tsx->cTexID];
    iMax = uls->l;
    ul = uls + 1;

    if (!iMax) return;

    for (j = 0; j < iMax; j++, ul++)
        if (ul->l == 0xffffffff) break;

    if (j < CSUBSIZE - 2) {
        if (j == iMax) uls->l = uls->l + 1;

        x1 = tsx->posTX;
        dx = tsx->pos.c[2] - tsx->pos.c[3];
        if (tsx->posTX) {
            x1--;
            dx += 3;
        }
        y1 = tsx->posTY;
        dy = tsx->pos.c[0] - tsx->pos.c[1];
        if (tsx->posTY) {
            y1--;
            dy += 3;
        }

        ul->c[3] = x1;
        ul->c[2] = dx;
        ul->c[1] = y1;
        ul->c[0] = dy;
    }
}

void InvalidateSubSTextureArea(int X, int Y, int W, int H) {
    int i, j, k, iMax, px, py, px1, px2, py1, py2, iYM = 1;
    EXLong npos;
    textureSubCacheEntryS *tsb;
    int x1, x2, y1, y2, xa, sw;

    W += X - 1;
    H += Y - 1;
    if (X < 0) X = 0;
    if (X > 1023) X = 1023;
    if (W < 0) W = 0;
    if (W > 1023) W = 1023;
    if (Y < 0) Y = 0;
    if (Y > iGPUHeightMask) Y = iGPUHeightMask;
    if (H < 0) H = 0;
    if (H > iGPUHeightMask) H = iGPUHeightMask;
    W++;
    H++;

    if (iGPUHeight == 1024) iYM = 3;

    py1 = min(iYM, Y >> 8);
    py2 = min(iYM, H >> 8); // y: 0 or 1
    px1 = max(0, (X >> 6) - 3);
    px2 = min(15, (W >> 6) + 3); // x: 0-15

    for (py = py1; py <= py2; py++) {
        j = (py << 4) + px1; // get page

        y1 = py * 256;
        y2 = y1 + 255;

        if (H < y1) continue;
        if (Y > y2) continue;

        if (Y > y1) y1 = Y;
        if (H < y2) y2 = H;
        if (y2 < y1) {
            sw = y1;
            y1 = y2;
            y2 = sw;
        }
        y1 = ((y1 % 256) << 8);
        y2 = (y2 % 256);

        for (px = px1; px <= px2; px++, j++) {
            for (k = 0; k < 3; k++) {
                xa = x1 = px << 6;
                if (W < x1) continue;
                x2 = x1 + (64 << k) - 1;
                if (X > x2) continue;

                if (X > x1) x1 = X;
                if (W < x2) x2 = W;
                if (x2 < x1) {
                    sw = x1;
                    x1 = x2;
                    x2 = sw;
                }

                if (dwGPUVersion == 2)
                    npos.l = 0x00ff00ff;
                else
                    npos.l = ((x1 - xa) << (26 - k)) | ((x2 - xa) << (18 - k)) | y1 | y2;

                {
                    tsb = pscSubtexStore[k][j] + SOFFA;
                    iMax = tsb->pos.l;
                    tsb++;
                    for (i = 0; i < iMax; i++, tsb++)
                        if (tsb->ClutID && XCHECK(tsb->pos, npos)) {
                            tsb->ClutID = 0;
                            MarkFree(tsb);
                        }

                    //         if(npos.l & 0x00800000)
                    {
                        tsb = pscSubtexStore[k][j] + SOFFB;
                        iMax = tsb->pos.l;
                        tsb++;
                        for (i = 0; i < iMax; i++, tsb++)
                            if (tsb->ClutID && XCHECK(tsb->pos, npos)) {
                                tsb->ClutID = 0;
                                MarkFree(tsb);
                            }
                    }

                    //         if(npos.l & 0x00000080)
                    {
                        tsb = pscSubtexStore[k][j] + SOFFC;
                        iMax = tsb->pos.l;
                        tsb++;
                        for (i = 0; i < iMax; i++, tsb++)
                            if (tsb->ClutID && XCHECK(tsb->pos, npos)) {
                                tsb->ClutID = 0;
                                MarkFree(tsb);
                            }
                    }

                    //         if(npos.l & 0x00800080)
                    {
                        tsb = pscSubtexStore[k][j] + SOFFD;
                        iMax = tsb->pos.l;
                        tsb++;
                        for (i = 0; i < iMax; i++, tsb++)
                            if (tsb->ClutID && XCHECK(tsb->pos, npos)) {
                                tsb->ClutID = 0;
                                MarkFree(tsb);
                            }
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////
// Invalidate some parts of cache: main routine
////////////////////////////////////////////////////////////////////////

void InvalidateTextureAreaEx(void) {
    short W = sxmax - sxmin;
    short H = symax - symin;

    if (W == 0 && H == 0) return;

    if (iMaxTexWnds)
        InvalidateWndTextureArea(sxmin, symin, W, H);

    InvalidateSubSTextureArea(sxmin, symin, W, H);
}

////////////////////////////////////////////////////////////////////////

void InvalidateTextureArea(int X, int Y, int W, int H) {
    if (W == 0 && H == 0) return;

    if (iMaxTexWnds) InvalidateWndTextureArea(X, Y, W, H);

    InvalidateSubSTextureArea(X, Y, W, H);
}


////////////////////////////////////////////////////////////////////////
// tex window: define
////////////////////////////////////////////////////////////////////////

void DefineTextureWnd(void) {
    /*
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
     */
    
    if (iFilterType && iFilterType < 3 && iHiResTextures != 2) {
        gTexName->use_filtering = XE_TEXF_LINEAR;
    }
    else{
        gTexName->use_filtering = XE_TEXF_POINT;
    }
    
    gTexName = Xe_CreateTexture(xe, TWin.Position.x1, TWin.Position.y1, 1,  XE_FMT_8888 | XE_FMT_ARGB, 0);
    xeGfx_setTextureData(gTexName,texturepart);
/*
    glTexImage2D(GL_TEXTURE_2D, 0, giWantedRGBA,
            TWin.Position.x1,
            TWin.Position.y1,
            0, giWantedFMT, giWantedTYPE, texturepart);
*/
}

////////////////////////////////////////////////////////////////////////
// tex window: load stretched ZN ???
////////////////////////////////////////////////////////////////////////

void LoadStretchWndTexturePage(int pageid, int mode, short cx, short cy) {
    TR;
    uint32_t start, row, column, j, sxh, sxm, ldx, ldy, ldxo, s;
    unsigned int palstart;
    uint32_t *px, *pa, *ta;
    unsigned char *cSRCPtr, *cOSRCPtr;
    unsigned short *wSRCPtr, *wOSRCPtr;
    uint32_t LineOffset;
    int pmult = pageid / 16;
    uint32_t(*LTCOL)(uint32_t);

    LTCOL = TCF[DrawSemiTrans];

    ldxo = TWin.Position.x1 - TWin.OPosition.x1;
    ldy = TWin.Position.y1 - TWin.OPosition.y1;

    pa = px = (uint32_t *) ubPaletteBuffer;
    ta = (uint32_t *) texturepart;
    palstart = cx + (cy * 1024);

    ubOpaqueDraw = 0;

    switch (mode) {
            //--------------------------------------------------//
            // 4bit texture load ..
        case 0:
            //------------------- ZN STUFF

            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 4;
                do {
/*
                    *px = LTCOL(*wSRCPtr);
                    *(px + 1) = LTCOL(*(wSRCPtr + 1));
                    *(px + 2) = LTCOL(*(wSRCPtr + 2));
                    *(px + 3) = LTCOL(*(wSRCPtr + 3));
*/
                    PUTLE32(px,LTCOL(*wSRCPtr));
                    PUTLE32(px+1,LTCOL(*wSRCPtr+1));
                    PUTLE32(px+2,LTCOL(*wSRCPtr+2));
                    PUTLE32(px+3,LTCOL(*wSRCPtr+3));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                column = g_y2 - ldy;
                for (TXV = g_y1; TXV <= column; TXV++) {
                    ldx = ldxo;
                    for (TXU = g_x1; TXU <= g_x2 - ldxo; TXU++) {
                        n_xi = ((TXU >> 2) & ~0x3c) + ((TXV << 2) & 0x3c);
                        n_yi = (TXV & ~0xf) + ((TXU >> 4) & 0xf);

                        s = *(pa + ((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x03) << 2)) & 0x0f));
                        *ta++ = s;

                        if (ldx) {
                            *ta++ = s;
                            ldx--;
                        }
                    }

                    if (ldy) {
                        ldy--;
                        for (TXU = g_x1; TXU <= g_x2; TXU++)
                        {
                            *ta++ = *(ta - (g_x2 - g_x1));
                        }
                    }
                }

                DefineTextureWnd();

                break;
            }

            //-------------------

            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;
            // convert CLUT to 32bits .. and then use THAT as a lookup table

            wSRCPtr = psxVuw + palstart;
            for (row = 0; row < 16; row++)
            {
                *px++ = LTCOL(*wSRCPtr++);
            }

            sxm = g_x1 & 1;
            sxh = g_x1 >> 1;
            if (sxm) j = g_x1 + 1;
            else j = g_x1;
            cSRCPtr = psxVub + start + (2048 * g_y1) + sxh;
            for (column = g_y1; column <= g_y2; column++) {
                cOSRCPtr = cSRCPtr;
                ldx = ldxo;
                if (sxm) *ta++ = *(pa + ((*cSRCPtr++ >> 4) & 0xF));

                for (row = j; row <= g_x2 - ldxo; row++) {
                    s = *(pa + (*cSRCPtr & 0xF));
                    *ta++ = s;
                    if (ldx) {
                        *ta++ = s;
                        ldx--;
                    }
                    row++;
                    if (row <= g_x2 - ldxo) {
                        s = *(pa + ((*cSRCPtr >> 4) & 0xF));
                        *ta++ = s;
                        if (ldx) {
                            *ta++ = s;
                            ldx--;
                        }
                    }
                    cSRCPtr++;
                }
                if (ldy && column & 1) {
                    ldy--;
                    cSRCPtr = cOSRCPtr;
                } else cSRCPtr = psxVub + start + (2048 * (column + 1)) + sxh;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // 8bit texture load ..
        case 1:
            //------------ ZN STUFF
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 64;
                do {
                    *px = LTCOL(*wSRCPtr);
                    *(px + 1) = LTCOL(*(wSRCPtr + 1));
                    *(px + 2) = LTCOL(*(wSRCPtr + 2));
                    *(px + 3) = LTCOL(*(wSRCPtr + 3));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                column = g_y2 - ldy;
                for (TXV = g_y1; TXV <= column; TXV++) {
                    ldx = ldxo;
                    for (TXU = g_x1; TXU <= g_x2 - ldxo; TXU++) {
                        n_xi = ((TXU >> 1) & ~0x78) + ((TXU << 2) & 0x40) + ((TXV << 3) & 0x38);
                        n_yi = (TXV & ~0x7) + ((TXU >> 5) & 0x7);

                        s = *(pa + ((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x01) << 3)) & 0xff));
                        *ta++ = s;
                        if (ldx) {
                            *ta++ = s;
                            ldx--;
                        }
                    }

                    if (ldy) {
                        ldy--;
                        for (TXU = g_x1; TXU <= g_x2; TXU++)
                            *ta++ = *(ta - (g_x2 - g_x1));
                    }

                }

                DefineTextureWnd();

                break;
            }
            //------------

            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;

            // not using a lookup table here... speeds up smaller texture areas
            cSRCPtr = psxVub + start + (2048 * g_y1) + g_x1;
            LineOffset = 2048 - (g_x2 - g_x1 + 1) + ldxo;

            for (column = g_y1; column <= g_y2; column++) {
                cOSRCPtr = cSRCPtr;
                ldx = ldxo;
                for (row = g_x1; row <= g_x2 - ldxo; row++) {
                    s = LTCOL(psxVuw[palstart + *cSRCPtr++]);
                    *ta++ = s;
                    if (ldx) {
                        *ta++ = s;
                        ldx--;
                    }
                }
                if (ldy && column & 1) {
                    ldy--;
                    cSRCPtr = cOSRCPtr;
                } else cSRCPtr += LineOffset;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // 16bit texture load ..
        case 2:
            start = ((pageid - 16 * pmult)*64) + 256 * 1024 * pmult;

            wSRCPtr = psxVuw + start + (1024 * g_y1) + g_x1;
            LineOffset = 1024 - (g_x2 - g_x1 + 1) + ldxo;

            for (column = g_y1; column <= g_y2; column++) {
                wOSRCPtr = wSRCPtr;
                ldx = ldxo;
                for (row = g_x1; row <= g_x2 - ldxo; row++) {
                    s = LTCOL(*wSRCPtr++);
                    *ta++ = s;
                    if (ldx) {
                        *ta++ = s;
                        ldx--;
                    }
                }
                if (ldy && column & 1) {
                    ldy--;
                    wSRCPtr = wOSRCPtr;
                } else wSRCPtr += LineOffset;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // others are not possible !
    }
}

////////////////////////////////////////////////////////////////////////
// tex window: load packed simple
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// tex window: load simple
////////////////////////////////////////////////////////////////////////

void LoadWndTexturePage(int pageid, int mode, short cx, short cy) {

    TR;
    uint32_t start, row, column, j, sxh, sxm;
    unsigned int palstart;
    uint32_t *px, *pa, *ta;
    unsigned char *cSRCPtr;
    unsigned short *wSRCPtr;
    uint32_t LineOffset;
    int pmult = pageid / 16;
    uint32_t(*LTCOL)(uint32_t);

    LTCOL = TCF[DrawSemiTrans];

    pa = px = (uint32_t *) ubPaletteBuffer;
    ta = (uint32_t *) texturepart;
    palstart = cx + (cy * 1024);

    ubOpaqueDraw = 0;

    switch (mode) {
            //--------------------------------------------------//
            // 4bit texture load ..
        case 0:
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 4;
                do {
/*
                    *px = LTCOL(*wSRCPtr);
                    *(px + 1) = LTCOL(*(wSRCPtr + 1));
                    *(px + 2) = LTCOL(*(wSRCPtr + 2));
                    *(px + 3) = LTCOL(*(wSRCPtr + 3));
*/
                    PUTLE32(px,LTCOL(*wSRCPtr));
                    PUTLE32(px+1,LTCOL(*wSRCPtr+1));
                    PUTLE32(px+2,LTCOL(*wSRCPtr+2));
                    PUTLE32(px+3,LTCOL(*wSRCPtr+3));

                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                for (TXV = g_y1; TXV <= g_y2; TXV++) {
                    for (TXU = g_x1; TXU <= g_x2; TXU++) {
                        n_xi = ((TXU >> 2) & ~0x3c) + ((TXV << 2) & 0x3c);
                        n_yi = (TXV & ~0xf) + ((TXU >> 4) & 0xf);

                        //*ta++ = *(pa + ((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x03) << 2)) & 0x0f));
                        PUTLE32(ta, *(pa + ((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x03) << 2)) & 0x0f)));
                        ta++;
                    }
                }

                DefineTextureWnd();

                break;
            }

            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;

            // convert CLUT to 32bits .. and then use THAT as a lookup table

            wSRCPtr = psxVuw + palstart;
            for (row = 0; row < 16; row++)
            {
                //*px++ = LTCOL(*wSRCPtr++);
                PUTLE32(px,LTCOL(*wSRCPtr++));
                px++;
            }

            sxm = g_x1 & 1;
            sxh = g_x1 >> 1;
            if (sxm) j = g_x1 + 1;
            else j = g_x1;
            cSRCPtr = psxVub + start + (2048 * g_y1) + sxh;
            for (column = g_y1; column <= g_y2; column++) {
                cSRCPtr = psxVub + start + (2048 * column) + sxh;

                if (sxm) *ta++ = *(pa + ((*cSRCPtr++ >> 4) & 0xF));

                for (row = j; row <= g_x2; row++) {
                    //*ta++ = *(pa + (*cSRCPtr & 0xF));
                    PUTLE32(ta,*(pa + (*cSRCPtr & 0xF)));
                    ta++;
                    row++;
                    if (row <= g_x2) {
                        //*ta++ = *(pa + ((*cSRCPtr >> 4) & 0xF));
                        PUTLE32(ta,*(pa + ((*cSRCPtr >> 4) & 0xF)));
                        ta++;
                    }
                    cSRCPtr++;
                }
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // 8bit texture load ..
        case 1:
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 64;
                do {
/*
                    *px = LTCOL(*wSRCPtr);
                    *(px + 1) = LTCOL(*(wSRCPtr + 1));
                    *(px + 2) = LTCOL(*(wSRCPtr + 2));
                    *(px + 3) = LTCOL(*(wSRCPtr + 3));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
*/

                    PUTLE32(px,LTCOL(*wSRCPtr));
                    PUTLE32(px+1,LTCOL(*wSRCPtr+1));
                    PUTLE32(px+2,LTCOL(*wSRCPtr+2));
                    PUTLE32(px+3,LTCOL(*wSRCPtr+3));
                    row--;
                    px += 4;
                    wSRCPtr += 4;

                } while (row);

                for (TXV = g_y1; TXV <= g_y2; TXV++) {
                    for (TXU = g_x1; TXU <= g_x2; TXU++) {
                        n_xi = ((TXU >> 1) & ~0x78) + ((TXU << 2) & 0x40) + ((TXV << 3) & 0x38);
                        n_yi = (TXV & ~0x7) + ((TXU >> 5) & 0x7);

//                        *ta++ = *(pa + ((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x01) << 3)) & 0xff));
                        PUTLE32(ta,*(pa + ((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x01) << 3)) & 0xff)));
                        ta++;
                    }
                }

                DefineTextureWnd();

                break;
            }

            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;

            // not using a lookup table here... speeds up smaller texture areas
            cSRCPtr = psxVub + start + (2048 * g_y1) + g_x1;
            LineOffset = 2048 - (g_x2 - g_x1 + 1);

            for (column = g_y1; column <= g_y2; column++) {
                for (row = g_x1; row <= g_x2; row++)
                {
                    //*ta++ = LTCOL(psxVuw[palstart + *cSRCPtr++]);
                    PUTLE32(ta,LTCOL(psxVuw[palstart + *cSRCPtr]));
                    ta++;cSRCPtr++;
                }
                cSRCPtr += LineOffset;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // 16bit texture load ..
        case 2:
            start = ((pageid - 16 * pmult)*64) + 256 * 1024 * pmult;

            wSRCPtr = psxVuw + start + (1024 * g_y1) + g_x1;
            LineOffset = 1024 - (g_x2 - g_x1 + 1);

            for (column = g_y1; column <= g_y2; column++) {
                for (row = g_x1; row <= g_x2; row++)
                {
                    //*ta++ = LTCOL(*wSRCPtr++);
                    PUTLE32(ta,LTCOL(*wSRCPtr));
                    ta++;wSRCPtr++;
                }
                wSRCPtr += LineOffset;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // others are not possible !
    }
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void UploadTexWndPal(int mode, short cx, short cy) {
    unsigned int i, iSize;
    unsigned short *wSrcPtr;
    uint32_t *ta = (uint32_t *) texturepart;

    wSrcPtr = psxVuw + cx + (cy * 1024);
    if (mode == 0) i = 4;
    else i = 64;
    iSize = i << 2;
    ubOpaqueDraw = 0;

    do {
/*
        *ta = PALCOL(*wSrcPtr);
        *(ta + 1) = PALCOL(*(wSrcPtr + 1));
        *(ta + 2) = PALCOL(*(wSrcPtr + 2));
        *(ta + 3) = PALCOL(*(wSrcPtr + 3));
*/
        PUTLE32(&ta[0],PALCOL(*wSrcPtr));
        PUTLE32(&ta[1],PALCOL(*wSrcPtr+1));
        PUTLE32(&ta[2],PALCOL(*wSrcPtr+2));
        PUTLE32(&ta[3],PALCOL(*wSrcPtr+3));

        ta += 4;
        wSrcPtr += 4;
        i--;
    } while (i);

    /*
     (*glColorTableEXTEx)(GL_TEXTURE_2D,GL_RGBA8,iSize,
                        GL_RGBA,GL_UNSIGNED_BYTE,texturepart);
     */

   // XeTexImage(0,256, 256, XE_FMT_ARGB, XE_FMT_8888, texturepart);
    TR;
}

////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// tex window: main selecting, cache handler included
////////////////////////////////////////////////////////////////////////

struct XenosSurface * LoadTextureWnd(int pageid, int TextureMode, uint32_t GivenClutId) {
    textureWndCacheEntry *ts, *tsx = NULL;
    int i;
    short cx, cy;
    EXLong npos;

    npos.c[3] = TWin.Position.x0;
    npos.c[2] = TWin.OPosition.x1;
    npos.c[1] = TWin.Position.y0;
    npos.c[0] = TWin.OPosition.y1;

    g_x1 = TWin.Position.x0;
    g_x2 = g_x1 + TWin.Position.x1 - 1;
    g_y1 = TWin.Position.y0;
    g_y2 = g_y1 + TWin.Position.y1 - 1;

    if (TextureMode == 2) {
        GivenClutId = 0;
        cx = cy = 0;
    } else {
        cx = ((GivenClutId << 4) & 0x3F0);
        cy = ((GivenClutId >> 6) & CLUTYMASK);
        GivenClutId = (GivenClutId & CLUTMASK) | (DrawSemiTrans << 30);

        // palette check sum
        {
            uint32_t l = 0, row;
             uint32_t *lSRCPtr = (uint32_t *) (psxVuw + cx + (cy * 1024));
#if 1
            if (TextureMode == 1)
                for (row = 1; row < 129; row++)
                {
                    l += ((GETLE32(lSRCPtr)) - 1) * row;

                    *lSRCPtr++;
                }
            else
                for (row = 1; row < 9; row++)
                {
                    l += ((GETLE32(lSRCPtr)) - 1) << row;

                    *lSRCPtr++;
                }
#else
             if (TextureMode == 1) for (row = 1; row < 129; row++) l += ((*lSRCPtr++) - 1) * row;
            else for (row = 1; row < 9; row++) l += ((*lSRCPtr++) - 1) << row;
#endif
            l = (l + HIWORD(l))&0x3fffL;
            GivenClutId |= (l << 16);
           // printf("LoadTextureWnd : GivenClutId=%d\r\n",GivenClutId);
        }

    }

    ts = wcWndtexStore;

    for (i = 0; i < iMaxTexWnds; i++, ts++) {
        if (ts->used) {
            if (ts->pos.l == npos.l &&
                    ts->pageid == pageid &&
                    ts->textureMode == TextureMode) {
                if (ts->ClutID == GivenClutId) {
                    ubOpaqueDraw = ts->Opaque;
                    return ts->texname;
                } else if (TextureMode != 2) {
                    ts->ClutID = GivenClutId;
                    if (ts->texname != gTexName) {
                        gTexName = ts->texname;
                        //XeBindTexture(gTexName);
                    }
                    UploadTexWndPal(TextureMode, cx, cy);
                    ts->Opaque = ubOpaqueDraw;
                    return gTexName;
                }
            }
        } else tsx = ts;
    }

    if (!tsx) {
        if (iMaxTexWnds == iTexWndLimit) {
            tsx = wcWndtexStore + iTexWndTurn;
            iTexWndTurn++;
            if (iTexWndTurn == iTexWndLimit) iTexWndTurn = 0;
        } else {
            tsx = wcWndtexStore + iMaxTexWnds;
            iMaxTexWnds++;
        }
    }

    gTexName = tsx->texname;

    if (TWin.OPosition.y1 == TWin.Position.y1 &&
            TWin.OPosition.x1 == TWin.Position.x1) {
        LoadWndTexturePage(pageid, TextureMode, cx, cy);
    } else {
        LoadStretchWndTexturePage(pageid, TextureMode, cx, cy);
    }

    tsx->Opaque = ubOpaqueDraw;
    tsx->pos.l = npos.l;
    tsx->ClutID = GivenClutId;
    tsx->pageid = pageid;
    tsx->textureMode = TextureMode;
    tsx->texname = gTexName;
    tsx->used = 1;

    return gTexName;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// movie texture: define
////////////////////////////////////////////////////////////////////////

void DefinePackedTextureMovie(void) {
    if (gTexMovieName == 0) {       
        gTexMovieName = Xe_CreateTexture(xe, 256, 256, 1,  XE_FMT_8888 | XE_FMT_ARGB, 0);
        if (!bUseFastMdec) {
            gTexMovieName->use_filtering = XE_TEXF_LINEAR;
        }
        else{
            gTexMovieName->use_filtering = XE_TEXF_POINT;
        }
        gTexMovieName->u_addressing = iClampType;
        gTexMovieName->v_addressing = iClampType;
        xeGfx_setTextureData(gTexMovieName, texturepart);
    }
    gTexName = gTexMovieName;

    XeTexSubImage(0,0,(xrMovieArea.x1 - xrMovieArea.x0),(xrMovieArea.y1 - xrMovieArea.y0),texturepart);
    TR;
}

////////////////////////////////////////////////////////////////////////

void DefineTextureMovie(void) {
    if (gTexMovieName == 0) {
        gTexMovieName = Xe_CreateTexture(xe, 256, 256, 1,  XE_FMT_8888 | XE_FMT_ARGB, 0);

        if (!bUseFastMdec) {
            gTexMovieName->use_filtering = XE_TEXF_LINEAR;
        }
        else{
            gTexMovieName->use_filtering = XE_TEXF_POINT;
        }
        gTexMovieName->u_addressing = iClampType;
        gTexMovieName->v_addressing = iClampType;
        
        xeGfx_setTextureData(gTexMovieName, texturepart);
    }
    gTexName = gTexMovieName;

    XeTexSubImage(0,0,(xrMovieArea.x1 - xrMovieArea.x0),(xrMovieArea.y1 - xrMovieArea.y0),texturepart);
}

////////////////////////////////////////////////////////////////////////
// movie texture: load
////////////////////////////////////////////////////////////////////////

#define MRED(x)   ((x>>3) & 0x1f)
#define MGREEN(x) ((x>>6) & 0x3e0)
#define MBLUE(x)  ((x>>9) & 0x7c00)

#define XMGREEN(x) ((x>>5)  & 0x07c0)
#define XMRED(x)   ((x<<8)  & 0xf800)
#define XMBLUE(x)  ((x>>18) & 0x003e)

////////////////////////////////////////////////////////////////////////
// movie texture: load
////////////////////////////////////////////////////////////////////////

unsigned char * LoadDirectMovieFast(void) {

    TR;

    int row, column;
    unsigned int startxy;

    uint32_t *ta = (uint32_t *) texturepart;

    if (PSXDisplay.RGB24) {
        unsigned char * pD;

        startxy = ((1024) * xrMovieArea.y0) + xrMovieArea.x0;

        for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++, startxy += 1024) {
            pD = (unsigned char *) &psxVuw[startxy];
            for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                *ta++ = *((uint32_t *) pD) | 0xff000000;
                pD += 3;
            }
        }
    } else {
        uint32_t(*LTCOL)(uint32_t);

        LTCOL = XP8RGBA_0; //TCF[0];

        ubOpaqueDraw = 0;

        for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
            startxy = ((1024) * column) + xrMovieArea.x0;
            for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                *ta++ = LTCOL(psxVuw[startxy++] | 0x8000);
        }
    }

    return texturepart;
}

////////////////////////////////////////////////////////////////////////

struct XenosSurface * LoadTextureMovieFast(void) {
    int row, column;
    unsigned int start, startxy;

    if (bGLFastMovie) {
        if (PSXDisplay.RGB24) {
            unsigned char * pD;
            uint32_t lu1, lu2;
            unsigned short * ta = (unsigned short *) texturepart;
            short sx0 = xrMovieArea.x1 - 1;

            start = 0;

            startxy = ((1024) * xrMovieArea.y0) + xrMovieArea.x0;
            for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                pD = (unsigned char *) &psxVuw[startxy];
                startxy += 1024;

                for (row = xrMovieArea.x0; row < sx0; row += 2) {
                    lu1 = *((uint32_t *) pD);
                    pD += 3;
                    lu2 = *((uint32_t *) pD);
                    pD += 3;

                    *((uint32_t *) ta) =
                            (XMBLUE(lu1) | XMGREEN(lu1) | XMRED(lu1) | 1) |
                            ((XMBLUE(lu2) | XMGREEN(lu2) | XMRED(lu2) | 1) << 16);
                    ta += 2;
                }
                if (row == sx0) {
                    lu1 = *((uint32_t *) pD);
                    *ta++ = XMBLUE(lu1) | XMGREEN(lu1) | XMRED(lu1) | 1;
                }
            }
        } else {
            unsigned short *ta = (unsigned short *) texturepart;
            uint32_t lc;
            short sx0 = xrMovieArea.x1 - 1;

            for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                startxy = ((1024) * column) + xrMovieArea.x0;
                for (row = xrMovieArea.x0; row < sx0; row += 2) {
                    lc = *((uint32_t *) & psxVuw[startxy]);
                    *((uint32_t *) ta) =
                            ((lc & 0x001f001f) << 11) | ((lc & 0x03e003e0) << 1) | ((lc & 0x7c007c00) >> 9) | 0x00010001;
                    ta += 2;
                    startxy += 2;
                }
                if (row == sx0) *ta++ = (psxVuw[startxy] << 1) | 1;
            }
        }
        DefinePackedTextureMovie();
    } else {
        if (PSXDisplay.RGB24) {
            unsigned char *pD;
            uint32_t *ta = (uint32_t *) texturepart;

            startxy = ((1024) * xrMovieArea.y0) + xrMovieArea.x0;

/*
            printf("xrMovieArea.y0 = %d\r\n",xrMovieArea.y0);
            printf("xrMovieArea.x0 = %d\r\n",xrMovieArea.x0);
            printf("xrMovieArea.y1 = %d\r\n",xrMovieArea.y1);
            printf("xrMovieArea.x1 = %d\r\n",xrMovieArea.x1);
*/

            for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++, startxy += 1024) {
                //startxy=((1024)*column)+xrMovieArea.x0;
                pD = (unsigned char *) &psxVuw[startxy];
                for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                    *ta++ = *((uint32_t *) pD) | 0xff000000;
                    pD += 3;
                }
            }
            //TR;
        } else {
            uint32_t(*LTCOL)(uint32_t);
            uint32_t s;
            uint32_t *ta;

            LTCOL = XP8RGBA_0; //TCF[0];

            ubOpaqueDraw = 0;
            ta = (uint32_t *) texturepart;

            for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                startxy = (1024 * column) + xrMovieArea.x0;
                for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++){
                    *ta++ = LTCOL(psxVuw[startxy++] | 0x8000);
                }

            }
            //TR;
        }
        //TR;
        DefineTextureMovie();
    }
    return gTexName;
}

////////////////////////////////////////////////////////////////////////

struct XenosSurface * LoadTextureMovie(void) {
    short row, column, dx;
    unsigned int startxy;
    BOOL b_X, b_Y;

    if (bUseFastMdec) return LoadTextureMovieFast();

    b_X = FALSE;
    b_Y = FALSE;

    if ((xrMovieArea.x1 - xrMovieArea.x0) < 255) b_X = TRUE;
    if ((xrMovieArea.y1 - xrMovieArea.y0) < 255) b_Y = TRUE;

    if (bGLFastMovie) {
        unsigned short c;

        if (PSXDisplay.RGB24) {
            unsigned char * pD;
            uint32_t lu;
            unsigned short * ta = (unsigned short *) texturepart;

            if (b_X) {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    pD = (unsigned char *) &psxVuw[startxy];
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        lu = *((uint32_t *) pD);
                        pD += 3;
                        *ta++ = XMBLUE(lu) | XMGREEN(lu) | XMRED(lu) | 1;
                    }
                    *ta++ = *(ta - 1);
                }
                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0 + 1;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = *(ta - dx);
                    *ta++ = *(ta - 1);
                }
            } else {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    pD = (unsigned char *) &psxVuw[startxy];
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        lu = *((uint32_t *) pD);
                        pD += 3;
                        *ta++ = XMBLUE(lu) | XMGREEN(lu) | XMRED(lu) | 1;
                    }
                }
                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = *(ta - dx);
                }
            }
        } else {
            unsigned short *ta;

            ubOpaqueDraw = 0;

            ta = (unsigned short *) texturepart;

            if (b_X) {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        c = psxVuw[startxy++];
                        *ta++ = ((c & 0x1f) << 11) | ((c & 0x3e0) << 1) | ((c & 0x7c00) >> 9) | 1;
                    }

                    *ta++ = *(ta - 1);
                }
                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0 + 1;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = *(ta - dx);
                    *ta++ = *(ta - 1);
                }
            } else {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        c = psxVuw[startxy++];
                        *ta++ = ((c & 0x1f) << 11) | ((c & 0x3e0) << 1) | ((c & 0x7c00) >> 9) | 1;
                    }
                }
                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = *(ta - dx);
                }
            }
        }
        xrMovieArea.x1 += b_X;
        xrMovieArea.y1 += b_Y;
        DefinePackedTextureMovie();
        xrMovieArea.x1 -= b_X;
        xrMovieArea.y1 -= b_Y;
    } else {
        if (PSXDisplay.RGB24) {
            unsigned char * pD;
            uint32_t * ta = (uint32_t *) texturepart;

            if (b_X) {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    pD = (unsigned char *) &psxVuw[startxy];
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        *ta++ = *((uint32_t *) pD) | 0xff000000;
                        pD += 3;
                    }
                    *ta++ = *(ta - 1);
                }
                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0 + 1;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = *(ta - dx);
                    *ta++ = *(ta - 1);
                }
            } else {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    pD = (unsigned char *) &psxVuw[startxy];
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        *ta++ = *((uint32_t *) pD) | 0xff000000;
                        pD += 3;
                    }
                }
                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = *(ta - dx);
                }
            }
        } else {
            uint32_t(*LTCOL)(uint32_t);
            uint32_t *ta;

            LTCOL = XP8RGBA_0; //TCF[0];

            ubOpaqueDraw = 0;
            ta = (uint32_t *) texturepart;

            if (b_X) {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = LTCOL(psxVuw[startxy++] | 0x8000);
                    *ta++ = *(ta - 1);
                }

                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0 + 1;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = *(ta - dx);
                    *ta++ = *(ta - 1);
                }
            } else {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = LTCOL(psxVuw[startxy++] | 0x8000);
                }

                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                        *ta++ = *(ta - dx);
                }
            }
        }

        xrMovieArea.x1 += b_X;
        xrMovieArea.y1 += b_Y;
        DefineTextureMovie();
        xrMovieArea.x1 -= b_X;
        xrMovieArea.y1 -= b_Y;
    }
    return gTexName;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct XenosSurface * BlackFake15BitTexture(void) {
    int pmult;
    short x1, x2, y1, y2;

    if (PSXDisplay.InterlacedTest) return 0;

    pmult = GlobalTexturePage / 16;
    x1 = gl_ux[7];
    x2 = gl_ux[6] - gl_ux[7];
    y1 = gl_ux[5];
    y2 = gl_ux[4] - gl_ux[5];

    if (iSpriteTex) {
        if (x2 < 255) x2++;
        if (y2 < 255) y2++;
    }

    y1 += pmult * 256;
    x1 += ((GlobalTexturePage - 16 * pmult) << 6);

    if (FastCheckAgainstFrontScreen(x1, y1, x2, y2)
            || FastCheckAgainstScreen(x1, y1, x2, y2)) {
        if (!gTexFrameName) {
            /*
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, iClampType);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, iClampType);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
*/
            if (bGLExt) {
                unsigned short s;
                unsigned short * ta;

/*
                if (giWantedTYPE == GL_UNSIGNED_SHORT_4_4_4_4_EXT)
                    s = 0x000f;
                else
*/
                    s = 0x0001;

                ta = (unsigned short *) texturepart;
                for (y1 = 0; y1 <= 4; y1++)
                    for (x1 = 0; x1 <= 4; x1++)
                        *ta++ = s;
            } else {
                uint32_t *ta = (uint32_t *) texturepart;
                for (y1 = 0; y1 <= 4; y1++)
                    for (x1 = 0; x1 <= 4; x1++)
                        *ta++ = 0xff000000;
            }
            TR
            //glTexImage2D(GL_TEXTURE_2D, 0, giWantedRGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, texturepart);
           // XeTexImage(gTexName,4, 4,texturepart);
            gTexName = Xe_CreateTexture(xe, 4, 4, 1,  XE_FMT_8888 | XE_FMT_ARGB, 0);
            xeGfx_setTextureData(gTexName,texturepart);
            gTexName->u_addressing = iClampType;
            gTexName->v_addressing = iClampType;
            gTexName->use_filtering = XE_TEXF_POINT;

        } else {
            gTexName = gTexFrameName;
            //XeBindTexture(gTexName);
            xeSetTexture();
        }

        ubOpaqueDraw = 0;

        return gTexName;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

BOOL bFakeFrontBuffer = FALSE;
BOOL bIgnoreNextTile = FALSE;

int iFTex = 512;

struct XenosSurface * Fake15BitTexture(void) {
   
    int pmult;
    short x1, x2, y1, y2;
    int iYAdjust;
    float ScaleX, ScaleY;
    RECT rSrc;

    if (iFrameTexType == 1) return BlackFake15BitTexture();
    if (PSXDisplay.InterlacedTest) return 0;

    pmult = GlobalTexturePage / 16;
    x1 = gl_ux[7];
    x2 = gl_ux[6] - gl_ux[7];
    y1 = gl_ux[5];
    y2 = gl_ux[4] - gl_ux[5];

    y1 += pmult * 256;
    x1 += ((GlobalTexturePage - 16 * pmult) << 6);

    if (iFrameTexType == 3) {
        if (iFrameReadType == 4) return 0;

        if (!FastCheckAgainstFrontScreen(x1, y1, x2, y2) &&
                !FastCheckAgainstScreen(x1, y1, x2, y2))
            return 0;

        if (bFakeFrontBuffer) bIgnoreNextTile = TRUE;
        CheckVRamReadEx(x1, y1, x1 + x2, y1 + y2);
        return 0;
    }

    /////////////////////////

    if (FastCheckAgainstFrontScreen(x1, y1, x2, y2)) {
        x1 -= PSXDisplay.DisplayPosition.x;
        y1 -= PSXDisplay.DisplayPosition.y;
    } else
        if (FastCheckAgainstScreen(x1, y1, x2, y2)) {
        x1 -= PreviousPSXDisplay.DisplayPosition.x;
        y1 -= PreviousPSXDisplay.DisplayPosition.y;
    } else return 0;

    bDrawMultiPass = FALSE;

    if (!gTexFrameName) {
        char * p;

        if (iResX > 1280 || iResY > 1024) iFTex = 2048;
        else
            if (iResX > 640 || iResY > 480) iFTex = 1024;
        else iFTex = 512;

        gTexName = gTexFrameName;

/*
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, iClampType);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, iClampType);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
*/
        p = (char *) malloc(iFTex * iFTex * 4);
        memset(p, 0, iFTex * iFTex * 4);
        //glTexImage2D(GL_TEXTURE_2D, 0, 3, iFTex, iFTex, 0, GL_RGB, GL_UNSIGNED_BYTE, p);

        gTexName = Xe_CreateTexture(xe, iFTex, iFTex, 1,  XE_FMT_8888 | XE_FMT_ARGB, 0);
        xeGfx_setTextureData(gTexName,texturepart);
        gTexName->u_addressing = iClampType;
        gTexName->v_addressing = iClampType;
        TR
        free(p);

        glGetError();
    } else {
        gTexName = gTexFrameName;
    }

    x1 += PreviousPSXDisplay.Range.x0;
    y1 += PreviousPSXDisplay.Range.y0;

    if (PSXDisplay.DisplayMode.x)
        ScaleX = (float) rRatioRect.right / (float) PSXDisplay.DisplayMode.x;
    else ScaleX = 1.0f;
    if (PSXDisplay.DisplayMode.y)
        ScaleY = (float) rRatioRect.bottom / (float) PSXDisplay.DisplayMode.y;
    else ScaleY = 1.0f;

    rSrc.left = max(x1*ScaleX, 0);
    rSrc.right = min((x1 + x2) * ScaleX + 0.99f, iResX - 1);
    rSrc.top = max(y1*ScaleY, 0);
    rSrc.bottom = min((y1 + y2) * ScaleY + 0.99f, iResY - 1);

    iYAdjust = (y1 + y2) - PSXDisplay.DisplayMode.y;
    if (iYAdjust > 0)
        iYAdjust = (int) ((float) iYAdjust * ScaleY) + 1;
    else iYAdjust = 0;

    gl_vy[0] = 255 - gl_vy[0];
    gl_vy[1] = 255 - gl_vy[1];
    gl_vy[2] = 255 - gl_vy[2];
    gl_vy[3] = 255 - gl_vy[3];

    y1 = min(gl_vy[0], min(gl_vy[1], min(gl_vy[2], gl_vy[3])));

    gl_vy[0] -= y1;
    gl_vy[1] -= y1;
    gl_vy[2] -= y1;
    gl_vy[3] -= y1;
    gl_ux[0] -= gl_ux[7];
    gl_ux[1] -= gl_ux[7];
    gl_ux[2] -= gl_ux[7];
    gl_ux[3] -= gl_ux[7];

    ScaleX *= 256.0f / ((float) (iFTex));
    ScaleY *= 256.0f / ((float) (iFTex));

    y1 = ((float) gl_vy[0] * ScaleY);
    if (y1 > 255) y1 = 255;
    gl_vy[0] = y1;
    y1 = ((float) gl_vy[1] * ScaleY);
    if (y1 > 255) y1 = 255;
    gl_vy[1] = y1;
    y1 = ((float) gl_vy[2] * ScaleY);
    if (y1 > 255) y1 = 255;
    gl_vy[2] = y1;
    y1 = ((float) gl_vy[3] * ScaleY);
    if (y1 > 255) y1 = 255;
    gl_vy[3] = y1;

    x1 = ((float) gl_ux[0] * ScaleX);
    if (x1 > 255) x1 = 255;
    gl_ux[0] = x1;
    x1 = ((float) gl_ux[1] * ScaleX);
    if (x1 > 255) x1 = 255;
    gl_ux[1] = x1;
    x1 = ((float) gl_ux[2] * ScaleX);
    if (x1 > 255) x1 = 255;
    gl_ux[2] = x1;
    x1 = ((float) gl_ux[3] * ScaleX);
    if (x1 > 255) x1 = 255;
    gl_ux[3] = x1;

    x1 = rSrc.right - rSrc.left;
    if (x1 <= 0) x1 = 1;
    if (x1 > iFTex) x1 = iFTex;

    y1 = rSrc.bottom - rSrc.top;
    if (y1 <= 0) y1 = 1;
    if (y1 + iYAdjust > iFTex) y1 = iFTex - iYAdjust;

/*
    if (bFakeFrontBuffer)
        glReadBuffer(GL_FRONT);

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
            0,
            iYAdjust,
            rSrc.left + rRatioRect.left,
            iResY - rSrc.bottom - rRatioRect.top,
            x1, y1);
*/

    if (glGetError()) {
        char * p = (char *) malloc(iFTex * iFTex * 4);
        memset(p, 0, iFTex * iFTex * 4);
/*
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iFTex, iFTex,
                GL_RGB, GL_UNSIGNED_BYTE, p);
*/
        free(p);
    }

    if (bFakeFrontBuffer) {
/*
        glReadBuffer(GL_BACK);
*/
        bIgnoreNextTile = TRUE;
    }

    ubOpaqueDraw = 0;

    if (iSpriteTex) {
        sprtW = gl_ux[1] - gl_ux[0];
        sprtH = -(gl_vy[0] - gl_vy[2]);
    }

    return gTexName;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// load texture part (unpacked) the good one
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void LoadSubTexturePageSort(int pageid, int mode, short cx, short cy) {
    //TR
    uint32_t start, row, column, j, sxh, sxm;
    unsigned int palstart;
    uint32_t *px, *pa, *ta;
    unsigned char *cSRCPtr;
    unsigned short *wSRCPtr;
    uint32_t LineOffset;
    uint32_t x2a, xalign = 0;
    uint32_t x1 = gl_ux[7];
    uint32_t x2 = gl_ux[6];
    uint32_t y1 = gl_ux[5];
    uint32_t y2 = gl_ux[4];
    uint32_t dx = x2 - x1 + 1;
    uint32_t dy = y2 - y1 + 1;
    int pmult = pageid / 16;
    uint32_t(*LTCOL)(uint32_t);
    unsigned int a, r, g, b, cnt, h;
    uint32_t scol[8];

    LTCOL = TCF[DrawSemiTrans];

    pa = px = (uint32_t *) ubPaletteBuffer;
    ta = (uint32_t *) texturepart;
    palstart = cx + (cy << 10);

    ubOpaqueDraw = 0;

    if (YTexS) {
        ta += dx;
        if (XTexS) ta += 2;
    }
    if (XTexS) {
        ta += 1;
        xalign = 2;
    }

/*

    printf("YTexS : %d | XTexS : %d\r\n",YTexS,XTexS);
    printf(" x1 | x2 | y1 | y2 | pageid\r\n");
    printf("%4d|%4d|%4d|%4d|%4d\r\n",x1,x2,y1,y2,pageid);
    printf("GlobalTextAddrY = %d\r\n",GlobalTextAddrY);
    printf("GlobalTextAddrX = %d\r\n",GlobalTextAddrX);
*/


    switch (mode) {
            //--------------------------------------------------//
            // 4bit texture load ..
        case 0:
           // TR;
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 4;
                do {
/*
                    *px = (LTCOL(( *wSRCPtr)));
                    *(px + 1) = (LTCOL((*wSRCPtr+1)));
                    *(px + 2) = (LTCOL((*wSRCPtr+2)));
                    *(px + 3) = (LTCOL((*wSRCPtr+3)));
*/
                    *px = (LTCOL(GETLE32(wSRCPtr)));
                    *(px + 1) = (LTCOL(GETLE32(wSRCPtr+1)));
                    *(px + 2) = (LTCOL(GETLE32(wSRCPtr+2)));
                    *(px + 3) = (LTCOL(GETLE32(wSRCPtr+3)));
                    
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                for (TXV = y1; TXV <= y2; TXV++) {
                    for (TXU = x1; TXU <= x2; TXU++) {
                        n_xi = ((TXU >> 2) & ~0x3c) + ((TXV << 2) & 0x3c);
                        n_yi = (TXV & ~0xf) + ((TXU >> 4) & 0xf);

                        //*ta++ = *(pa + ((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x03) << 2)) & 0x0f));
                        *ta++ = *(pa + ((GETLE32(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x03) << 2)) & 0x0f));
                    }
                    ta += xalign;
                }
                break;
            }


            start = ((pageid - 16 * pmult) << 7) + 524288 * pmult;
            // convert CLUT to 32bits .. and then use THAT as a lookup table
/*
            printf("start= %08x\r\n",start);
*/
            wSRCPtr = psxVuw + palstart;

            row = 4;
            do {
/*
                *px = (LTCOL((*wSRCPtr)));
                *(px + 1) = (LTCOL((*(wSRCPtr + 1))));
                *(px + 2) = (LTCOL((*(wSRCPtr + 2))));
                *(px + 3) = (LTCOL((*(wSRCPtr + 3))));
*/
                *px = (LTCOL(GETLE32(wSRCPtr)));
                *(px + 1) = (LTCOL(GETLE32(wSRCPtr + 1)));
                *(px + 2) = (LTCOL(GETLE32(wSRCPtr + 2)));
                *(px + 3) = (LTCOL(GETLE32(wSRCPtr + 3)));
                row--;
                px += 4;
                wSRCPtr += 4;
            } while (row);

            x2a = x2 ? (x2 - 1) : 0; //if(x2) x2a=x2-1; else x2a=0;
            sxm = x1 & 1;
            sxh = x1 >> 1;
            j = sxm ? (x1 + 1) : x1; //if(sxm) j=x1+1; else j=x1;
            for (column = y1; column <= y2; column++) {
                cSRCPtr = psxVub + start + (column << 11) + sxh;

                if (sxm) 
                {
                    //*ta++ = (*(pa + (((*cSRCPtr++) >> 4) & 0xF)));
                    *ta++ = (*(pa + ((GETLE32(cSRCPtr++) >> 4) & 0xF)));
                }

                for (row = j; row < x2a; row += 2) {
                    //*ta = (*(pa + ((*cSRCPtr++) & 0xF)));
                    //*(ta + 1) = (*(pa + (((*cSRCPtr++) >> 4) & 0xF)));
                    *ta = (*(pa + (GETLE32(cSRCPtr++) & 0xF)));
                    *(ta + 1) = (*(pa + ((GETLE32(cSRCPtr++) >> 4) & 0xF)));
                    cSRCPtr++;
                    ta += 2;
                }

                if (row <= x2) {
                   // *ta++ = (*(pa + ((*cSRCPtr++) & 0xF)));
                     *ta++ = (*(pa + (GETLE32(cSRCPtr++) & 0xF)));
                    row++;
                    if (row <= x2) {
                        // *ta++ = (*(pa + (((*cSRCPtr++) >> 4) & 0xF)));
                        *ta++ = (*(pa + ((GETLE32(cSRCPtr++) >> 4) & 0xF)));
                    }
                }

                ta += xalign;
            }

            break;
            //--------------------------------------------------//
            // 8bit texture load ..
        case 1:
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 64;
                do {
/*
                    *px = LTCOL((*wSRCPtr));
                    *(px + 1) = LTCOL((*(wSRCPtr + 1)));
                    *(px + 2) = LTCOL((*(wSRCPtr + 2)));
                    *(px + 3) = LTCOL((*(wSRCPtr + 3)));
*/
                    *px = (LTCOL(GETLE32(wSRCPtr)));
                    *(px + 1) = (LTCOL(GETLE32(wSRCPtr + 1)));
                    *(px + 2) = (LTCOL(GETLE32(wSRCPtr + 2)));
                    *(px + 3) = (LTCOL(GETLE32(wSRCPtr + 3)));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                for (TXV = y1; TXV <= y2; TXV++) {
                    for (TXU = x1; TXU <= x2; TXU++) {
                        n_xi = ((TXU >> 1) & ~0x78) + ((TXU << 2) & 0x40) + ((TXV << 3) & 0x38);
                        n_yi = (TXV & ~0x7) + ((TXU >> 5) & 0x7);

                        //*ta++ = *(pa + (((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi)) >> ((TXU & 0x01) << 3)) & 0xff));
                        *ta++ = *(pa + (((GETLE32(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi)) >> ((TXU & 0x01) << 3)) & 0xff));
                    }
                    ta += xalign;
                }

                break;
            }

            start = ((pageid - 16 * pmult) << 7) + 524288 * pmult;

            cSRCPtr = psxVub + start + (y1 << 11) + x1;
            LineOffset = 2048 - dx;

            if (dy * dx > 384) {
                wSRCPtr = psxVuw + palstart;

                row = 64;
                do {
/*
                    *px = LTCOL(*wSRCPtr);
                    *(px + 1) = LTCOL(*(wSRCPtr + 1));
                    *(px + 2) = LTCOL(*(wSRCPtr + 2));
                    *(px + 3) = LTCOL(*(wSRCPtr + 3));
*/
                    *px = (LTCOL(GETLE32(wSRCPtr)));
                    *(px + 1) = (LTCOL(GETLE32(wSRCPtr + 1)));
                    *(px + 2) = (LTCOL(GETLE32(wSRCPtr + 2)));
                    *(px + 3) = (LTCOL(GETLE32(wSRCPtr + 3)));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                column = dy;
                do {
                    row = dx;
                    do {
                        *ta++ = *(pa + (*cSRCPtr++));
                        //*ta++ = *(pa + GETLE32(cSRCPtr++)); // crash
                        row--;
                    } while (row);
                    ta += xalign;
                    cSRCPtr += LineOffset;
                    column--;
                } while (column);
            } else {
                wSRCPtr = psxVuw + palstart;

                column = dy;
                do {
                    row = dx;
                    do {
                        //*ta++ = LTCOL(*(wSRCPtr + *cSRCPtr++));
                        *ta++ = LTCOL(GETLE32(wSRCPtr + *cSRCPtr++));
                        // *ta++ = LTCOL(GETLE32(wSRCPtr + GETLE32(cSRCPtr++))); // crash ?
                        row--;
                    } while (row);
                    ta += xalign;
                    cSRCPtr += LineOffset;
                    column--;
                } while (column);
            }

            break;
            //--------------------------------------------------//
            // 16bit texture load ..
        case 2:
            //TR
            start = ((pageid - 16 * pmult) << 6) + 262144 * pmult;

            wSRCPtr = psxVuw + start + (y1 << 10) + x1;
            LineOffset = 1024 - dx;

            column = dy;
            do {
                row = dx;
                do {
                    //*ta++ = LTCOL(*wSRCPtr++);
                    *ta++ = (LTCOL(GETLE32(wSRCPtr++)));
                    row--;
                } while (row);
                ta += xalign;
                wSRCPtr += LineOffset;
                column--;
            } while (column);

            break;
            //--------------------------------------------------//
            // others are not possible !
    }

    x2a = dx + xalign;

    if (YTexS) {
        ta = (uint32_t *) texturepart;
        pa = (uint32_t *) texturepart + x2a;
        row = x2a;
        do {
            *ta++ = GETLE32(pa++);
            row--;
        } while (row);
        pa = (uint32_t *) texturepart + dy*x2a;
        ta = pa + x2a;
        row = x2a;
        do {
            *ta++ = GETLE32(pa++);
            row--;
        } while (row);
        YTexS--;
        dy += 2;
    }

    if (XTexS) {
        ta = (uint32_t *) texturepart;
        pa = ta + 1;
        row = dy;
        do {
            *ta = GETLE32(pa);
            ta += x2a;
            pa += x2a;
            row--;
        } while (row);
        pa = (uint32_t *) texturepart + dx;
        ta = pa + 1;
        row = dy;
        do {
            *ta = GETLE32(pa);
            ta += x2a;
            pa += x2a;
            row--;
        } while (row);
        XTexS--;
        dx += 2;
    }

    DXTexS = dx;
    DYTexS = dy;

    if (!iFilterType) {
        DefineSubTextureSort();
        return;
    }
    if (iFilterType != 2 && iFilterType != 4 && iFilterType != 6) {
        DefineSubTextureSort();
        return;
    }
    if ((iFilterType == 4 || iFilterType == 6) && ly0 == ly1 && ly2 == ly3 && lx0 == lx3 && lx1 == lx2) {
        DefineSubTextureSort();
        return;
    }

    ta = (uint32_t *) texturepart;
    x1 = dx - 1;
    y1 = dy - 1;

    if (bOpaquePass) {
        if (bSmallAlpha) {
            for (column = 0; column < dy; column++) {
                for (row = 0; row < dx; row++) {
                    if (*ta == 0x03000000) {
                        cnt = 0;

                        if (column && *(ta - dx) >> 24 != 0x03) scol[cnt++] = *(ta - dx);
                        if (row && *(ta - 1) >> 24 != 0x03) scol[cnt++] = *(ta - 1);
                        if (row != x1 && *(ta + 1) >> 24 != 0x03) scol[cnt++] = *(ta + 1);
                        if (column != y1 && *(ta + dx) >> 24 != 0x03) scol[cnt++] = *(ta + dx);

                        if (row && column && *(ta - dx - 1) >> 24 != 0x03) scol[cnt++] = *(ta - dx - 1);
                        if (row != x1 && column && *(ta - dx + 1) >> 24 != 0x03) scol[cnt++] = *(ta - dx + 1);
                        if (row && column != y1 && *(ta + dx - 1) >> 24 != 0x03) scol[cnt++] = *(ta + dx - 1);
                        if (row != x1 && column != y1 && *(ta + dx + 1) >> 24 != 0x03) scol[cnt++] = *(ta + dx + 1);

                        if (cnt) {
                            r = g = b = a = 0;
                            for (h = 0; h < cnt; h++) {
                                r += (scol[h] >> 16)&0xff;
                                g += (scol[h] >> 8)&0xff;
                                b += scol[h]&0xff;
                            }
                            r /= cnt;
                            b /= cnt;
                            g /= cnt;

                            *ta = (r << 16) | (g << 8) | b;
                            *ta |= 0x03000000;
                        }
                    }
                    ta++;
                }
            }
        } else {
            for (column = 0; column < dy; column++) {
                for (row = 0; row < dx; row++) {
                    if (*ta == 0x50000000) {
                        cnt = 0;

                        if (column && *(ta - dx) != 0x50000000 && *(ta - dx) >> 24 != 1) scol[cnt++] = *(ta - dx);
                        if (row && *(ta - 1) != 0x50000000 && *(ta - 1) >> 24 != 1) scol[cnt++] = *(ta - 1);
                        if (row != x1 && *(ta + 1) != 0x50000000 && *(ta + 1) >> 24 != 1) scol[cnt++] = *(ta + 1);
                        if (column != y1 && *(ta + dx) != 0x50000000 && *(ta + dx) >> 24 != 1) scol[cnt++] = *(ta + dx);

                        if (row && column && *(ta - dx - 1) != 0x50000000 && *(ta - dx - 1) >> 24 != 1) scol[cnt++] = *(ta - dx - 1);
                        if (row != x1 && column && *(ta - dx + 1) != 0x50000000 && *(ta - dx + 1) >> 24 != 1) scol[cnt++] = *(ta - dx + 1);
                        if (row && column != y1 && *(ta + dx - 1) != 0x50000000 && *(ta + dx - 1) >> 24 != 1) scol[cnt++] = *(ta + dx - 1);
                        if (row != x1 && column != y1 && *(ta + dx + 1) != 0x50000000 && *(ta + dx + 1) >> 24 != 1) scol[cnt++] = *(ta + dx + 1);

                        if (cnt) {
                            r = g = b = a = 0;
                            for (h = 0; h < cnt; h++) {
                                a += (scol[h] >> 24);
                                r += (scol[h] >> 16)&0xff;
                                g += (scol[h] >> 8)&0xff;
                                b += scol[h]&0xff;
                            }
                            r /= cnt;
                            b /= cnt;
                            g /= cnt;

                            *ta = (r << 16) | (g << 8) | b;
                            if (a) *ta |= 0x50000000;
                            else *ta |= 0x01000000;
                        }
                    }
                    ta++;
                }
            }
        }
    } else
        for (column = 0; column < dy; column++) {
            for (row = 0; row < dx; row++) {
                if (*ta == 0x00000000) {
                    cnt = 0;

                    if (row != x1 && *(ta + 1) != 0x00000000) scol[cnt++] = *(ta + 1);
                    if (column != y1 && *(ta + dx) != 0x00000000) scol[cnt++] = *(ta + dx);

                    if (cnt) {
                        r = g = b = 0;
                        for (h = 0; h < cnt; h++) {
                            r += (scol[h] >> 16)&0xff;
                            g += (scol[h] >> 8)&0xff;
                            b += scol[h]&0xff;
                        }
                        r /= cnt;
                        b /= cnt;
                        g /= cnt;
                        *ta = (r << 16) | (g << 8) | b;
                    }
                }
                ta++;
            }
        }

    DefineSubTextureSort();
}


/////////////////////////////////////////////////////////////////////////////
#include "tex2Sai.h"
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// ogl texture defines
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


void DefineSubTextureSortHiRes(void) {
    int x, y, dx2;

    if (!gTexName) {
        gTexName = Xe_CreateTexture(xe, 512, 512, 1,  XE_FMT_8888 | XE_FMT_ARGB, 0);
        if (iFilterType) {
            gTexName->use_filtering = XE_TEXF_LINEAR;
        }
        else{
            gTexName->use_filtering = XE_TEXF_POINT;
        }
        gTexName->u_addressing = iClampType;
        gTexName->v_addressing = iClampType;
        xeGfx_setTextureData(gTexName, texturepart);
    }

    if (bGLExt && (iTexQuality == 1 || iTexQuality == 2)) {
        if (DXTexS < 4 || DYTexS < 4 || iHiResTextures == 2) {
            unsigned short * pS, *pD1, *pD2;
            dx2 = (DXTexS << 1);
            pS = (unsigned short *) texturepart;
            pD1 = (unsigned short *) texturebuffer;
            pD2 = (unsigned short *) texturebuffer;
            pD2 += dx2;
            for (y = 0; y < DYTexS; y++) {
                for (x = 0; x < DXTexS; x++) {
                    *(pD2 + 1) = *pD2 = *(pD1 + 1) = *pD1 = *pS;
                    pS++;
                    pD1 += 2;
                    pD2 += 2;
                }
                pD1 += dx2;
                pD2 += dx2;
            }
        } else {
            if (iTexQuality == 1)
                Super2xSaI_ex4(texturepart, DXTexS << 1, texturebuffer, DXTexS, DYTexS);
            else
                Super2xSaI_ex5(texturepart, DXTexS << 1, texturebuffer, DXTexS, DYTexS);
        }
    } else {
        if (DXTexS < 4 || DYTexS < 4 || iHiResTextures == 2) {
            uint32_t * pS, *pD1, *pD2;
            dx2 = (DXTexS << 1);
            pS = (uint32_t *) texturepart;
            pD1 = (uint32_t *) texturebuffer;
            pD2 = (uint32_t *) texturebuffer;
            pD2 += dx2;
            for (y = 0; y < DYTexS; y++) {
                for (x = 0; x < DXTexS; x++) {
                    *(pD2 + 1) = *pD2 = *(pD1 + 1) = *pD1 = *pS;
                    pS++;
                    pD1 += 2;
                    pD2 += 2;
                }
                pD1 += dx2;
                pD2 += dx2;
            }
        } else
            if (bSmallAlpha)
            Super2xSaI_ex8_Ex(texturepart, DXTexS * 4, texturebuffer, DXTexS, DYTexS);
        else
            Super2xSaI_ex8(texturepart, DXTexS * 4, texturebuffer, DXTexS, DYTexS);
    }

    XeTexSubImage(XTexS << 1, YTexS << 1,
            DXTexS << 1, DYTexS << 1,texturebuffer);
     xeSetTexture();

/*
    glTexSubImage2D(GL_TEXTURE_2D, 0, XTexS << 1, YTexS << 1,
            DXTexS << 1, DYTexS << 1,
            giWantedFMT, giWantedTYPE, texturebuffer);
*/
}


/////////////////////////////////////////////////////////////////////////////
void DefineSubTextureSort(void) {
    if (iHiResTextures) {
        DefineSubTextureSortHiRes();
        return;
    }

    if (!gTexName) {
        //glTexImage2D(GL_TEXTURE_2D, 0, giWantedRGBA, 256, 256, 0, giWantedFMT, giWantedTYPE, texturepart);
        gTexName = Xe_CreateTexture(xe, 256, 256, 1,  XE_FMT_8888 | XE_FMT_ARGB, 0);
        xeGfx_setTextureData(gTexName, texturepart);
        TR;
    }

    //TR;
    XeTexSubImage(XTexS, YTexS, DXTexS, DYTexS,texturepart);
}


/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// texture cache garbage collection
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void DoTexGarbageCollection(void) {
    static unsigned short LRUCleaned = 0;
    unsigned short iC, iC1, iC2;
    int i, j, iMax;
    textureSubCacheEntryS * tsb;

    iC = 4; //=iSortTexCnt/2,
    LRUCleaned += iC; // we clean different textures each time
    if ((LRUCleaned + iC) >= iSortTexCnt) LRUCleaned = 0; // wrap? wrap!
    iC1 = LRUCleaned; // range of textures to clean
    iC2 = LRUCleaned + iC;

    for (iC = iC1; iC < iC2; iC++) // make some textures available
    {
        pxSsubtexLeft[iC]->l = 0;
    }

    for (i = 0; i < 3; i++) // remove all references to that textures
        for (j = 0; j < MAXTPAGES; j++)
            for (iC = 0; iC < 4; iC++) // loop all texture rect info areas
            {
                tsb = pscSubtexStore[i][j]+(iC * SOFFB);
                iMax = tsb->pos.l;
                if (iMax)
                    do {
                        tsb++;
                        if (tsb->cTexID >= iC1 && tsb->cTexID < iC2) // info uses the cleaned textures? remove info
                            tsb->ClutID = 0;
                    } while (--iMax);
            }

    usLRUTexPage = LRUCleaned;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// search cache for existing (already used) parts
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void dump_gl_uv(){
    int pi = 0;
    for(pi=0;pi<8;pi++)
        printf("gl_ux[%d] => %x\r\n",pi,gl_ux[pi]);
}

unsigned char * CheckTextureInSubSCache(int TextureMode, uint32_t GivenClutId, unsigned short * pCache) {
    textureSubCacheEntryS * tsx, * tsb, *tsg; //, *tse=NULL;
    int i, iMax;
    EXLong npos;
    unsigned char cx, cy;
    int iC, j, k;
    uint32_t rx, ry, mx, my;
    EXLong * ul = 0, * uls;
    EXLong rfree;
    unsigned char cXAdj, cYAdj;

    //npos.l = GETLE32((uint32_t *)&gl_ux[4]);
    npos.l = *(uint32_t *)&gl_ux[4];

    //dump_gl_uv();
    //printf("npos.l => %x\r\n",npos.l);

    
    //--------------------------------------------------------------//
    // find matching texturepart first... speed up...
    //--------------------------------------------------------------//

    tsg = pscSubtexStore[TextureMode][GlobalTexturePage];
    tsg += ((GivenClutId & CLUTCHK) >> CLUTSHIFT) * SOFFB;

    iMax = tsg->pos.l;
    if (iMax) {
        i = iMax;
        tsb = tsg + 1;
        do {
            if (GivenClutId == tsb->ClutID &&
                    (INCHECK(tsb->pos, npos))) {
                {
                    cx = tsb->pos.c[3] - tsb->posTX;
                    cy = tsb->pos.c[1] - tsb->posTY;

                    gl_ux[0] -= cx;
                    gl_ux[1] -= cx;
                    gl_ux[2] -= cx;
                    gl_ux[3] -= cx;
                    gl_vy[0] -= cy;
                    gl_vy[1] -= cy;
                    gl_vy[2] -= cy;
                    gl_vy[3] -= cy;

                    ubOpaqueDraw = tsb->Opaque;
                    *pCache = tsb->cTexID;
                    return NULL;
                }
            }
            tsb++;
        } while (--i);
    }

    //----------------------------------------------------//

    cXAdj = 1;
    cYAdj = 1;

    rx = (int) gl_ux[6]-(int) gl_ux[7];
    ry = (int) gl_ux[4]-(int) gl_ux[5];

    tsx = NULL;
    tsb = tsg + 1;
    for (i = 0; i < iMax; i++, tsb++) {
        if (!tsb->ClutID) {
            tsx = tsb;
            break;
        }
    }

    if (!tsx) {
        iMax++;
        if (iMax >= SOFFB - 2) {
            if (iTexGarbageCollection) // gc mode?
            {
                if (*pCache == 0) {
                    dwTexPageComp |= (1 << GlobalTexturePage);
                    *pCache = 0xffff;
                    return 0;
                }

                iMax--;
                tsb = tsg + 1;

                for (i = 0; i < iMax; i++, tsb++) // 1. search other slots with same cluts, and unite the area
                    if (GivenClutId == tsb->ClutID) {
                        if (!tsx) {
                            tsx = tsb;
                            rfree.l = npos.l;
                        }//
                        else tsb->ClutID = 0;
                        rfree.c[3] = min(rfree.c[3], tsb->pos.c[3]);
                        rfree.c[2] = max(rfree.c[2], tsb->pos.c[2]);
                        rfree.c[1] = min(rfree.c[1], tsb->pos.c[1]);
                        rfree.c[0] = max(rfree.c[0], tsb->pos.c[0]);
                        MarkFree(tsb);
                    }

                if (tsx) // 3. if one or more found, create a new rect with bigger size
                {
                    *((uint32_t *) & gl_ux[4]) = npos.l = rfree.l;
                    
                    //npos.l = rfree.l;
                    //PUTLE32((uint32_t *)&gl_ux[4],npos.l);
                    
                    dump_gl_uv();
                    printf("npos.l => %x\r\n",npos.l);
                    printf("npos.l => %x\r\n",rfree.l);
                    
                    rx = (int) rfree.c[2]-(int) rfree.c[3];
                    ry = (int) rfree.c[0]-(int) rfree.c[1];
                    DoTexGarbageCollection();

                    goto ENDLOOP3;
                }
            }

            iMax = 1;
        }
        tsx = tsg + iMax;
        tsg->pos.l = iMax;
    }

    //----------------------------------------------------//
    // now get a free texture space
    //----------------------------------------------------//

    if (iTexGarbageCollection) usLRUTexPage = 0;

ENDLOOP3:

    rx += 3;
    if (rx > 255) {
        cXAdj = 0;
        rx = 255;
    }
    ry += 3;
    if (ry > 255) {
        cYAdj = 0;
        ry = 255;
    }

    iC = usLRUTexPage;

    for (k = 0; k < iSortTexCnt; k++) {
        uls = pxSsubtexLeft[iC];
        iMax = uls->l;
        ul = uls + 1;

        //--------------------------------------------------//
        // first time

        if (!iMax) {
            rfree.l = 0;

            if (rx > 252 && ry > 252) {
                uls->l = 1;
                ul->l = 0xffffffff;
                ul = 0;
                goto ENDLOOP;
            }

            if (rx < 253) {
                uls->l = uls->l + 1;
                ul->c[3] = rx;
                ul->c[2] = 255 - rx;
                ul->c[1] = 0;
                ul->c[0] = ry;
                ul++;
            }

            if (ry < 253) {
                uls->l = uls->l + 1;
                ul->c[3] = 0;
                ul->c[2] = 255;
                ul->c[1] = ry;
                ul->c[0] = 255 - ry;
            }
            ul = 0;
            goto ENDLOOP;
        }

        //--------------------------------------------------//
        for (i = 0; i < iMax; i++, ul++) {
            if (ul->l != 0xffffffff &&
                    ry <= ul->c[0] &&
                    rx <= ul->c[2]) {
                rfree = *ul;
                mx = ul->c[2] - 2;
                my = ul->c[0] - 2;
                if (rx < mx && ry < my) {
                    ul->c[3] += rx;
                    ul->c[2] -= rx;
                    ul->c[0] = ry;

                    for (ul = uls + 1, j = 0; j < iMax; j++, ul++)
                        if (ul->l == 0xffffffff) break;

                    if (j < CSUBSIZE - 2) {
                        if (j == iMax) uls->l = uls->l + 1;

                        ul->c[3] = rfree.c[3];
                        ul->c[2] = rfree.c[2];
                        ul->c[1] = rfree.c[1] + ry;
                        ul->c[0] = rfree.c[0] - ry;
                    }
                } else if (rx < mx) {
                    ul->c[3] += rx;
                    ul->c[2] -= rx;
                } else if (ry < my) {
                    ul->c[1] += ry;
                    ul->c[0] -= ry;
                } else {
                    ul->l = 0xffffffff;
                }
                ul = 0;
                goto ENDLOOP;
            }
        }

        //--------------------------------------------------//

        iC++;
        if (iC >= iSortTexCnt) iC = 0;
    }

    //----------------------------------------------------//
    // check, if free space got
    //----------------------------------------------------//

ENDLOOP:
    if (ul) {
        //////////////////////////////////////////////////////

        {
            dwTexPageComp = 0;

            for (i = 0; i < 3; i++) // cleaning up
                for (j = 0; j < MAXTPAGES; j++) {
                    tsb = pscSubtexStore[i][j];
                    (tsb + SOFFA)->pos.l = 0;
                    (tsb + SOFFB)->pos.l = 0;
                    (tsb + SOFFC)->pos.l = 0;
                    (tsb + SOFFD)->pos.l = 0;
                }
            for (i = 0; i < iSortTexCnt; i++) {
                ul = pxSsubtexLeft[i];
                ul->l = 0;
            }
            usLRUTexPage = 0;
        }

        //////////////////////////////////////////////////////
        iC = usLRUTexPage;
        uls = pxSsubtexLeft[usLRUTexPage];
        uls->l = 0;
        ul = uls + 1;
        rfree.l = 0;

        if (rx > 252 && ry > 252) {
            uls->l = 1;
            ul->l = 0xffffffff;
        } else {
            if (rx < 253) {
                uls->l = uls->l + 1;
                ul->c[3] = rx;
                ul->c[2] = 255 - rx;
                ul->c[1] = 0;
                ul->c[0] = ry;
                ul++;
            }
            if (ry < 253) {
                uls->l = uls->l + 1;
                ul->c[3] = 0;
                ul->c[2] = 255;
                ul->c[1] = ry;
                ul->c[0] = 255 - ry;
            }
        }
        tsg->pos.l = 1;
        tsx = tsg + 1;
    }

    rfree.c[3] += cXAdj;
    rfree.c[1] += cYAdj;

    tsx->cTexID = *pCache = iC;
    tsx->pos = npos;
    tsx->ClutID = GivenClutId;
    tsx->posTX = rfree.c[3];
    tsx->posTY = rfree.c[1];

    cx = gl_ux[7] - rfree.c[3];
    cy = gl_ux[5] - rfree.c[1];

    gl_ux[0] -= cx;
    gl_ux[1] -= cx;
    gl_ux[2] -= cx;
    gl_ux[3] -= cx;
    gl_vy[0] -= cy;
    gl_vy[1] -= cy;
    gl_vy[2] -= cy;
    gl_vy[3] -= cy;

    XTexS = rfree.c[3];
    YTexS = rfree.c[1];

    return &tsx->Opaque;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// search cache for free place (on compress)
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOL GetCompressTexturePlace(textureSubCacheEntryS * tsx) {
    int i, j, k, iMax, iC;
    uint32_t rx, ry, mx, my;
    EXLong * ul = 0, * uls, rfree;
    unsigned char cXAdj = 1, cYAdj = 1;

    rx = (int) tsx->pos.c[2]-(int) tsx->pos.c[3];
    ry = (int) tsx->pos.c[0]-(int) tsx->pos.c[1];

    rx += 3;
    if (rx > 255) {
        cXAdj = 0;
        rx = 255;
    }
    ry += 3;
    if (ry > 255) {
        cYAdj = 0;
        ry = 255;
    }

    iC = usLRUTexPage;

    for (k = 0; k < iSortTexCnt; k++) {
        uls = pxSsubtexLeft[iC];
        iMax = uls->l;
        ul = uls + 1;

        //--------------------------------------------------//
        // first time

        if (!iMax) {
            rfree.l = 0;

            if (rx > 252 && ry > 252) {
                uls->l = 1;
                ul->l = 0xffffffff;
                ul = 0;
                goto TENDLOOP;
            }

            if (rx < 253) {
                uls->l = uls->l + 1;
                ul->c[3] = rx;
                ul->c[2] = 255 - rx;
                ul->c[1] = 0;
                ul->c[0] = ry;
                ul++;
            }

            if (ry < 253) {
                uls->l = uls->l + 1;
                ul->c[3] = 0;
                ul->c[2] = 255;
                ul->c[1] = ry;
                ul->c[0] = 255 - ry;
            }
            ul = 0;
            goto TENDLOOP;
        }

        //--------------------------------------------------//
        for (i = 0; i < iMax; i++, ul++) {
            if (ul->l != 0xffffffff &&
                    ry <= ul->c[0] &&
                    rx <= ul->c[2]) {
                rfree = *ul;
                mx = ul->c[2] - 2;
                my = ul->c[0] - 2;

                if (rx < mx && ry < my) {
                    ul->c[3] += rx;
                    ul->c[2] -= rx;
                    ul->c[0] = ry;

                    for (ul = uls + 1, j = 0; j < iMax; j++, ul++)
                        if (ul->l == 0xffffffff) break;

                    if (j < CSUBSIZE - 2) {
                        if (j == iMax) uls->l = uls->l + 1;

                        ul->c[3] = rfree.c[3];
                        ul->c[2] = rfree.c[2];
                        ul->c[1] = rfree.c[1] + ry;
                        ul->c[0] = rfree.c[0] - ry;
                    }
                } else if (rx < mx) {
                    ul->c[3] += rx;
                    ul->c[2] -= rx;
                } else if (ry < my) {
                    ul->c[1] += ry;
                    ul->c[0] -= ry;
                } else {
                    ul->l = 0xffffffff;
                }
                ul = 0;
                goto TENDLOOP;
            }
        }

        //--------------------------------------------------//

        iC++;
        if (iC >= iSortTexCnt) iC = 0;
    }

    //----------------------------------------------------//
    // check, if free space got
    //----------------------------------------------------//

TENDLOOP:
    if (ul) return FALSE;

    rfree.c[3] += cXAdj;
    rfree.c[1] += cYAdj;

    tsx->cTexID = iC;
    tsx->posTX = rfree.c[3];
    tsx->posTY = rfree.c[1];

    XTexS = rfree.c[3];
    YTexS = rfree.c[1];

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// compress texture cache (to make place for new texture part, if needed)
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CompressTextureSpace(void) {
    textureSubCacheEntryS * tsx, * tsg, * tsb;
    int i, j, k, m, n, iMax;
    EXLong * ul, r, opos;
    short sOldDST = DrawSemiTrans, cx, cy;
    int lOGTP = GlobalTexturePage;
    uint32_t l, row;
    uint32_t *lSRCPtr;
    opos.l = *((uint32_t *) & gl_ux[4]);
    //opos.l = GETLE32((uint32_t *)&gl_ux[4]);
    //dump_gl_uv();
    //printf("opos.l => %x\r\n",opos.l);
    
    // 1. mark all textures as free
    for (i = 0; i < iSortTexCnt; i++) {
        ul = pxSsubtexLeft[i];
        ul->l = 0;
    }
    usLRUTexPage = 0;

    // 2. compress
    for (j = 0; j < 3; j++) {
        for (k = 0; k < MAXTPAGES; k++) {
            tsg = pscSubtexStore[j][k];

            if ((!(dwTexPageComp & (1 << k)))) {
                (tsg + SOFFA)->pos.l = 0;
                (tsg + SOFFB)->pos.l = 0;
                (tsg + SOFFC)->pos.l = 0;
                (tsg + SOFFD)->pos.l = 0;
                continue;
            }

            for (m = 0; m < 4; m++, tsg += SOFFB) {
                iMax = tsg->pos.l;

                tsx = tsg + 1;
                for (i = 0; i < iMax; i++, tsx++) {
                    if (tsx->ClutID) {
                        r.l = tsx->pos.l;
                        for (n = i + 1, tsb = tsx + 1; n < iMax; n++, tsb++) {
                            if (tsx->ClutID == tsb->ClutID) {
                                r.c[3] = min(r.c[3], tsb->pos.c[3]);
                                r.c[2] = max(r.c[2], tsb->pos.c[2]);
                                r.c[1] = min(r.c[1], tsb->pos.c[1]);
                                r.c[0] = max(r.c[0], tsb->pos.c[0]);
                                tsb->ClutID = 0;
                            }
                        }

                        //           if(r.l!=tsx->pos.l)
                        {
                            cx = ((tsx->ClutID << 4) & 0x3F0);
                            cy = ((tsx->ClutID >> 6) & CLUTYMASK);

                            if (j != 2) {
                                // palette check sum
                                l = 0;
                                lSRCPtr = (uint32_t *) (psxVuw + cx + (cy * 1024));
                                if (j == 1)
                                    for (row = 1; row < 129; row++)
                                    {
                                        l += ((GETLE32(lSRCPtr)) - 1) * row;

                                        *lSRCPtr++;
                                    }
                                else
                                    for (row = 1; row < 9; row++)
                                    {
                                        l += ((GETLE32(lSRCPtr)) - 1) << row;

                                        *lSRCPtr++;
                                    }

                                l = ((l + HIWORD(l))&0x3fffL) << 16;
                                if (l != (tsx->ClutID & (0x00003fff << 16))) {
                                    tsx->ClutID = 0;
                                    continue;
                                }
                            }

                            tsx->pos.l = r.l;
                            if (!GetCompressTexturePlace(tsx)) // no place?
                            {
                                for (i = 0; i < 3; i++) // -> clean up everything
                                    for (j = 0; j < MAXTPAGES; j++) {
                                        tsb = pscSubtexStore[i][j];
                                        (tsb + SOFFA)->pos.l = 0;
                                        (tsb + SOFFB)->pos.l = 0;
                                        (tsb + SOFFC)->pos.l = 0;
                                        (tsb + SOFFD)->pos.l = 0;
                                    }
                                for (i = 0; i < iSortTexCnt; i++) {
                                    ul = pxSsubtexLeft[i];
                                    ul->l = 0;
                                }
                                usLRUTexPage = 0;
                                DrawSemiTrans = sOldDST;
                                GlobalTexturePage = lOGTP;
                                *((uint32_t *) & gl_ux[4]) = opos.l;
                                //PUTLE32((uint32_t *)&gl_ux[4],opos.l);
                                dump_gl_uv();
                                printf("opos.l => %x\r\n",opos.l);
                                dwTexPageComp = 0;

                                return;
                            }

                            if (tsx->ClutID & (1 << 30)) DrawSemiTrans = 1;
                            else
                                DrawSemiTrans = 0;
                            *((uint32_t *) & gl_ux[4]) = r.l;
                            //PUTLE32((uint32_t *)&gl_ux[4],r.l);
                            //dump_gl_uv();
                            printf("r.l => %x\r\n",r.l);

                            gTexName = uiStexturePage[tsx->cTexID];
                            LoadSubTexturePageSort(k, j, cx, cy);
                            uiStexturePage[tsx->cTexID] = gTexName;
                            tsx->Opaque = ubOpaqueDraw;
                        }
                    }
                }

                if (iMax) {
                    tsx = tsg + iMax;
                    while (!tsx->ClutID && iMax) {
                        tsx--;
                        iMax--;
                    }
                    tsg->pos.l = iMax;
                }

            }
        }
    }

    if (dwTexPageComp == 0xffffffff) dwTexPageComp = 0;

    *((uint32_t *) & gl_ux[4]) = opos.l;
    //PUTLE32((uint32_t *)&gl_ux[4],opos.l);
    dump_gl_uv();
    printf("opos.l => %x\r\n",opos.l);
    GlobalTexturePage = lOGTP;
    DrawSemiTrans = sOldDST;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// main entry for searching/creating textures, called from prim.c
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct XenosSurface * SelectSubTextureS(int TextureMode, uint32_t GivenClutId) {
    //printf("SelectSubTextureS(%08x,%08x);\r\n",TextureMode,GivenClutId);
    unsigned char * OPtr;
    unsigned short iCache;
    short cx, cy;

    // sort sow/tow infos for fast access

    unsigned char ma1, ma2, mi1, mi2;
    if (gl_ux[0] > gl_ux[1]) {
        mi1 = gl_ux[1];
        ma1 = gl_ux[0];
    } else {
        mi1 = gl_ux[0];
        ma1 = gl_ux[1];
    }
    if (gl_ux[2] > gl_ux[3]) {
        mi2 = gl_ux[3];
        ma2 = gl_ux[2];
    } else {
        mi2 = gl_ux[2];
        ma2 = gl_ux[3];
    }
    if (mi1 > mi2) gl_ux[7] = mi2;
    else gl_ux[7] = mi1;
    if (ma1 > ma2) gl_ux[6] = ma1;
    else gl_ux[6] = ma2;

    if (gl_vy[0] > gl_vy[1]) {
        mi1 = gl_vy[1];
        ma1 = gl_vy[0];
    } else {
        mi1 = gl_vy[0];
        ma1 = gl_vy[1];
    }
    if (gl_vy[2] > gl_vy[3]) {
        mi2 = gl_vy[3];
        ma2 = gl_vy[2];
    } else {
        mi2 = gl_vy[2];
        ma2 = gl_vy[3];
    }
    if (mi1 > mi2) gl_ux[5] = mi2;
    else gl_ux[5] = mi1;
    if (ma1 > ma2) gl_ux[4] = ma1;
    else gl_ux[4] = ma2;

    // get clut infos in one 32 bit val

    if (TextureMode == 2) // no clut here
    {
       // TR
        GivenClutId = CLUTUSED | (DrawSemiTrans << 30);
        cx = cy = 0;

        if (iFrameTexType && Fake15BitTexture())
            return gTexName;
    }
    else {
        cx = ((GivenClutId << 4) & 0x3F0); // but here
        cy = ((GivenClutId >> 6) & CLUTYMASK);
        GivenClutId = (GivenClutId & CLUTMASK) | (DrawSemiTrans << 30) | CLUTUSED;

        // palette check sum.. removed MMX asm, this easy func works as well
        {
            uint32_t l = 0, row;

            uint32_t *lSRCPtr = (uint32_t *) (psxVuw + cx + (cy * 1024));
            if (TextureMode == 1)
                for (row = 1; row < 129; row++)
                {
                    l += ((GETLE32(lSRCPtr)) - 1) * row;

                    *lSRCPtr++;
                }
            else
                for (row = 1; row < 9; row++)
                {
                    l += ((GETLE32(lSRCPtr)) - 1) << row;

                    *lSRCPtr++;
                }
            l = (l + HIWORD(l))&0x3fffL;
            GivenClutId |= (l << 16);
            //printf("SelectSubTextureS : GivenClutId=%08x\r\n",GivenClutId);
        }

    }

    // search cache
    iCache = 0;
    OPtr = CheckTextureInSubSCache(TextureMode, GivenClutId, &iCache);

    // cache full? compress and try again
    if (iCache == 0xffff) {

        CompressTextureSpace();
        OPtr = CheckTextureInSubSCache(TextureMode, GivenClutId, &iCache);
    }

    // found? fine
    usLRUTexPage = iCache;
    if (!OPtr) 
        return uiStexturePage[iCache];

    // not found? upload texture and store infos in cache
    gTexName = uiStexturePage[iCache];
    //printf("GlobalTexturePage : %d\r\n TextureMode : %d\r\n cx : %d | cy : %d\r\n",GlobalTexturePage,TextureMode,cx,cy);
    LoadSubTexturePageSort(GlobalTexturePage, TextureMode, cx, cy);
    uiStexturePage[iCache] = gTexName;
    *OPtr = ubOpaqueDraw;
    return gTexName;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#undef Xe_CreateTexture

struct XenosSurface *T_CreateTexture(struct XenosDevice *xe, unsigned int width, unsigned int height, unsigned int levels, int format, int tiled){
    printf("Xe_CreateTexture : %d | %d \r\n",width,height);
    return Xe_CreateTexture(xe,width,height,levels,format,tiled);
}
