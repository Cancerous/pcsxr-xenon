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


#define _IN_TEXTURE

#include "externals.h"
using namespace xegpu;
#include "texture.h"
#include "gpu.h"
#include "prim.h"
#include "GpuRenderer.h"
#include "texture_movie.h"
#include "texture_load.h"


#define CLUTCHK   0x00060000
#define CLUTSHIFT 17


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


#define XCHECK(pos1,pos2) ((pos1._0>=pos2._1)&&(pos1._1<=pos2._0)&&(pos1._2>=pos2._3)&&(pos1._3<=pos2._2))
#define INCHECK(pos2,pos1) ((pos1._0<=pos2._0) && (pos1._1>=pos2._1) && (pos1._2<=pos2._2) && (pos1._3>=pos2._3))

namespace xegpu{
////////////////////////////////////////////////////////////////////////
// texture conversion buffer ..
////////////////////////////////////////////////////////////////////////

int iHiResTextures = 0;
GLubyte ubPaletteBuffer[256][4];
GpuTex * gTexMovieName = 0;
GpuTex * gTexBlurName = 0;
GpuTex * gTexFrameName = 0;
int iTexGarbageCollection = 1;
uint32_t dwTexPageComp = 0;
int iVRamSize = 0;
int iClampType = XE_TEXADDR_CLAMP;

void (*LoadSubTexFn) (int, int, short, short);
uint32_t(*PalTexturedColourFn) (uint32_t);


////////////////////////////////////////////////////////////////////////
// some globals
////////////////////////////////////////////////////////////////////////

GLint giWantedRGBA = 4;
GLint giWantedFMT = XE_FMT_ARGB;
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

textureWndCacheEntry wcWndtexStore[MAXWNDTEXCACHE];
textureSubCacheEntryS * pscSubtexStore[3][MAXTPAGES_MAX];
EXLong * pxSsubtexLeft [MAXSORTTEX_MAX];
GpuTex * uiStexturePage[MAXSORTTEX_MAX];

unsigned short usLRUTexPage = 0;

int iMaxTexWnds = 0;
int iTexWndTurn = 0;
int iTexWndLimit = MAXWNDTEXCACHE / 2;

GLubyte * texturepart = NULL;
GLubyte * texturebuffer = NULL;
uint32_t g_x1, g_y1, g_x2, g_y2;
unsigned char ubOpaqueDraw = 0;

unsigned short MAXTPAGES = 32;
unsigned short CLUTMASK = 0x7fff;
unsigned short CLUTYMASK = 0x1ff;
unsigned short MAXSORTTEX = 196;
}
#define printf(...)

////////////////////////////////////////////////////////////////////////
// Texture color conversions... all my ASM funcs are removed for easier
// porting... and honestly: nowadays the speed gain would be pointless
////////////////////////////////////////////////////////////////////////

uint32_t XP8RGBA(uint32_t BGR) {
    if (!(BGR & 0xffff)) return 0x50000000;
    if (DrawSemiTrans && !(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff);
    }
    return ((((BGR << 3)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 9)&0xf80000))&0xffffff) | 0xff000000;
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
    if (!(BGR & 0xffff)) return 0x50000000;
    return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff) | 0xff000000;
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
    if (!(BGR & 0xffff)) return 0x50000000;
    if (!(BGR & 0x8000)) {
        ubOpaqueDraw = 1;
        return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff);
    }
    return ((((BGR >> 7)&0xf8) | ((BGR << 6)&0xf800) | ((BGR << 19)&0xf80000))&0xffffff) | 0xff000000;
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


void _DumpExL(EXLongTag * l, const char * val_str){
    if(l!=NULL){
        printf("%s\r\n",val_str);
        printf("l = %08x\r\n",l->l);
        /*
        printf("0 = %x\r\n",l->c[0]);
        printf("1 = %x\r\n",l->c[1]);
        printf("2 = %x\r\n",l->c[2]);
        printf("3 = %x\r\n",l->c[3]);
        */
        printf("0 = %02x\r\n",l->_0);
        printf("1 = %02x\r\n",l->_1);
        printf("2 = %02x\r\n",l->_2);
        printf("3 = %02x\r\n",l->_3);
        printf("\r\n");
    }
}

#define DumpExL(x) {TR;_DumpExL(x, #x);}
//#define DumpExL(x) {}

//#define te_swap(x) bswap_32(x);
#define te_swap(x) x

////////////////////////////////////////////////////////////////////////
// CHECK TEXTURE MEM (on plugin startup)
////////////////////////////////////////////////////////////////////////

int iFTexA = 512;
int iFTexB = 512;

void CheckTextureMemory(void) {
#if 1
    GLboolean b;
    GLboolean * bDetail;
    int i, iCnt, iRam = iVRamSize * 1024 * 1024;
    int iTSize;
    char * p;

    if (iBlurBuffer) {
        char * p;

        if (iResX > 1024) iFTexA = 2048;
        else
            if (iResX > 512) iFTexA = 1024;
        else iFTexA = 512;
        if (iResY > 1024) iFTexB = 2048;
        else
            if (iResY > 512) iFTexB = 1024;
        else iFTexB = 512;

        p = (char *) malloc(iFTexA * iFTexB * 4);
        memset(p, 0, iFTexA * iFTexB * 4);

        free(p);
        glGetError();
        iRam -= iFTexA * iFTexB * 3;
        iFTexA = (iResX * 256) / iFTexA;
        iFTexB = (iResY * 256) / iFTexB;
    }

    if (iVRamSize) {
        int ts;

        iRam -= (iResX * iResY * 8);
        iRam -= (iResX * iResY * (iZBufferDepth / 8));

        if (iTexQuality == 0 || iTexQuality == 3) ts = 4;
        else ts = 2;

        if (iHiResTextures)
            iSortTexCnt = iRam / (512 * 512 * ts);
        else iSortTexCnt = iRam / (256 * 256 * ts);

        if (iSortTexCnt > MAXSORTTEX) {
            iSortTexCnt = MAXSORTTEX - min(1, iHiResTextures);
        } else {
            iSortTexCnt -= 3 + min(1, iHiResTextures);
            if (iSortTexCnt < 8) iSortTexCnt = 8;
        }

        for (i = 0; i < MAXSORTTEX; i++)
            uiStexturePage[i] = 0;

        return;
    }


    if (iHiResTextures) iTSize = 512;
    else iTSize = 256;
    p = (char *) malloc(iTSize * iTSize * 4);

    iCnt = 0;

    free(p);

    bDetail = (GLboolean*)malloc(MAXSORTTEX * sizeof (GLboolean));
    memset(bDetail, 0, MAXSORTTEX * sizeof (GLboolean));

    for (i = 0; i < MAXSORTTEX; i++) {
        if (bDetail[i]) iCnt++;
        uiStexturePage[i] = 0;
    }

    free(bDetail);

    if (b) iSortTexCnt = MAXSORTTEX - min(1, iHiResTextures);
    else iSortTexCnt = iCnt - 3 + min(1, iHiResTextures); // place for menu&texwnd

    if (iSortTexCnt < 8) iSortTexCnt = 8;
    
#else
    int i = 0;
    for (i = 0; i < MAXSORTTEX; i++) {
        uiStexturePage[i] = 0;
    }
    
    iSortTexCnt = 8;
#endif
    
    
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
    } else {
        MAXTPAGES = 32;
        CLUTMASK = 0x7fff;
        CLUTYMASK = 0x1ff;
        MAXSORTTEX = 196;
    }

    memset(vertex, 0, 4 * sizeof (OGLVertex)); // init vertices

    gTexName = 0; // init main tex name

    iTexWndLimit = MAXWNDTEXCACHE;
    if (!iUsePalTextures) iTexWndLimit /= 2;

    memset(wcWndtexStore, 0, sizeof (textureWndCacheEntry) *
            MAXWNDTEXCACHE);
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
        uiStexturePage[i] = 0;
    }
}

////////////////////////////////////////////////////////////////////////
// Clean up on exit
////////////////////////////////////////////////////////////////////////
void CleanupTextureStore() {
    int i, j;
    textureWndCacheEntry * tsx;
    //----------------------------------------------------//
    /*
     glBindTexture(GL_TEXTURE_2D,0);
     */
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
            gpuRenderer.DestroyTexture(tsx->texname); // --> delete it
    }
    iMaxTexWnds = 0; // no more tex wnds
    //----------------------------------------------------//
    if (gTexMovieName != 0) // some movie tex?
        gpuRenderer.DestroyTexture( gTexMovieName); // -> delete it
    gTexMovieName = 0; // no more movie tex
    //----------------------------------------------------//
    if (gTexFrameName != 0) // some 15bit framebuffer tex?
        gpuRenderer.DestroyTexture( gTexFrameName); // -> delete it
    gTexFrameName = 0; // no more movie tex
    //----------------------------------------------------//
    if (gTexBlurName != 0) // some 15bit framebuffer tex?
        gpuRenderer.DestroyTexture( gTexBlurName); // -> delete it
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
            gpuRenderer.DestroyTexture( uiStexturePage[i]);
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
        //glBindTexture(GL_TEXTURE_2D,0);gTexName=0;
        gpuRenderer.DisableTexture();
        gTexName = 0;

    }
    //----------------------------------------------------//
    tsx = wcWndtexStore;
    for (i = 0; i < MAXWNDTEXCACHE; i++, tsx++) {
        tsx->used = 0;
        if (bDelTex && tsx->texname) {
            gpuRenderer.DestroyTexture( tsx->texname);
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
//        DumpExL(lu);
        if (bDelTex && uiStexturePage[i]) {
            gpuRenderer.DestroyTexture( uiStexturePage[i]);
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
                
                //DumpExL(&tsw->pos);
                
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
                
                //DumpExL(&tsw->pos);
                
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
    DumpExL(&tsx->pos);
    printf("iMax = %08x\r\n",iMax);

    for (j = 0; j < iMax; j++, ul++)
        if (ul->l == 0xffffffff) break;

    if (j < CSUBSIZE - 2) {
        if (j == iMax) 
            uls->l = uls->l + 1;

        DumpExL(uls);
        
        x1 = tsx->posTX;
        dx = tsx->pos._2 - tsx->pos._3;
        if (tsx->posTX) {
            x1--;
            dx += 3;
        }
        y1 = tsx->posTY;
        dy = tsx->pos._0 - tsx->pos._1;
        if (tsx->posTY) {
            y1--;
            dy += 3;
        }

        ul->_3 = x1;
        ul->_2 = dx;
        ul->_1 = y1;
        ul->_0 = dy;
    }
}

//#define DUMP_ISTA(){ \
///*printf("InvalidateSubSTextureArea %d %d %d %d\r\n",ISTA_X,ISTA_Y,ISTA_W,ISTA_H);*/ \
//printf("iMax:%08x\r\n",iMax); \
//printf("tsb->ClutID : %08x\r\n",tsb->ClutID);\
//DumpExL(&tsb->pos);\
//DumpExL(&npos);\
//}

#define DUMP_ISTA(){ }

// clutid 0xa6706abe
void InvalidateSubSTextureArea(int X, int Y, int W, int H) {
    int ISTA_X = X;
    int ISTA_Y = Y;
    int ISTA_W = W;
    int ISTA_H = H;
    // printf("InvalidateSubSTextureArea %d %d %d %d\r\n",X,Y,W,H);

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


                //npos.l=bswap_32(npos.l);
                {
                    tsb = pscSubtexStore[k][j] + SOFFA;

                    iMax = tsb->pos.l;
                    //DumpExL(&tsb->pos);
                    tsb++;
                    for (i = 0; i < iMax; i++, tsb++)

                        if (tsb->ClutID && XCHECK(tsb->pos, npos)) {

                            DUMP_ISTA()

                            tsb->ClutID = 0;
                            MarkFree(tsb);
                        }

                    //         if(npos.l & 0x00800000)
                    {
                        tsb = pscSubtexStore[k][j] + SOFFB;

                        iMax = tsb->pos.l;
                        //DumpExL(&tsb->pos);
                        tsb++;
                        for (i = 0; i < iMax; i++, tsb++)

                            if (tsb->ClutID && XCHECK(tsb->pos, npos)) {
                                DUMP_ISTA()
                                tsb->ClutID = 0;
                                MarkFree(tsb);
                            }
                    }

                    //         if(npos.l & 0x00000080)
                    {
                        tsb = pscSubtexStore[k][j] + SOFFC;

                        iMax = tsb->pos.l;
                        //DumpExL(&tsb->pos);
                        tsb++;
                        for (i = 0; i < iMax; i++, tsb++)

                            if (tsb->ClutID && XCHECK(tsb->pos, npos)) {
                                DUMP_ISTA()
                                tsb->ClutID = 0;
                                MarkFree(tsb);
                            }
                    }

                    //         if(npos.l & 0x00800080)
                    {
                        tsb = pscSubtexStore[k][j] + SOFFD;

                        iMax = tsb->pos.l;
                        //DumpExL(&tsb->pos);
                        tsb++;
                        for (i = 0; i < iMax; i++, tsb++)

                            if (tsb->ClutID && XCHECK(tsb->pos, npos)) {
                                DUMP_ISTA()
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

/////////////////////////////////////////////////////////////////////////////
// texture cache garbage collection
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
// search cache for existing (already used) parts
/////////////////////////////////////////////////////////////////////////////
unsigned char * CheckTextureInSubSCache(int TextureMode, uint32_t GivenClutId, unsigned short * pCache) {
    textureSubCacheEntryS * tsx, * tsb, *tsg; //, *tse=NULL;
    int i, iMax;
    EXLong npos,tpos;
    unsigned char cx, cy;
    int iC, j, k;
    uint32_t rx, ry, mx, my;
    EXLong * ul = 0, * uls;
    EXLong rfree;
    unsigned char cXAdj, cYAdj;

    npos.l = GETLE32((uint32_t *) & gl_ux[4]);
        
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
                    cx = tsb->pos._3 - tsb->posTX;
                    cy = tsb->pos._1 - tsb->posTY;

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
                        rfree._3 = min(rfree._3, tsb->pos._3);
                        rfree._2 = max(rfree._2, tsb->pos._2);
                        rfree._1 = min(rfree._1, tsb->pos._1);
                        rfree._0 = max(rfree._0, tsb->pos._0);
                        MarkFree(tsb);
                    }

                if (tsx) // 3. if one or more found, create a new rect with bigger size
                {
                    // *((uint32_t *) & gl_ux[4]) = npos.l = rfree.l;
                    npos.l = rfree.l;
                    PUTLE32(((uint32_t *) & gl_ux[4]), rfree.l);

                    rx = (int) rfree._2 - (int) rfree._3;
                    ry = (int) rfree._0 - (int) rfree._1;
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
                ul->_3 = rx;
                ul->_2 = 255 - rx;
                ul->_1 = 0;
                ul->_0 = ry;
                ul++;
            }

            if (ry < 253) {
                uls->l = uls->l + 1;
                ul->_3 = 0;
                ul->_2 = 255;
                ul->_1 = ry;
                ul->_0 = 255 - ry;
            }
            ul = 0;
            goto ENDLOOP;
        }

        //--------------------------------------------------//
        for (i = 0; i < iMax; i++, ul++) {
            if (ul->l != 0xffffffff &&
                    ry <= ul->_0 &&
                    rx <= ul->_2) {
                rfree = *ul;
                mx = ul->_2 - 2;
                my = ul->_0 - 2;
                if (rx < mx && ry < my) {
                    ul->_3 += rx;
                    ul->_2 -= rx;
                    ul->_0 = ry;

                    for (ul = uls + 1, j = 0; j < iMax; j++, ul++)
                        if (ul->l == 0xffffffff) break;

                    if (j < CSUBSIZE - 2) {
                        if (j == iMax) uls->l = uls->l + 1;

                        ul->_3 = rfree._3;
                        ul->_2 = rfree._2;
                        ul->_1 = rfree._1 + ry;
                        ul->_0 = rfree._0 - ry;
                    }
                } else if (rx < mx) {
                    ul->_3 += rx;
                    ul->_2 -= rx;
                } else if (ry < my) {
                    ul->_1 += ry;
                    ul->_0 -= ry;
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
                ul->_3 = rx;
                ul->_2 = 255 - rx;
                ul->_1 = 0;
                ul->_0 = ry;
                ul++;
            }
            if (ry < 253) {
                uls->l = uls->l + 1;
                ul->_3 = 0;
                ul->_2 = 255;
                ul->_1 = ry;
                ul->_0 = 255 - ry;
            }
        }
        tsg->pos.l = 1;


        tsx = tsg + 1;
    }

    rfree._3 += cXAdj;
    rfree._1 += cYAdj;

    tsx->cTexID = *pCache = iC;
    tsx->pos = npos;

    tsx->ClutID = GivenClutId;
    tsx->posTX = rfree._3;
    tsx->posTY = rfree._1;

    cx = gl_ux[7] - rfree._3;
    cy = gl_ux[5] - rfree._1;

    gl_ux[0] -= cx;
    gl_ux[1] -= cx;
    gl_ux[2] -= cx;
    gl_ux[3] -= cx;
    gl_vy[0] -= cy;
    gl_vy[1] -= cy;
    gl_vy[2] -= cy;
    gl_vy[3] -= cy;

    XTexS = rfree._3;
    YTexS = rfree._1;

    return &tsx->Opaque;
}


/////////////////////////////////////////////////////////////////////////////
// search cache for free place (on compress)
/////////////////////////////////////////////////////////////////////////////

BOOL GetCompressTexturePlace(textureSubCacheEntryS * tsx) {

    TR;
    //    printf("tsx->cTexID = %d\r\n",tsx->cTexID);
    //    printf("XTexS = %d\r\n",XTexS);
    //    printf("YTexS = %d\r\n",YTexS);

    int i, j, k, iMax, iC;
    uint32_t rx, ry, mx, my;
    EXLong * ul = 0, * uls, rfree;
    unsigned char cXAdj = 1, cYAdj = 1;

    rx = (int) tsx->pos._2 - (int) tsx->pos._3;
    ry = (int) tsx->pos._0 - (int) tsx->pos._1;

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
                ul->_3 = rx;
                ul->_2 = 255 - rx;
                ul->_1 = 0;
                ul->_0 = ry;
                ul++;
            }

            if (ry < 253) {
                uls->l = uls->l + 1;
                ul->_3 = 0;
                ul->_2 = 255;
                ul->_1 = ry;
                ul->_0 = 255 - ry;
            }
            ul = 0;
            goto TENDLOOP;
        }

        //--------------------------------------------------//
        for (i = 0; i < iMax; i++, ul++) {
            if (ul->l != 0xffffffff &&
                    ry <= ul->_0 &&
                    rx <= ul->_2) {
                rfree = *ul;
                mx = ul->_2 - 2;
                my = ul->_0 - 2;

                if (rx < mx && ry < my) {
                    ul->_3 += rx;
                    ul->_2 -= rx;
                    ul->_0 = ry;

                    for (ul = uls + 1, j = 0; j < iMax; j++, ul++)
                        if (ul->l == 0xffffffff) break;

                    if (j < CSUBSIZE - 2) {
                        if (j == iMax) uls->l = uls->l + 1;

                        ul->_3 = rfree._3;
                        ul->_2 = rfree._2;
                        ul->_1 = rfree._1 + ry;
                        ul->_0 = rfree._0 - ry;
                    }
                } else if (rx < mx) {
                    ul->_3 += rx;
                    ul->_2 -= rx;
                } else if (ry < my) {
                    ul->_1 += ry;
                    ul->_0 -= ry;
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

    rfree._3 += cXAdj;
    rfree._1 += cYAdj;

    tsx->cTexID = iC;
    tsx->posTX = rfree._3;
    tsx->posTY = rfree._1;

    XTexS = rfree._3;
    YTexS = rfree._1;

    //    printf("tsx->cTexID = %d\r\n",tsx->cTexID);
    //    
    //    printf("XTexS = %d\r\n",XTexS);
    //    printf("YTexS = %d\r\n",YTexS);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// compress texture cache (to make place for new texture part, if needed)
/////////////////////////////////////////////////////////////////////////////

void CompressTextureSpace(void) {
    textureSubCacheEntryS * tsx, * tsg, * tsb;
    int i, j, k, m, n, iMax;
    EXLong * ul, r, opos;
    short sOldDST = DrawSemiTrans, cx, cy;
    int lOGTP = GlobalTexturePage;
    uint32_t l, row;
    uint32_t *lSRCPtr;

    opos.l = GETLE32((uint32_t *) & gl_ux[4]);

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
                                r._3 = min(r._3, tsb->pos._3);
                                r._2 = max(r._2, tsb->pos._2);
                                r._1 = min(r._1, tsb->pos._1);
                                r._0 = max(r._0, tsb->pos._0);
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
                                if (j == 1) for (row = 1; row < 129; row++) l += (GETLE32(lSRCPtr++) - 1) * row;
                                else for (row = 1; row < 9; row++) l += (GETLE32(lSRCPtr++) - 1) << row;
                                l = ((l + HIWORD(l))&0x3fffL) << 16;
                                if (l != (tsx->ClutID & (0x00003fff << 16))) {
                                    tsx->ClutID = 0;
                                    continue;
                                }
                            }

                            tsx->pos.l = r.l;
                            DumpExL(&tsx->pos);
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
                                //                                DumpExL(ul);

                                usLRUTexPage = 0;
                                DrawSemiTrans = sOldDST;
                                GlobalTexturePage = lOGTP;

                                PUTLE32(((uint32_t *) & gl_ux[4]), opos.l);

                                dwTexPageComp = 0;

                                return;
                            }

                            if (tsx->ClutID & (1 << 30)) DrawSemiTrans = 1;
                            else DrawSemiTrans = 0;

                            PUTLE32(((uint32_t *) & gl_ux[4]), r.l);

                            gTexName = uiStexturePage[tsx->cTexID];
                            LoadSubTexFn(k, j, cx, cy);
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
                    DumpExL(&tsg->pos);
                }

            }
        }
    }

    if (dwTexPageComp == 0xffffffff) dwTexPageComp = 0;

    PUTLE32(((uint32_t *) & gl_ux[4]), opos.l);

    GlobalTexturePage = lOGTP;
    DrawSemiTrans = sOldDST;
}

