#include <xenos/xe.h>
#include <xenos/xenos.h>
#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}
struct XenosDevice *xe = NULL;
// used only for some param ....
static struct XenosSurface tmpsurf;


#define MAX_GL_SURFACE 128

struct GlSurface {
    struct XenosSurface * surf;
    int id;
    int used;
};

struct GlSurface XeTextureArray[MAX_GL_SURFACE];

void InitGlSurface() {
    TR
    int i = 0;
    for (i = 0; i < MAX_GL_SURFACE; i++) {
        XeTextureArray[i].surf = NULL;
        XeTextureArray[i].id = i;
        XeTextureArray[i].used = 0;
    }
}
/*

void xeGfx_setTextureData(void * tex, void * buffer) {
    //printf("xeGfx_setTextureData\n");
    struct XenosSurface * surf = (struct XenosSurface *) tex;
    u8 * buf = (u8*) buffer;

    u8 * surfbuf;
    int j, i;
    surfbuf = (u8*) Xe_Surface_LockRect(xe, surf, 0, 0, 0, 0, XE_LOCK_WRITE);
    for (j = 0; j < surf->hpitch; ++j)
        for (i = 0; i < surf->wpitch; i += surf->width * 4)
            memcpy(&surfbuf[surf->wpitch * j + i], &buf[surf->width * (j % surf->height)*4], surf->width * 4);

    Xe_Surface_Unlock(xe, surf);
}
*/
void xeGfx_setTextureData(void * tex, void * buffer) {
    //printf("xeGfx_setTextureData\n");
    struct XenosSurface * surf = (struct XenosSurface *) tex;
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
        
        for (y = 0; y < (surf->height); y++)
	{
            //dstdata = surfbuf + (y * width + xoffset) * dstbytes;
            //srcdata = buffer + ((surf->width)*(y))+x;
            //dstdata = surfbuf + (y-yoffset)*(surf->wpitch);// ok
            dstdata = surfbuf + (y)*(pitch);// ok
            //srcdata = buffer + ((surf->width)*(y))+xoffset;
            for (x = 0; x < (surf->width); x++)
            {
                // 0 a r
                // 1 r g
                // 2 g b
                // 3 b a

                dstdata[0] = srcdata[3];
                dstdata[1] = srcdata[0];
                dstdata[2] = srcdata[1];
                dstdata[3] = srcdata[2];

/*
                dstdata[0] = 255;//a
                dstdata[1] = 255;//r
                dstdata[2] = 0;//g
                dstdata[3] = 0;//b
*/
                
                srcdata += srcbytes;
                dstdata += dstbytes;
            }            
        }
        
        Xe_Surface_Unlock(xe,surf);
    }
}
/*
void xeTexImage(int id,int w,int h, void * buffer){
     struct XenosSurface * surf = NULL;
     int i = 0;
     for(i=0;i<MAX_GL_SURFACE;i++){
         if(XeTextureArray[i].id==id){
             surf = XeTextureArray[i].surf;
             if(surf)
                Xe_DestroyTexture(xe,surf);
             
             Xe_CreateTexture(xe,w,h,0,XE_FMT_8888|XE_FMT_ARGB,0);
             xeGfx_setTextureData(surf,buffer);
         }
    }
}
 */

/*
void xeTexImage(int id,int w,int h, void * buffer){
     if(selectedSurf!=NULL){
         
         if(selectedSurf->surf)
            Xe_DestroyTexture(xe,selectedSurf->surf);
         
         Xe_CreateTexture(xe,w,h,0,XE_FMT_8888|XE_FMT_ARGB,0);
         xeGfx_setTextureData(selectedSurf->surf,buffer);
     }
}
 */

void xeGenTextures(int nbr, int * id) {
    int i = 0;
    int j;
    for (j = 0; j < nbr; j++) {
        for (i = 0; i < MAX_GL_SURFACE; i++) {
            if (XeTextureArray[i].used == 0) {
                id[j] = XeTextureArray[i].id;
                XeTextureArray[i].used = 1;
                printf("xeGenTextures id=%d\r\n", i);
                break;
            }
        }
    }

}

void xeDeleteTextures(int nbr, int * id) {
    TR;
    int i = 0;
    int j;
    for (j = 0; j < nbr; j++) {
        for (i = 0; i < MAX_GL_SURFACE; i++) {
            if (XeTextureArray[i].id == id[j]) {
                printf("xeDeleteTextures id=%d\r\n", id[j]);
                XeTextureArray[i].used = 0;
                if (XeTextureArray[i].surf != NULL)
                    Xe_DestroyTexture(xe, XeTextureArray[i].surf);
                break;
            }
        }
    }
}

int iXeSelectedSurf = 0;

struct XenosSurface * xeGetTextureInSlot() {
    if (iXeSelectedSurf)
        if (XeTextureArray[iXeSelectedSurf].surf)
            return XeTextureArray[iXeSelectedSurf].surf;
    printf("Texture not found ...\r\n");
    return NULL;
}

void xeBindTexture(int i) {
    //printf("xeBindTexture %d\r\n", i);
    iXeSelectedSurf = i;
    Xe_SetTexture(xe, 0, NULL);
    if (XeTextureArray[i].surf == NULL) {
        printf("Hmm?\r\n");
    }
}

void xeSetTexture() {
    if (XeTextureArray[iXeSelectedSurf].surf)
        Xe_SetTexture(xe, 0, XeTextureArray[iXeSelectedSurf].surf);
    else
        Xe_SetTexture(xe, 0, NULL);
}

void xeTexImage(int internalformat, int width, int height, int format, const void *pixels) {
    //printf("xeTexImage(%d,%d,%d,%x,%p)\r\n", internalformat, width, height, format, pixels);
    //TR;
    if (XeTextureArray[iXeSelectedSurf].used) {

        if (XeTextureArray[iXeSelectedSurf].surf)
            Xe_DestroyTexture(xe, XeTextureArray[iXeSelectedSurf].surf);
        XeTextureArray[iXeSelectedSurf].surf = NULL;
        //TR;
        XeTextureArray[iXeSelectedSurf].surf = Xe_CreateTexture(xe, width, height, 1, format, 0);
        //TR;
        // take if from tmp instance
        XeTextureArray[iXeSelectedSurf].surf->u_addressing = tmpsurf.u_addressing;
        XeTextureArray[iXeSelectedSurf].surf->v_addressing = tmpsurf.v_addressing;
        XeTextureArray[iXeSelectedSurf].surf->use_filtering = tmpsurf.use_filtering;
       // TR;
        if (pixels)
            xeGfx_setTextureData(XeTextureArray[iXeSelectedSurf].surf, pixels);
        //TR;
    } else {
        TR
    }
}

void XeViewport(int left, int top, int right, int bottom) {
    TR

/*
    float persp[4][4] = {
        {w,0,0,2*x+w-1.0f},
        {0,h,0,-2*y-h+1.0f},
        {0,0,0.5f,0.5f},
            {0,0,0,1},
    };
 */

            float persp[4][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };

    Xe_SetVertexShaderConstantF(xe, 0, (float*) persp, 4);

}

void XeEnableTexture() {

}

void XeDisableTexture() {
    Xe_SetTexture(xe, 0, NULL);
}

void XeEnableScissor() {
    // xe->scissor_enable = 1;
}

void XeDisableScissor() {
    //xe->scissor_enable = 0;
}

void XeAlphaFunc(int func, float ref) {

    Xe_SetAlphaTestEnable(xe, 1);
    Xe_SetAlphaFunc(xe, func);
    Xe_SetAlphaRef(xe, ref);


}

void XeDisableBlend() {

    Xe_SetAlphaTestEnable(xe, 0);

    /*
        Xe_SetSrcBlend(xe,XE_BLEND_ONE);
        Xe_SetDestBlend(xe,XE_BLEND_ZERO);
        Xe_SetBlendOp(xe,XE_BLENDOP_ADD);
        Xe_SetSrcBlendAlpha(xe,XE_BLEND_ONE);
        Xe_SetDestBlendAlpha(xe,XE_BLEND_ZERO);
        Xe_SetBlendOpAlpha(xe,XE_BLENDOP_ADD);
     */


}

void XeEnableBlend() {

    Xe_SetAlphaTestEnable(xe, 1);
    /*
        Xe_SetBlendOp(xe,XE_BLENDOP_ADD);
        Xe_SetBlendOpAlpha(xe,XE_BLENDOP_ADD);
     */


}

void xeTexWrap(int uwrap, int vwrap) {
    tmpsurf.u_addressing = uwrap;
    tmpsurf.v_addressing = vwrap;
}

void xeTexUseFiltering(int enabled) {
    tmpsurf.use_filtering = enabled;
}