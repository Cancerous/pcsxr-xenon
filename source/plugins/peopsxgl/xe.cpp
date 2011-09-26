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


#include "xe.h"
#include "gl_api.h"

typedef unsigned int DWORD;
#include "psx.ps.h"
#include "psx.ps.f.h"
#include "psx.ps.g.h"
#include "psx.ps.c.h"
#include "psx.vs.h"

//#define printf
#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}
struct XenosDevice *xe = NULL;

#ifdef LZX_GUI 
extern "C"{
    struct XenosDevice * getLzxVideoDevice();
}
#endif

//#include "externals.h"
extern struct XenosDevice *xe;
static struct XenosShader * g_pVertexShader = NULL;
static struct XenosShader * g_pPixelShader = NULL;

static struct XenosShader * g_pPixelShaderC = NULL;
static struct XenosShader * g_pPixelShaderF = NULL;
static struct XenosShader * g_pPixelShaderG = NULL;

#ifndef LZX_GUI  
static struct XenosSurface * fb = NULL;
static struct XenosVertexBuffer *vb = NULL;
static struct XenosDevice _xe;
#endif

extern "C" {
    void doScreenCapture();
}



//------------------------------------------------------------------------------
// Plugin gpu call
//------------------------------------------------------------------------------
void DoBufferSwap() {

    glSync();

    
    Xe_Resolve(xe);
    //while (!Xe_IsVBlank(xe)); 
    Xe_Sync(xe);
    
    doScreenCapture();
    
    // saveShots();
    XeResetStates();

    glReset();
}


unsigned long ulInitDisplay() {
    TR
    
#ifndef LZX_GUI    
    xe = &_xe;
    Xe_Init(xe);
#else
    xe = getLzxVideoDevice();
#endif
    Xe_SetRenderTarget(xe, Xe_GetFramebufferSurface(xe));

    static const struct XenosVBFFormat vbf = {
        3,
        {
            {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT3},
            {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
            {XE_USAGE_COLOR, 0, XE_TYPE_UBYTE4},
        }
    };

    
    g_pPixelShaderC = Xe_LoadShaderFromMemory(xe, (void*) g_xps_psC);
    Xe_InstantiateShader(xe, g_pPixelShaderC, 0);
    
    
    g_pPixelShaderF = Xe_LoadShaderFromMemory(xe, (void*) g_xps_psF);
    Xe_InstantiateShader(xe, g_pPixelShaderF, 0);
    
    g_pPixelShaderG = Xe_LoadShaderFromMemory(xe, (void*) g_xps_psG);
    Xe_InstantiateShader(xe, g_pPixelShaderG, 0);

    g_pPixelShader = Xe_LoadShaderFromMemory(xe, (void*) g_xps_main);
    Xe_InstantiateShader(xe, g_pPixelShader, 0);

    g_pVertexShader = Xe_LoadShaderFromMemory(xe, (void*) g_xvs_main);
    Xe_InstantiateShader(xe, g_pVertexShader, 0);

    Xe_SetShader(xe, SHADER_TYPE_PIXEL, g_pPixelShader, 0);
    Xe_SetShader(xe, SHADER_TYPE_VERTEX, g_pVertexShader, 0);

    Xe_ShaderApplyVFetchPatches(xe, g_pVertexShader, 0, &vbf);

    Xe_InvalidateState(xe);

    Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
    Xe_SetSrcBlend(xe, XE_BLEND_SRCALPHA);
    Xe_SetDestBlend(xe, XE_BLEND_INVSRCALPHA);

    edram_init(xe);
    
    Xe_Resolve(xe);
    Xe_Sync(xe);
    
    glInit();
    
    return 1;
}

void CloseDisplay() {

}




void glStatesChanged();


static int blend_enabled = 0;
static int blend_src = XE_BLEND_ONE;
static int blend_dst = XE_BLEND_ZERO;
static int blend_op = XE_BLENDOP_ADD;

static int alpha_func = 0;
static float alpha_ref = 0;
static int alpha_test_enabled = 0;

static XenosSurface * gl_old_s = NULL;
extern int texture_combiner_enabled;
extern int color_combiner_enabled;


//------------------------------------------------------------------------------
// Gpu Command
//------------------------------------------------------------------------------

/**
 * Blend
 */
void XeDisableBlend() {

    if (blend_enabled) {

        blend_enabled = 0;

        glStatesChanged();
        /*
       Xe_SetAlphaTestEnable(xe, 0);
       Xe_SetSrcBlend(xe, XE_BLEND_ONE);
       Xe_SetDestBlend(xe, XE_BLEND_ZERO);
       Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
       Xe_SetSrcBlendAlpha(xe, XE_BLEND_ONE);
       Xe_SetDestBlendAlpha(xe, XE_BLEND_ZERO);
       Xe_SetBlendOpAlpha(xe, XE_BLENDOP_ADD);
         */
       // Xe_SetBlendControl(xe, XE_BLEND_ONE, XE_BLENDOP_ADD, XE_BLEND_ZERO, XE_BLEND_ONE, XE_BLENDOP_ADD, XE_BLEND_ZERO);

            Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
            Xe_SetBlendOpAlpha(xe, XE_BLENDOP_ADD);
        
            Xe_SetSrcBlend(xe, XE_BLEND_SRCALPHA);
            Xe_SetSrcBlendAlpha(xe, XE_BLEND_SRCALPHA);
        
            Xe_SetDestBlend(xe, XE_BLEND_INVSRCALPHA);
            Xe_SetDestBlendAlpha(xe, XE_BLEND_INVSRCALPHA);

        //Xe_SetAlphaTestEnable(xe, 0);
        XeAlphaBlend();
    }

}

void XeEnableBlend() {
    if (!blend_enabled) {

        blend_enabled = 1;

        glStatesChanged();


        /*
        Xe_SetAlphaTestEnable(xe, 1);
        Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
        Xe_SetBlendOpAlpha(xe, XE_BLENDOP_ADD);
         */
        //Xe_SetAlphaTestEnable(xe, 1);
        //Xe_SetBlendControl(xe,blend_src,blend_op,blend_dst,XE_BLEND_ONE,XE_BLENDOP_ADD,XE_BLEND_ZERO);
        Xe_SetBlendControl(xe, blend_src, blend_op, blend_dst, blend_src, blend_op, blend_dst);
        //XeAlphaBlend();

    }

}

void XeAlphaBlend() {
    Xe_SetBlendOpAlpha(xe, XE_BLENDOP_ADD);
    Xe_SetSrcBlendAlpha(xe, XE_BLEND_ONE);
    Xe_SetDestBlendAlpha(xe, XE_BLEND_ZERO);
}

void XeBlendFunc(int src, int dst) {
    if ((src != blend_src) || (dst != blend_dst)) {
        blend_src = src;
        blend_dst = dst;

        glStatesChanged();

        //Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
        Xe_SetSrcBlend(xe, blend_src);
        Xe_SetDestBlend(xe, blend_dst);
        //    Xe_SetSrcBlendAlpha(xe, XE_BLEND_ONE);
        //    Xe_SetDestBlendAlpha(xe, XE_BLEND_ZERO);

        //    Xe_SetSrcBlendAlpha(xe, blend_src);
        //    Xe_SetDestBlendAlpha(xe, blend_dst);
        //XeAlphaBlend();
    }

}

void XeBlendOp(int op) {
    if (op != blend_op) {
        glStatesChanged();

        blend_op = op;
        Xe_SetBlendOp(xe, op);
    }
}

void XeAlphaFunc(int func, float ref) {
    if ((alpha_func != func) || (alpha_ref != ref)) {

        alpha_func = func;
        alpha_ref = ref;

        glStatesChanged();
        // Xe_SetAlphaTestEnable(xe, 1);
        Xe_SetAlphaFunc(xe, func);
        Xe_SetAlphaRef(xe, ref);
    }
}

void XeEnableAlphaTest() {
    if (!alpha_test_enabled) {
        alpha_test_enabled = 1;

        glStatesChanged();
        Xe_SetAlphaTestEnable(xe, 1);
    }

}

void XeDisableAlphaTest() {
    if (alpha_test_enabled) {

        alpha_test_enabled = 0;

        glStatesChanged();
        Xe_SetAlphaTestEnable(xe, 0);
    }
}

/**
 * Scissor
 */
void XeEnableScissor() {
    // xe->scissor_enable = 1;
}

void XeDisableScissor() {
    //xe->scissor_enable = 0;
}

/**
 * Clear
 */
void XeClear(uint32_t flags) {
    Xe_Clear(xe,~0);
    Xe_Resolve(xe);
}


void XeClearColor(float r, float g, float b, float a) {
    //Xe_SetClearColor(xe, color);
}

/**
 * View
 */
void XeViewport(int left, int top, int right, int bottom) {
    TR
    float persp[4][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };

    Xe_SetVertexShaderConstantF(xe, 0, (float*) persp, 4);

}
static float screen[2] = {0, 0};
void XeOrtho(int l, int r, int b, int t, int zn, int zf) {
    
    
    // TR
    screen[0] = r;
    screen[1] = b;
    printf("setOrtho => %d - %d \r\n", r, b);
    Xe_SetVertexShaderConstantF(xe, 1, (float*) screen, 1);
}

/**
 * Textures
 */
void XeTexSubImage(struct XenosSurface * surf, int srcbpp, int dstbpp, int xoffset, int yoffset, int width, int height, const void * buffer) {
    if (surf) {
        //copy data
        //printf("xeGfx_setTextureData\n");
        uint8_t * surfbuf = (uint8_t*) Xe_Surface_LockRect(xe, surf, 0, 0, 0, 0, XE_LOCK_WRITE);
        uint8_t * srcdata = (uint8_t*) buffer;
        uint8_t * dstdata = surfbuf;
        int srcbytes = srcbpp;
        int dstbytes = dstbpp;
        int y, x;

        int pitch = (surf->wpitch);
        int offset = 0;

        for (y = yoffset; y < (yoffset + height); y++) {
            offset = (y * pitch)+(xoffset * dstbytes);

            dstdata = surfbuf + offset; // ok
            //dstdata = surfbuf + (y*pitch)+(offset);// ok
            for (x = xoffset; x < (xoffset + width); x++) {
                if (srcbpp == 4 && dstbytes == 4) {
                    // 0 a r
                    // 1 r g
                    // 2 g b
                    // 3 b a
                    dstdata[0] = srcdata[0];
                    dstdata[1] = srcdata[1];
                    dstdata[2] = srcdata[2];
                    dstdata[3] = srcdata[3];

                    srcdata += srcbytes;
                    dstdata += dstbytes;
                }
            }
        }

        Xe_Surface_Unlock(xe, surf);
    }
}

void xeGfx_setTextureData(void * tex, void * buffer) {
    //printf("xeGfx_setTextureData\n");
    struct XenosSurface * surf = (struct XenosSurface *) tex;
    if (surf) {
        //copy data
        // printf("xeGfx_setTextureData\n");
        uint8_t * surfbuf = (uint8_t*) Xe_Surface_LockRect(xe, surf, 0, 0, 0, 0, XE_LOCK_WRITE);
        uint8_t * srcdata = (uint8_t*) buffer;
        uint8_t * dstdata = surfbuf;
        int srcbytes = 4;
        int dstbytes = 4;
        int y, x;

        int pitch = surf->wpitch;

        for (y = 0; y < (surf->height); y++) {
            dstdata = surfbuf + (y)*(pitch); // ok
            for (x = 0; x < (surf->width); x++) {

                dstdata[0] = srcdata[0];
                dstdata[1] = srcdata[1];
                dstdata[2] = srcdata[2];
                dstdata[3] = srcdata[3];

                srcdata += srcbytes;
                dstdata += dstbytes;
            }
        }

        Xe_Surface_Unlock(xe, surf);
    }
}


void XeSetTexture(struct XenosSurface * s ){
    if(gl_old_s !=s)
    {
        gl_old_s = s;
        glStatesChanged();
        Xe_SetTexture(xe, 0, s);
    }
}

void XeEnableTexture() {
    texture_combiner_enabled = 1;
}

void XeDisableTexture() {
   texture_combiner_enabled = 0;
   XeSetTexture(NULL);
}

/**
 * Others
 */

void XeDepthFunc(int func) {

}

void XeResetStates() {
    Xe_InvalidateState(xe);
    Xe_SetVertexShaderConstantF(xe, 1, (float*) screen, 1);
    Xe_SetCullMode(xe, XE_CULL_NONE);

    // Wireframe
    //Xe_SetFillMode(xe,0x25,0x25);
}


void XeSetCombinerC(){
    Xe_SetShader(xe, SHADER_TYPE_PIXEL, g_pPixelShaderC, 0);
}

void XeSetCombinerF(){
    Xe_SetShader(xe, SHADER_TYPE_PIXEL, g_pPixelShaderF, 0);
}

void XeSetCombinerG(){
    Xe_SetShader(xe, SHADER_TYPE_PIXEL, g_pPixelShaderG, 0);
}