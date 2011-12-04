/***************************************************************************
                          texture.h  -  description
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

#ifndef _GPU_TEXTURE_H_
#define _GPU_TEXTURE_H_

#define TEXTUREPAGESIZE 256 * 256
#define MAXTPAGES_MAX  64
#define MAXSORTTEX_MAX 196
#define MAXWNDTEXCACHE 128

// "texture window" cache entry
typedef struct textureWndCacheEntryTag {
    uint32_t ClutID;
    short pageid;
    short textureMode;
    short Opaque;
    short used;
    EXLong pos;
    GpuTex * texname;
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

void           InitializeTextureStore();
void           CleanupTextureStore();
GpuTex *         LoadTextureWnd(int pageid, int TextureMode, uint32_t GivenClutId);
GpuTex *         LoadTextureMovie(void);
void           InvalidateTextureArea(int imageX0, int imageY0, int imageX1, int imageY1);
void           InvalidateTextureAreaEx(void);
void           LoadTexturePage(int pageid, int mode, short cx, short cy);
void           ResetTextureArea(BOOL bDelTex);
GpuTex *         SelectSubTextureS(int TextureMode, uint32_t GivenClutId);
void           CheckTextureMemory(void);

void           LoadSubTexturePage(int pageid, int mode, short cx, short cy);
void           LoadSubTexturePageSort(int pageid, int mode, short cx, short cy);
void           LoadPackedSubTexturePage(int pageid, int mode, short cx, short cy);
void           LoadPackedSubTexturePageSort(int pageid, int mode, short cx, short cy);
uint32_t       XP8RGBA(uint32_t BGR);
uint32_t       XP8RGBAEx(uint32_t BGR);
uint32_t       XP8RGBA_0(uint32_t BGR);
uint32_t       XP8RGBAEx_0(uint32_t BGR);
uint32_t       XP8BGRA_0(uint32_t BGR);
uint32_t       XP8BGRAEx_0(uint32_t BGR);
uint32_t       XP8RGBA_1(uint32_t BGR);
uint32_t       XP8RGBAEx_1(uint32_t BGR);
uint32_t       XP8BGRA_1(uint32_t BGR);
uint32_t       XP8BGRAEx_1(uint32_t BGR);
uint32_t       P8RGBA(uint32_t BGR);
uint32_t       P8BGRA(uint32_t BGR);
uint32_t       CP8RGBA_0(uint32_t BGR);
uint32_t       CP8RGBAEx_0(uint32_t BGR);
uint32_t       CP8BGRA_0(uint32_t BGR);
uint32_t       CP8BGRAEx_0(uint32_t BGR);
uint32_t       CP8RGBA(uint32_t BGR);
uint32_t       CP8RGBAEx(uint32_t BGR);
unsigned short XP5RGBA (unsigned short BGR);
unsigned short XP5RGBA_0 (unsigned short BGR);
unsigned short XP5RGBA_1 (unsigned short BGR);
unsigned short P5RGBA (unsigned short BGR);
unsigned short CP5RGBA_0 (unsigned short BGR);
unsigned short XP4RGBA (unsigned short BGR);
unsigned short XP4RGBA_0 (unsigned short BGR);
unsigned short XP4RGBA_1 (unsigned short BGR);
unsigned short P4RGBA (unsigned short BGR);
unsigned short CP4RGBA_0 (unsigned short BGR);

static inline int glGetError() {
    return 0;
}

// for faster BSWAP test
static __inline uint32_t ptr32(void * addr){
    return GETLE32((uint32_t *)addr);
}


namespace xegpu{
    
    extern uint32_t(*TCF[2]) (uint32_t);
    extern void (*LoadSubTexFn) (int, int, short, short);

    extern GpuTex * gTexFrameName;

    extern GLubyte * texturepart;
    extern GLubyte * texturebuffer;
    extern int iHiResTextures;

    extern GLubyte ubPaletteBuffer[256][4];
    extern unsigned char ubOpaqueDraw;
    extern uint32_t g_x1, g_y1, g_x2, g_y2;

    extern unsigned short MAXTPAGES;
    extern unsigned short CLUTMASK;
    extern unsigned short CLUTYMASK;
    extern unsigned short MAXSORTTEX;

    extern int iMaxTexWnds;
    extern int iTexWndTurn;
    extern int iTexWndLimit;

    extern GLint XTexS;
    extern GLint YTexS;
    extern GLint DXTexS;
    extern GLint DYTexS;

    extern int iFrameTexType;
    extern int iFrameReadType;

    extern int GlobalTexturePage;

    extern GpuTex * uiStexturePage[MAXSORTTEX_MAX];
    extern textureWndCacheEntry wcWndtexStore[MAXWNDTEXCACHE];
    extern unsigned short usLRUTexPage;

    extern int iClampType;
    
    extern GpuTex * gTexMovieName;
    extern int iClampType;
    extern unsigned char ubOpaqueDraw;
    extern GLubyte * texturepart;
    extern BOOL bUseFastMdec;
}
#endif // _TEXTURE_H_
