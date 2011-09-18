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

#include <png.h>
#include <pngconf.h>

#include "xe.h"
#include "gl_api.h"

typedef unsigned int DWORD;
#include "psx.ps.h"
#include "psx.ps.f.h"
#include "psx.ps.g.h"
#include "psx.ps.c.h"
#include "psx.vs.h"
#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}

#define MAX_VERTEX_COUNT 16384

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

static struct XenosSurface * fb = NULL;
static struct XenosVertexBuffer *vb = NULL;
static struct XenosDevice _xe;

static int vertexCount = 0;
float screen[2] = {0, 0};

PsxVerticeFormats * PsxVertex = NULL;

void SaveFbToPng(const char *filename);

void ResetVb() {

}

void CreateVb() {
    vb = Xe_CreateVertexBuffer(xe, MAX_VERTEX_COUNT * sizeof (PsxVerticeFormats));
}

void fpoint(PsxVerticeFormats * psxvertices) {
    psxvertices[0].w = psxvertices[1].w = psxvertices[2].w = psxvertices[3].w = 1;
    int i = 0;
    for (i = 0; i < 4; i++) {

        // psxvertices[i].x = psxvertices[i].x / 640.f;
        // psxvertices[i].y = psxvertices[i].y / 480.f;
        psxvertices[i].x = ((psxvertices[i].x / screen[0])*2.f) - 1.0f;
        psxvertices[i].y = ((psxvertices[i].y / screen[1])*2.f) - 1.0f;
        psxvertices[i].z = -psxvertices[i].z;
    }
}

void LockVb() {
    Xe_SetStreamSource(xe, 0, vb, vertexCount, 4);
    PsxVertex = (PsxVerticeFormats *) Xe_VB_Lock(xe, vb, vertexCount, 4 * sizeof (PsxVerticeFormats), XE_LOCK_WRITE);
}

void UnlockVb() {
    //Xe_VB_Unlock(xe, vb);
}

void iXeDrawTri(PsxVerticeFormats * psxvertices) {
    fpoint(psxvertices);
    Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 1);
    Xe_VB_Unlock(xe, vb);
    vertexCount += 3 * sizeof (PsxVerticeFormats);
}

void iXeDrawTri2(PsxVerticeFormats * psxvertices) {
    fpoint(psxvertices);
    Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLESTRIP, 0, 2);
    Xe_VB_Unlock(xe, vb);
    vertexCount += 4 * sizeof (PsxVerticeFormats);
}

void iXeDrawQuad(PsxVerticeFormats * psxvertices) {
    fpoint(psxvertices);
    //Xe_DrawPrimitive(xe, XE_PRIMTYPE_QUADLIST, 0, 1);
    Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLESTRIP, 0, 2);
    Xe_VB_Unlock(xe, vb);
    vertexCount += 4 * sizeof (PsxVerticeFormats);
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
            {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT4},
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

    CreateVb();

    Xe_InvalidateState(xe);

    Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
    Xe_SetSrcBlend(xe, XE_BLEND_SRCALPHA);
    Xe_SetDestBlend(xe, XE_BLEND_INVSRCALPHA);


    InitGlSurface();
#ifdef USE_GL_API
    glInit();
#endif
    edram_init(xe);
    
    Xe_Resolve(xe);
    Xe_Sync(xe);
    
    return 1;
}

void CloseDisplay() {

}

void DoBufferSwap() {
    Xe_Resolve(xe);
    Xe_Sync(xe);
    //TR
    //printf("draw %d vertices\r\n",vertexCount);
    vertexCount = 0;
    
#ifdef USE_GL_API
    glReset();
#endif
    
    // saveShots();
    XeResetStates();

}

void XeOrtho(int l, int r, int b, int t, int zn, int zf) {
    // TR
    screen[0] = r;
    screen[1] = b;
    printf("setOrtho => %d - %d \r\n", r, b);
    Xe_SetVertexShaderConstantF(xe, 1, (float*) screen, 1);
}

void XeResetStates() {
    Xe_InvalidateState(xe);
    Xe_SetVertexShaderConstantF(xe, 1, (float*) screen, 1);
    Xe_SetCullMode(xe, XE_CULL_NONE);

    //Xe_SetFillMode(xe,0x25,0x25);
}


void XeClear(){

/*
    Xe_Clear(xe,~0);
    Xe_Resolve(xe);
*/

    //Xe_ResolveInto(xe,Xe_GetFramebufferSurface(xe),0,XE_CLEAR_COLOR);
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