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

typedef unsigned int DWORD;
#include "psx.ps.h"
#include "psx.vs.h"
#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}

#define MAX_VERTEX_COUNT 16384

//#include "externals.h"
extern struct XenosDevice *xe;
static struct XenosShader * g_pVertexShader = NULL;
static struct XenosShader * g_pPixelShader = NULL;
static struct XenosSurface * fb = NULL;
static struct XenosVertexBuffer *vb = NULL;
static struct XenosDevice _xe;



static int vertexCount = 0;
float screen[2] = {0, 0};
static PsxVerticeFormats * vertices = NULL;

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

    }
/*
    // uv mod
    psxvertices[0].u += gPsxUV.bottom;
    psxvertices[0].v += gPsxUV.right;
    
    psxvertices[1].u += gPsxUV.bottom;
    psxvertices[1].v += gPsxUV.left;
    
    psxvertices[2].u += gPsxUV.top;
    psxvertices[2].v += gPsxUV.right;
    
    psxvertices[3].u += gPsxUV.top;
    psxvertices[3].v += gPsxUV.left;
*/
}

void iXeDrawTri(PsxVerticeFormats * psxvertices) {

#if 1
    fpoint(psxvertices);

    Xe_SetStreamSource(xe, 0, vb, vertexCount, 4);
    vertices = (PsxVerticeFormats *) Xe_VB_Lock(xe, vb, vertexCount, 3 * sizeof (PsxVerticeFormats), XE_LOCK_WRITE);
    memcpy(vertices, psxvertices, 3 * sizeof (PsxVerticeFormats));
    Xe_VB_Unlock(xe, vb);

    Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 1);

    vertexCount += 3 * sizeof (PsxVerticeFormats);
#endif

}

void iXeDrawTri2(PsxVerticeFormats * psxvertices) {
    fpoint(psxvertices);

    Xe_SetStreamSource(xe, 0, vb, vertexCount, 4);
    vertices = (PsxVerticeFormats *) Xe_VB_Lock(xe, vb, vertexCount, 4 * sizeof (PsxVerticeFormats), XE_LOCK_WRITE);
    memcpy(vertices, psxvertices, 4 * sizeof (PsxVerticeFormats));
    Xe_VB_Unlock(xe, vb);

    Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLESTRIP, 0, 2);

    vertexCount += 4 * sizeof (PsxVerticeFormats);
}

void iXeDrawQuad(PsxVerticeFormats * psxvertices) {
    fpoint(psxvertices);

    Xe_SetStreamSource(xe, 0, vb, vertexCount, 4);
    vertices = (PsxVerticeFormats *) Xe_VB_Lock(xe, vb, vertexCount, 6 * sizeof (PsxVerticeFormats), XE_LOCK_WRITE);

    memcpy(&vertices[0], &psxvertices[0], 1 * sizeof (PsxVerticeFormats));
    memcpy(&vertices[1], &psxvertices[1], 1 * sizeof (PsxVerticeFormats));
    memcpy(&vertices[2], &psxvertices[2], 1 * sizeof (PsxVerticeFormats));
    memcpy(&vertices[3], &psxvertices[1], 1 * sizeof (PsxVerticeFormats));
    memcpy(&vertices[4], &psxvertices[3], 1 * sizeof (PsxVerticeFormats));
    memcpy(&vertices[5], &psxvertices[2], 1 * sizeof (PsxVerticeFormats));
    Xe_VB_Unlock(xe, vb);

    Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 2);
    vertexCount += 6 * sizeof (PsxVerticeFormats);

}

unsigned long ulInitDisplay() {
    TR
    xe = &_xe;

    Xe_Init(xe);
    fb = Xe_GetFramebufferSurface(xe);
    Xe_SetRenderTarget(xe, fb);

    static const struct XenosVBFFormat vbf = {
        3,
        {
            {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT4},
            {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
            // {XE_USAGE_COLOR, 0, XE_TYPE_UBYTE4},
            {XE_USAGE_COLOR, 0, XE_TYPE_FLOAT4},
        }
    };


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

    edram_init(xe);
    return 1;
}

void CloseDisplay() {

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

void XeClear(uint32_t color) {
    TR
    Xe_SetClearColor(xe, color);
    //Xe_Resolve(xe);
    XeResetStates();
}

int ss = 0;
char sshot[256];

void saveShots() {
    static unsigned long lastTick = 10000;
    static int frames = 0;
    unsigned long nowTick;
    frames++;
    nowTick = mftb() / (PPC_TIMEBASE_FREQ / 1000);
    if (lastTick + 10000 <= nowTick) {
        printf("SaveFbToPng\r\n");
        sprintf(sshot,"uda:/sshot.%03d.png",ss);
        SaveFbToPng(sshot);
        ss++;
        frames = 0;
        lastTick = nowTick;
    }
}

void DoBufferSwap() {
    //TR
    Xe_Resolve(xe);
    //TR
    Xe_Sync(xe);
    //TR
    //printf("draw %d vertices\r\n",vertexCount);
    vertexCount = 0;
    //saveShots();
    XeResetStates();


    // save things ...
    //

}

void PEOPS_GPUdisplayText(char * pText) // some debug func
{
    printf(pText);
}

struct ati_info {
	uint32_t unknown1[4];
	uint32_t base;
	uint32_t unknown2[8];
	uint32_t width;
	uint32_t height;
} __attribute__ ((__packed__)) ;

uint32_t fb_backup[1280*720*4];

void SaveFbToPng(const char *filename) {
    /* create file */
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("SaveFbToPng failed\r\n");
        return;
    }
    
    uint32_t height = *((uint32_t*)0xec806138);
    uint32_t width = *((uint32_t*)0xec806134);
    uint32_t pitch = *((uint32_t*)0xec806120);
    
    struct ati_info *ai = (struct ati_info*)0xec806100ULL;
    
    //uint8_t *data = (int8_t *)0xec806110;
    unsigned char * data = (unsigned char*)(long)(ai->base | 0x80000000);
    uint32_t * fb = (uint32_t*)data;
    
    png_structp png_ptr;
    png_infop info_ptr;
    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);


    if (!png_ptr) {
        printf("[read_png_file] png_create_read_struct failed\n");
        return 0;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        printf("[read_png_file] png_create_info_struct failed\n");
        return 0;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height,
            8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);
    //png_set_packing(png_ptr);
    
    // wait for gpu to be ready            
/*
    while((*((uint32_t*)0xec801740) & 0x80000000) == 0) {
            // Wait for gpu Lock
    };
*/
    //uint32_t * fb_backup = (uint32_t *)malloc(((height*(width*4))*sizeof(uint32_t*)));
    
    int x,y;
    for(y=0;y<height;y++){
        for(x=0;x<width;x++){
#define base (((y >> 5)*32*width + ((x >> 5)<<10) \
    + (x&3) + ((y&1)<<2) + (((x&31)>>2)<<3) + (((y&31)>>1)<<6)) ^ ((y&8)<<2))
            fb_backup[base] = &fb+(y*width)+x;
        }
    }
    png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);

    TR;
    for (y=0; y<height; y++)
        row_pointers[y] = (png_bytep)(u8 *)fb_backup + y * (width*4);

    TR;
    png_write_image(png_ptr, row_pointers);

    png_write_end(png_ptr, NULL);
    
    free(row_pointers);
   // free(fb_backup);
    fclose(fp);
}