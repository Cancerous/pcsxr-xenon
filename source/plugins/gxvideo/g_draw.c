/*
 * Copyright (C) 2010 Benoit Gschwind
 * Inspired by original author : Pete Bernert
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#include <math.h>

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
#include <time.h>

#include "globals.h"
#include "swap.h"


#include "draw_c_p_psu.h"
#include "draw_t_p_psu.h"
#include "draw_v_vsu.h"
//#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}
#define TR

struct XenosVertexBuffer *vb = NULL;
struct XenosDevice * g_pVideoDevice = NULL;
struct XenosShader * g_pVertexShader = NULL;
struct XenosShader * g_pPixelTexturedShader = NULL;
struct XenosSurface * g_pTexture = NULL;
struct XenosSurface * fb = NULL;
struct XenosDevice _xe;

typedef struct DrawVerticeFormats {
    float x, y; //z/w remove it ...
    float u, v;
    unsigned int color;
} DrawVerticeFormats;

DrawVerticeFormats Rect[6];

int g_pPitch = 0;
float texturesize[2];

enum {
    UvBottom = 0,
    UvTop,
    UvLeft,
    UvRight
};

float PsxScreenUv[4] = {0.f, 1.0f, 1.0f, 0.f};

float psxRealW = 1024.f;
float psxRealH = 512.f;

void DestroyDisplay(void) {

    if (g_draw.display) {

    }
}

void CreateTexture(int width, int height) {

    // Create display
    static int old_width = 0;
    static int old_height = 0;

    if ((width != old_width) || (old_height != height)) {

        texturesize[0] = width;
        texturesize[1] = height;

        printf("Old w:%d - h:%d\r\n", old_width, old_height);
        printf("New w:%d - h:%d\r\n", width, height);

        PsxScreenUv[UvTop] = (float) width / psxRealW;
        PsxScreenUv[UvLeft] = (float) height / psxRealH;

        // top left
        Rect[0].u = PsxScreenUv[UvBottom];
        Rect[0].v = PsxScreenUv[UvRight];

        // bottom left
        Rect[4].u = Rect[1].u = PsxScreenUv[UvBottom];
        Rect[4].v = Rect[1].v = PsxScreenUv[UvLeft];

        // top right
        Rect[3].u = Rect[2].u = PsxScreenUv[UvTop];
        Rect[3].v = Rect[2].v = PsxScreenUv[UvRight];

        // bottom right
        Rect[5].u = PsxScreenUv[UvTop];
        Rect[5].v = PsxScreenUv[UvLeft];


        vb = Xe_CreateVertexBuffer(g_pVideoDevice, 6 * sizeof (DrawVerticeFormats));
        void *v = Xe_VB_Lock(g_pVideoDevice, vb, 0, 6 * sizeof (DrawVerticeFormats), XE_LOCK_WRITE);
        memcpy(v, Rect, 6 * sizeof (DrawVerticeFormats));
        Xe_VB_Unlock(g_pVideoDevice, vb);

        old_width = width;
        old_height = height;
    }

}

// Create display

void create_display(void) {
    

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

    // Create the psxScreen texture
    if (g_pTexture)
        Xe_DestroyTexture(g_pVideoDevice, g_pTexture);

    
    g_pTexture = Xe_CreateTexture(g_pVideoDevice, psxRealW, psxRealH, 1, XE_FMT_8888 | XE_FMT_ARGB, 0);
    g_draw.display = (unsigned char*) Xe_Surface_LockRect(g_pVideoDevice, g_pTexture, 0, 0, 0, 0, XE_LOCK_WRITE);
    g_pPitch = g_pTexture->wpitch;
    Xe_Surface_Unlock(g_pVideoDevice, g_pTexture);
    
    // move it to ini file
    float x = -1.0f;
    float y = -1.0f;
    float w = 2.0f;
    float h = 2.0f;

    // top left
    Rect[0].x = x;
    Rect[0].y = y + h;
    Rect[0].u = PsxScreenUv[UvBottom];
    Rect[0].v = PsxScreenUv[UvRight];
    Rect[0].color = 0;

    // bottom left
    Rect[1].x = x;
    Rect[1].y = y;
    Rect[1].u = PsxScreenUv[UvBottom];
    Rect[1].v = PsxScreenUv[UvLeft];
    Rect[1].color = 0;

    // top right
    Rect[2].x = x + w;
    Rect[2].y = y + h;
    Rect[2].u = PsxScreenUv[UvTop];
    Rect[2].v = PsxScreenUv[UvRight];
    Rect[2].color = 0xFF00FF00;

    // top right
    Rect[3].x = x + w;
    Rect[3].y = y + h;
    Rect[3].u = PsxScreenUv[UvTop];
    ;
    Rect[3].v = PsxScreenUv[UvRight];
    Rect[3].color = 0;

    // bottom left
    Rect[4].x = x;
    Rect[4].y = y;
    Rect[4].u = PsxScreenUv[UvBottom];
    Rect[4].v = PsxScreenUv[UvLeft];
    Rect[4].color = 0;

    // bottom right
    Rect[5].x = x + w;
    Rect[5].y = y;
    Rect[5].u = PsxScreenUv[UvTop];
    Rect[5].v = PsxScreenUv[UvLeft];
    Rect[5].color = 0xFF00FF00;

    CreateTexture(psxRealW, psxRealH);

    
    edram_init(g_pVideoDevice);
    return;
}

void rgb_blit_16(int8_t * buff, int32_t x, int32_t y, int32_t w, int32_t h) {
    //fprintf(stderr, "BlitToYUV\n");
    int16_t * src_pxl;
    int8_t * dst_pxl;
    int32_t startxy;
    int32_t _x, _y;

    startxy = 0;
    dst_pxl = buff;
    for (_y = y; _y < h; ++_y) {
        src_pxl = &g_gpu.psx_vram.s16[startxy];
        dst_pxl = (int8_t *) (buff + (_y * g_pPitch));
        
        for (_x = x; _x < w; ++_x) {
            dst_pxl[1] = ((*src_pxl) << 3) & 0xf8;
            dst_pxl[2] = ((*src_pxl) >> 2) & 0xf8;
            dst_pxl[3] = ((*src_pxl) >> 7) & 0xf8;
            dst_pxl[0] = 0xFF;
            dst_pxl += 4;
            src_pxl += 1;
        }
        startxy += 2048;
    }
}

void rgb_blit_24(int8_t * buff, int32_t x, int32_t y, int32_t w, int32_t h) {
    //fprintf(stderr, "BlitToYUV\n");
    int8_t * src_pxl;
    int8_t * dst_pxl;
    int32_t startxy;
    int32_t _x, _y;

    dst_pxl = buff;
    startxy = 0;
    for (_y = y; _y < h; ++_y) {
        src_pxl = &g_gpu.psx_vram.s8[startxy];
        dst_pxl = (int8_t *) (buff + (_y * g_pPitch));
        for (_x = x; _x < w; ++_x) {
            dst_pxl[1] = src_pxl[0];
            dst_pxl[2] = src_pxl[1];
            dst_pxl[3] = src_pxl[2];
            dst_pxl[0] = 0xFF;
            dst_pxl += 4;
            src_pxl += 3;
        }
        startxy += 2048;
    }
}

void do_buffer_swap(void) {
    /*
        g_draw.finalw = g_gpu.dsp.position.x;
        g_draw.finalh = g_gpu.dsp.position.y;

        if (finalw == 0 || finalh == 0)
            return;
     */
    uint32_t _x_, _y_, _w_, _h_;

    if (g_gpu.dsp.mode.x == 0) {
        printf("g_gpu.dsp.mode.x = 0!!!\r\n");
        return;
    }
#if 1
    if (g_gpu.status_reg & STATUS_RGB24) {
        _x_ = (g_gpu.dsp.position.x * 2) / 3;
    } else {
        _x_ = g_gpu.dsp.position.x;
    }
    _y_ = g_gpu.dsp.position.y;
    _w_ = (g_gpu.dsp.range.x1 - g_gpu.dsp.range.x0) / g_gpu.dsp.mode.x;
    _h_ = (g_gpu.dsp.range.y1 - g_gpu.dsp.range.y0) * g_gpu.dsp.mode.y;

    // Update texture size
    CreateTexture(_w_, _h_);
    //BlitScreen32((unsigned char *) g_draw.display, g_gpu.dsp.position.x, g_gpu.dsp.position.y);
    if (g_gpu.status_reg & STATUS_RGB24) {
        rgb_blit_24((int8_t *)g_draw.display, _x_, _y_, _w_, _h_);
    } else {
        rgb_blit_16((int8_t *)g_draw.display, _x_, _y_, _w_, _h_);
    }
#else
    CreateTexture(_w_, _h_);
    BlitScreen32((unsigned char *) g_draw.display, g_gpu.dsp.position.x, g_gpu.dsp.position.y);
#endif
    // refresh texture cache
    Xe_Surface_LockRect(g_pVideoDevice, g_pTexture, 0, 0, 0, 0, XE_LOCK_WRITE);
    Xe_Surface_Unlock(g_pVideoDevice, g_pTexture);

    Xe_InvalidateState(g_pVideoDevice);
    Xe_SetClearColor(g_pVideoDevice, 0);

    Xe_SetBlendOp(g_pVideoDevice, XE_BLENDOP_ADD);
    Xe_SetSrcBlend(g_pVideoDevice, XE_BLEND_SRCALPHA);
    Xe_SetDestBlend(g_pVideoDevice, XE_BLEND_INVSRCALPHA);

    Xe_SetCullMode(g_pVideoDevice, XE_CULL_NONE);
    Xe_SetStreamSource(g_pVideoDevice, 0, vb, 0, sizeof (DrawVerticeFormats));

    Xe_SetTexture(g_pVideoDevice, 0, g_pTexture);

    Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixelTexturedShader, 0);
    Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);

    // set texture size
    Xe_SetVertexShaderConstantF(g_pVideoDevice, 0, texturesize, 1);

    Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_TRIANGLELIST, 0, 2);

    Xe_Resolve(g_pVideoDevice);
    Xe_Sync(g_pVideoDevice);
}

void do_clear_screen_buffer(void) // CLEAR DX BUFFER
{
    Xe_InvalidateState(g_pVideoDevice);
    Xe_SetClearColor(g_pVideoDevice, 0xFF000000);

    Xe_Resolve(g_pVideoDevice);
    Xe_Sync(g_pVideoDevice);
}

void Xcleanup() // X CLEANUP
{
    //CloseMenu();
}

/**
 *  Call on GPUopen
 *  Return the current display.
 **/
unsigned long init_display() {
    create_display(); // x stuff
    g_draw.bIsFirstFrame = 0;
    
    Xe_InvalidateState(g_pVideoDevice);
    Xe_SetClearColor(g_pVideoDevice, 0xFF000000);

    Xe_Resolve(g_pVideoDevice);
    Xe_Sync(g_pVideoDevice);
    
    return 1;
}

void CloseDisplay(void) {
    
    Xcleanup(); // cleanup dx
    DestroyDisplay();
}

void CreatePic(unsigned char * pMem) {
}

void DestroyPic(void) {

}

void ShowGpuPic(void) {
}

void ShowTextGpuPic(void) {
}
