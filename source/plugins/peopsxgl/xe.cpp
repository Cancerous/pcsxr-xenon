#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <stdio.h>
//#define printf
#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}
struct XenosDevice *xe = NULL;

void InitGlSurface() {

}

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
        int j, i = 0;

        int pitch = (surf->wpitch);
        int offset = 0;

        for (y = yoffset; y < (yoffset + height); y++) {
            offset = (y * pitch)+(xoffset * dstbytes);

            /*
                        if(surf->height != surf->hpitch){
                            offset=0;
                        }
             */
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
        int j, i = 0;

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

void XeEnableScissor() {
    // xe->scissor_enable = 1;
}

void XeDisableScissor() {
    //xe->scissor_enable = 0;
}

static int blend_src = XE_BLEND_ONE;
static int blend_dst = XE_BLEND_ZERO;
static int blend_op = XE_BLENDOP_ADD;


void XeAlphaBlend(){
    Xe_SetBlendOpAlpha(xe, XE_BLENDOP_ADD);
    Xe_SetSrcBlendAlpha(xe, XE_BLEND_ONE);
    Xe_SetDestBlendAlpha(xe, XE_BLEND_ZERO);
}

void XeBlendFunc(int src, int dst) {
    blend_src = src;
    blend_dst = dst;

    //Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
    Xe_SetSrcBlend(xe, blend_src);
    Xe_SetDestBlend(xe, blend_dst);
    //    Xe_SetSrcBlendAlpha(xe, XE_BLEND_ONE);
    //    Xe_SetDestBlendAlpha(xe, XE_BLEND_ZERO);

    //    Xe_SetSrcBlendAlpha(xe, blend_src);
    //    Xe_SetDestBlendAlpha(xe, blend_dst);
    XeAlphaBlend();
}

void XeBlendOp(int op) {
    blend_op = op;
    Xe_SetBlendOp(xe, op);
}

void XeAlphaFunc(int func, float ref) {
    // Xe_SetAlphaTestEnable(xe, 1);
    Xe_SetAlphaFunc(xe, func);
    Xe_SetAlphaRef(xe, ref);
}

void XeEnableAlphaTest() {
    Xe_SetAlphaTestEnable(xe, 1);
}

void XeDisableAlphaTest() {
    Xe_SetAlphaTestEnable(xe, 0);
}

void XeDisableBlend() {
    /*
        Xe_SetAlphaTestEnable(xe, 0);
        Xe_SetSrcBlend(xe, XE_BLEND_ONE);
        Xe_SetDestBlend(xe, XE_BLEND_ZERO);
        Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
        Xe_SetSrcBlendAlpha(xe, XE_BLEND_ONE);
        Xe_SetDestBlendAlpha(xe, XE_BLEND_ZERO);
        Xe_SetBlendOpAlpha(xe, XE_BLENDOP_ADD);
     */
    Xe_SetBlendControl(xe,XE_BLEND_ONE,XE_BLENDOP_ADD,XE_BLEND_ZERO,XE_BLEND_ONE,XE_BLENDOP_ADD,XE_BLEND_ZERO);

//    Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
//    Xe_SetBlendOpAlpha(xe, XE_BLENDOP_ADD);
//
//    Xe_SetSrcBlend(xe, XE_BLEND_SRCALPHA);
//    Xe_SetSrcBlendAlpha(xe, XE_BLEND_SRCALPHA);
//
//    Xe_SetDestBlend(xe, XE_BLEND_INVSRCALPHA);
//    Xe_SetDestBlendAlpha(xe, XE_BLEND_INVSRCALPHA);

    //Xe_SetAlphaTestEnable(xe, 0);
    XeAlphaBlend();
}

void XeEnableBlend() {
    /*
        Xe_SetAlphaTestEnable(xe, 1);
        Xe_SetBlendOp(xe, XE_BLENDOP_ADD);
        Xe_SetBlendOpAlpha(xe, XE_BLENDOP_ADD);
     */
    //Xe_SetAlphaTestEnable(xe, 1);
    //Xe_SetBlendControl(xe,blend_src,blend_op,blend_dst,XE_BLEND_ONE,XE_BLENDOP_ADD,XE_BLEND_ZERO);
    Xe_SetBlendControl(xe, blend_src, blend_op, blend_dst, blend_src, blend_op, blend_dst);
    XeAlphaBlend();
}

void XeClear(uint32_t flags) {

}

void XeDepthFunc(int func) {

}

void XeClearColor(float r, float g, float b, float a) {

}

#if 0

void XeClear(uint32_t color) {
    TR
    Xe_SetClearColor(xe, color);
    //Xe_Resolve(xe);
    XeResetStates();
}
#endif