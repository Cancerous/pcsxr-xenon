/***************************************************************************
                          draw.c  -  description
                             -------------------
    begin                : Sun Oct 28 2001
    copyright            : (C) 2001 by Pete Bernert
    email                : BlackDove@addcom.de
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

#define _IN_DRAW


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <console/console.h>

#include <ppc/timebase.h>
#include <time/time.h>

#include "draw_c_p_psu.h"
#include "draw_t_p_psu.h"
#include "draw_v_vsu.h"


#include "externals.h"
#include "gpu.h"
#include "draw.h"
#include "prim.h"
#include "menu.h"
#include "interp.h"
#include "swap.h"


// misc globals
int iResX;
int iResY;
long lLowerpart;
BOOL bIsFirstFrame = TRUE;
BOOL bCheckMask = FALSE;
unsigned short sSetMask = 0;
unsigned long lSetMask = 0;
int iDesktopCol = 16;
int iShowFPS = 0;
int iWinSize;
int iMaintainAspect = 0;
int iUseNoStretchBlt = 0;
int iFastFwd = 0;
int iDebugMode = 0;
int iFVDisplay = 0;
PSXPoint_t ptCursorPoint[8];
unsigned short usCursorActive = 0;

//unsigned int   LUT16to32[65536];
//unsigned int   RGBtoYUV[65536];
int finalw, finalh;


typedef struct DrawVerticeFormats {
    float x, y; //z/w remove it ...
    float u, v;
    unsigned int color;
} DrawVerticeFormats;
DrawVerticeFormats Rect[6];

#include <time.h>
////////////////////////////////////////////////////////////////////////

#ifndef MAX
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif

char * Xpixels;
char * pCaptionText;

unsigned char * psxScreen;
struct XenosVertexBuffer *vb = NULL;


static int fx = 0;


// close display

void DestroyDisplay(void) {
}

static int depth = 0;
int root_window_id = 0;

struct XenosDevice * g_pVideoDevice;
struct XenosShader * g_pVertexShader;
struct XenosShader * g_pPixelTexturedShader;
struct XenosSurface * g_pTexture = NULL;
struct XenosSurface * fb;
struct XenosDevice _xe;

static void ShowFPS() {
    /*
    static unsigned long lastTick = 0;
    static int frames=0,rendered_frames=0,frame_id=0;
    unsigned long nowTick;
    frame_id++;
    frames++;
    nowTick = mftb() / (PPC_TIMEBASE_FREQ / 1000);
    if (lastTick + 1000 <= nowTick) {

        printf("Real %d fps\r\n", frames);

        frames = 0;
        rendered_frames = 0;
        lastTick = nowTick;
    }
     */
}

int g_pPitch = 0;

void CreateTexture(int width, int height) {
    // Create display
    static int old_width = 0;
    static int old_height = 0;

    if ((width != old_width) || (old_height != height)) {
        printf("Old w:%d - h:%d\r\n", old_width, old_height);
        printf("New w:%d - h:%d\r\n", width, height);

        if (g_pTexture)
            Xe_DestroyTexture(g_pVideoDevice, g_pTexture);

        g_pTexture = Xe_CreateTexture(g_pVideoDevice, width, height, 1, XE_FMT_8888 | XE_FMT_ARGB, 0);
        psxScreen = (unsigned char*) Xe_Surface_LockRect(g_pVideoDevice, g_pTexture, 0, 0, 0, 0, XE_LOCK_WRITE);
        printf("g_pTexture w:%d - h:%d\r\n", g_pTexture->width, g_pTexture->height);
        g_pPitch = g_pTexture->wpitch;
        Xe_Surface_Unlock(g_pVideoDevice, g_pTexture);

        old_width = width;
        old_height = height;
    }
}

void CreateDisplay(void) {

    //xenos_init(VIDEO_MODE_AUTO);
    g_pVideoDevice = &_xe;
    Xe_Init(g_pVideoDevice);
    fb = Xe_GetFramebufferSurface(g_pVideoDevice);
    Xe_SetRenderTarget(g_pVideoDevice, fb);

    static const struct XenosVBFFormat vbf = {
        3,
        {
            {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT2},
            {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
            {XE_USAGE_COLOR, 0, XE_TYPE_UBYTE4},
        }
    };

    g_pPixelTexturedShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) draw_t_p_psu);
    Xe_InstantiateShader(g_pVideoDevice, g_pPixelTexturedShader, 0);

    g_pVertexShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) draw_v_vsu);
    Xe_InstantiateShader(g_pVideoDevice, g_pVertexShader, 0);
    Xe_ShaderApplyVFetchPatches(g_pVideoDevice, g_pVertexShader, 0, &vbf);

    CreateTexture(640, 512);

    edram_init(g_pVideoDevice);
    
    // Uv
    float bottom = 0.0f;
    float top = 1.0f;
    float left = 1.0f;
    float right = 0.0f;

    float x = -1.0f;
    float y = -1.0f;
    float w = 2.0f;
    float h = 2.0f;

    // top left
    Rect[0].x = x;
    Rect[0].y = y + h;
    Rect[0].u = bottom;
    Rect[0].v = right;
    Rect[0].color = 0;

    // bottom left
    Rect[1].x = x;
    Rect[1].y = y;
    Rect[1].u = bottom;
    Rect[1].v = left;
    Rect[1].color = 0;

    // top right
    Rect[2].x = x + w;
    Rect[2].y = y + h;
    Rect[2].u = top;
    Rect[2].v = right;
    Rect[2].color = 0xFF00FF00;

    // top right
    Rect[3].x = x + w;
    Rect[3].y = y + h;
    Rect[3].u = top;
    Rect[3].v = right;
    Rect[3].color = 0;

    // bottom left
    Rect[4].x = x;
    Rect[4].y = y;
    Rect[4].u = bottom;
    Rect[4].v = left;
    Rect[4].color = 0;

    // bottom right
    Rect[5].x = x + w;
    Rect[5].y = y;
    Rect[5].u = top;
    Rect[5].v = left;
    Rect[5].color = 0xFF00FF00;    
}

#if 0

void BlitScreen32(unsigned char *surf, int32_t x, int32_t y) {
    unsigned char *pD;
    unsigned int startxy;
    uint32_t lu;
    unsigned short s;
    unsigned short row, column;
    unsigned short dx = PreviousPSXDisplay.Range.x1;
    unsigned short dy = PreviousPSXDisplay.DisplayMode.y;

    int32_t lPitch = PSXDisplay.DisplayMode.x << 2;

    uint32_t *destpix;

    if (PreviousPSXDisplay.Range.y0) // centering needed?
    {
        memset(surf, 0, (PreviousPSXDisplay.Range.y0 >> 1) * lPitch);

        dy -= PreviousPSXDisplay.Range.y0;
        surf += (PreviousPSXDisplay.Range.y0 >> 1) * lPitch;

        memset(surf + dy * lPitch,
                0, ((PreviousPSXDisplay.Range.y0 + 1) >> 1) * lPitch);
    }

    if (PreviousPSXDisplay.Range.x0) {
        for (column = 0; column < dy; column++) {
            destpix = (uint32_t *) (surf + (column * lPitch));
            memset(destpix, 0, PreviousPSXDisplay.Range.x0 << 2);
        }
        surf += PreviousPSXDisplay.Range.x0 << 2;
    }

    if (PSXDisplay.RGB24) {
        for (column = 0; column < dy; column++) {
            startxy = ((1024) * (column + y)) + x;
            pD = (unsigned char *) &psxVuw[startxy];
            destpix = (uint32_t *) (surf + (column * lPitch));
            for (row = 0; row < dx; row++) {
                lu = *((uint32_t *) pD);
                destpix[row] =
                        0xff000000 | (RED(lu) << 16) | (GREEN(lu) << 8) | (BLUE(lu));
                pD += 3;
            }
        }
    } else {
        for (column = 0; column < dy; column++) {
            startxy = (1024 * (column + y)) + x;
            destpix = (uint32_t *) (surf + (column * lPitch));
            for (row = 0; row < dx; row++) {
                s = GETLE16(&psxVuw[startxy++]);
                destpix[row] =
                        (((s << 19) & 0xf80000) | ((s << 6) & 0xf800) | ((s >> 7) & 0xf8)) | 0xff000000;
            }
        }
    }
}
#else

void BlitScreen32(unsigned char *surf, int32_t x, int32_t y) {
    unsigned char *pD;
    unsigned int startxy;
    uint32_t lu;
    uint32_t lu0,lu1,lu2,lu3,lu4,lu5,lu6,lu7;
    unsigned short s0, s1, s2, s3, s4, s5, s6, s7;
    uint32_t d0, d1, d2, d3, d4, d5, d6, d7;
    uint32_t * __restrict rdest;
    unsigned short row, column;
    unsigned short dx = PreviousPSXDisplay.Range.x1;
    unsigned short dy = PreviousPSXDisplay.DisplayMode.y;

    //int32_t lPitch = PSXDisplay.DisplayMode.x << 2;

    int32_t lPitch = g_pPitch;

    uint32_t *destpix;

    if (PreviousPSXDisplay.Range.y0) // centering needed?
    {
        memset(surf, 0, (PreviousPSXDisplay.Range.y0 >> 1) * lPitch);

        dy -= PreviousPSXDisplay.Range.y0;
        surf += (PreviousPSXDisplay.Range.y0 >> 1) * lPitch;

        memset(surf + dy * lPitch,
                0, ((PreviousPSXDisplay.Range.y0 + 1) >> 1) * lPitch);
    }

    if (PreviousPSXDisplay.Range.x0) {
        for (column = 0; column < dy; column++) {
            destpix = (uint32_t *) (surf + (column * lPitch));
            memset(destpix, 0, PreviousPSXDisplay.Range.x0 << 2);
        }
        surf += PreviousPSXDisplay.Range.x0 << 2;
    }

    if (PSXDisplay.RGB24) {
        for (column = 0; column < dy; column++) {
            startxy = ((1024) * (column + y)) + x;
            pD = (unsigned char *) &psxVuw[startxy];
            destpix = (uint32_t *) (surf + (column * lPitch));
            for (row = 0; row < dx; row++) {
                
                lu = *((uint32_t *) pD);
                destpix[row] =
                       0xff000000 | (RED(lu) << 16) | (GREEN(lu) << 8) | (BLUE(lu));
                pD += 3;

            }
        }
    } else {
        for (column = 0; column < dy; column++) {
            startxy = (1024 * (column + y)) + x;
            destpix = (uint32_t *) (surf + (column * lPitch));

            for (row = 0; row < dx; row += 8) {
                rdest = &destpix[row];

                s0 = GETLE16(&psxVuw[startxy++]);
                s1 = GETLE16(&psxVuw[startxy++]);
                s2 = GETLE16(&psxVuw[startxy++]);
                s3 = GETLE16(&psxVuw[startxy++]);
                s4 = GETLE16(&psxVuw[startxy++]);
                s5 = GETLE16(&psxVuw[startxy++]);
                s6 = GETLE16(&psxVuw[startxy++]);
                s7 = GETLE16(&psxVuw[startxy++]);

                d0 = (((s0 << 19) & 0xf80000) | ((s0 << 6) & 0xf800) | ((s0 >> 7) & 0xf8)) | 0xff000000;
                d1 = (((s1 << 19) & 0xf80000) | ((s1 << 6) & 0xf800) | ((s1 >> 7) & 0xf8)) | 0xff000000;
                d2 = (((s2 << 19) & 0xf80000) | ((s2 << 6) & 0xf800) | ((s2 >> 7) & 0xf8)) | 0xff000000;
                d3 = (((s3 << 19) & 0xf80000) | ((s3 << 6) & 0xf800) | ((s3 >> 7) & 0xf8)) | 0xff000000;
                d4 = (((s4 << 19) & 0xf80000) | ((s4 << 6) & 0xf800) | ((s4 >> 7) & 0xf8)) | 0xff000000;
                d5 = (((s5 << 19) & 0xf80000) | ((s5 << 6) & 0xf800) | ((s5 >> 7) & 0xf8)) | 0xff000000;
                d6 = (((s6 << 19) & 0xf80000) | ((s6 << 6) & 0xf800) | ((s6 >> 7) & 0xf8)) | 0xff000000;
                d7 = (((s7 << 19) & 0xf80000) | ((s7 << 6) & 0xf800) | ((s7 >> 7) & 0xf8)) | 0xff000000;

                rdest[0] = d0;
                rdest[1] = d1;
                rdest[2] = d2;
                rdest[3] = d3;
                rdest[4] = d4;
                rdest[5] = d5;
                rdest[6] = d6;
                rdest[7] = d7;
            }
        }
    }
}
#endif



void DoBufferSwap(void) {
    //printf("DoBufferSwap\r\n");
    finalw = PSXDisplay.DisplayMode.x;
    finalh = PSXDisplay.DisplayMode.y;

    if (finalw == 0 || finalh == 0)
        return;

    // Show fps
    ShowFPS();

    // Update texture size
    CreateTexture(finalw, finalh);
    BlitScreen32((unsigned char *) psxScreen, PSXDisplay.DisplayPosition.x, PSXDisplay.DisplayPosition.y);

    // refresh texture cache
    Xe_Surface_LockRect(g_pVideoDevice, g_pTexture, 0, 0, 0, 0, XE_LOCK_WRITE);
    Xe_Surface_Unlock(g_pVideoDevice, g_pTexture);

    Xe_InvalidateState(g_pVideoDevice);
    Xe_SetClearColor(g_pVideoDevice, 0);

    Xe_SetBlendOp(g_pVideoDevice, XE_BLENDOP_ADD);
    Xe_SetSrcBlend(g_pVideoDevice, XE_BLEND_SRCALPHA);
    Xe_SetDestBlend(g_pVideoDevice, XE_BLEND_INVSRCALPHA);

    int len = 6 * sizeof (DrawVerticeFormats);

    Xe_VBBegin(g_pVideoDevice, sizeof (DrawVerticeFormats));
    Xe_VBPut(g_pVideoDevice, Rect, len);

    vb = Xe_VBEnd(g_pVideoDevice);
    Xe_VBPoolAdd(g_pVideoDevice, vb);    
    
    Xe_SetCullMode(g_pVideoDevice, XE_CULL_NONE);
    Xe_SetStreamSource(g_pVideoDevice, 0, vb, 0, sizeof (DrawVerticeFormats));
    Xe_SetTexture(g_pVideoDevice, 0, g_pTexture);
    Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixelTexturedShader, 0);
    Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);
    Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_TRIANGLELIST, 0, 2);

    Xe_Resolve(g_pVideoDevice);
    Xe_Sync(g_pVideoDevice);
}

void DoClearScreenBuffer(void) // CLEAR DX BUFFER
{

}

void DoClearFrontBuffer(void) // CLEAR DX BUFFER
{
}

int Xinitialize() {
    iDesktopCol = 32;

    bUsingTWin = FALSE;

    InitMenu();

    bIsFirstFrame = FALSE; // done

    return 0;
}

void Xcleanup() // X CLEANUP
{
    CloseMenu();

}

unsigned long ulInitDisplay(void) {
    CreateDisplay(); // x stuff
    Xinitialize(); // init x

    printf("ulInitDisplay !!\r\n");
    return 100;
}

void CloseDisplay(void) {
    Xcleanup(); // cleanup dx
    DestroyDisplay();
}

void CreatePic(unsigned char * pMem) {

}

void DestroyPic(void) {

}

void DisplayPic(void) {

}

void ShowGpuPic(void) {
}

void ShowTextGpuPic(void) {
}
