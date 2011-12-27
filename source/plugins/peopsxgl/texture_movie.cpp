#define _IN_TEXTURE

#include "stdafx.h"
#include "externals.h"
using namespace xegpu;
#include "texture.h"
#include "gpu.h"
#include "prim.h"
#include "GpuRenderer.h"
#include "texture_movie.h"

#define MRED(x)   ((x>>3) & 0x1f)
#define MGREEN(x) ((x>>6) & 0x3e0)
#define MBLUE(x)  ((x>>9) & 0x7c00)

#define XMGREEN(x) ((x>>5)  & 0x07c0)
#define XMRED(x)   ((x<<8)  & 0xf800)
#define XMBLUE(x)  ((x>>18) & 0x003e)

#define G_H     1024
#define G_W     1024

static GpuTex * movie_surf = NULL;

GpuTex * XeDefineTextureMovie(void) {
    if (movie_surf == 0) {
        movie_surf = GetFmvSurf();

        movie_surf->u_addressing = iClampType;
        movie_surf->v_addressing = iClampType;

        if (!bUseFastMdec) {
            movie_surf->use_filtering = XE_TEXF_LINEAR;
        } else {
            movie_surf->use_filtering = XE_TEXF_POINT;
        }
    }
    return movie_surf;
}

////////////////////////////////////////////////////////////////////////
// movie texture: load
////////////////////////////////////////////////////////////////////////

void XeLoadDirectMovieFast(void) {
    int row, column;
    unsigned int startxy;

    uint32_t * dest = (uint32_t *) gpuRenderer.TextureLock(movie_surf);
    uint32_t * ta = dest;

    if (PSXDisplay.RGB24) {
        unsigned char * pD;

        startxy = ((1024) * xrMovieArea.y0) + xrMovieArea.x0;

        for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++, startxy += 1024, ta += (movie_surf->width)) {
            pD = (unsigned char *) &psxVuw[startxy];
            for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                uint32_t lu = *((uint32_t *) pD);
                ta[row] = 0xff000000 | (RED(lu) << 16) | (BLUE(lu) << 8) | (GREEN(lu));
                pD += 3;
            }
        }
    } else {
        uint32_t(*LTCOL)(uint32_t);

        LTCOL = XP8RGBA_0; //TCF[0];

        ubOpaqueDraw = 0;

        for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++, ta += (movie_surf->width)) {
            startxy = ((1024) * column) + xrMovieArea.x0;
            for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                //ta[row] = LTCOL(bswap_32(psxVuw[startxy++] | 0x8000));
                ta[row] = LTCOL((psxVuw[startxy++] | 0x8000));
            }
        }
    }

    gpuRenderer.TextureUnlock(movie_surf);
}

////////////////////////////////////////////////////////////////////////

GpuTex * LoadTextureMovieFast(void) {
    XeDefineTextureMovie();
    XeLoadDirectMovieFast();

    return movie_surf;
}

////////////////////////////////////////////////////////////////////////

GpuTex * __LoadTextureMovie(void) {
    return LoadTextureMovieFast();
}

GpuTex * LoadTextureMovie(void) {
    short row, column, dx;
    unsigned int startxy;
    BOOL b_X, b_Y;

    XeDefineTextureMovie();

    uint32_t * ptr = (uint32_t *) gpuRenderer.TextureLock(movie_surf);

    b_X = FALSE;
    b_Y = FALSE;

    if ((xrMovieArea.x1 - xrMovieArea.x0) < 255) b_X = TRUE;
    if ((xrMovieArea.y1 - xrMovieArea.y0) < 255) b_Y = TRUE;

    {
        if (PSXDisplay.RGB24) {
            unsigned char * pD;
            //uint32_t * ta = (uint32_t *) texturepart;
            uint32_t * ta = ptr;

            if (b_X) {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    ta = (uint32_t*) (ptr + (column * movie_surf->width) + xrMovieArea.x0);
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    pD = (unsigned char *) &psxVuw[startxy];
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        uint32_t lu = *((uint32_t *) pD);
                        *ta++ = 0xff000000 | (RED(lu) << 16) | (BLUE(lu) << 8) | (GREEN(lu));
                        pD += 3;
                    }
                    *ta++ = *(ta - 1);
                }
                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0 + 1;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        *ta++ = *(ta - dx);
                    }
                    *ta++ = *(ta - 1);
                }
            } else {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    ta = (uint32_t*) (ptr + (column * movie_surf->width) + xrMovieArea.x0);
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    pD = (unsigned char *) &psxVuw[startxy];
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        uint32_t lu = *((uint32_t *) pD);
                        *ta++ = 0xff000000 | (RED(lu) << 16) | (BLUE(lu) << 8) | (GREEN(lu));
                        pD += 3;
                    }
                }
                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        *ta++ = *(ta - dx);
                    }
                }
            }
        } else {
            uint32_t(*LTCOL)(uint32_t);
            uint32_t *ta;

            LTCOL = TCF[0];

            ubOpaqueDraw = 0;
            ta = ptr;

            if (b_X) {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    ta = (uint32_t*) (ptr + (column * movie_surf->width) + xrMovieArea.x0);
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        //      *ta++ = LTCOL(bswap_32(psxVuw[startxy++] | 0x8000));
                        //*ta++ = LTCOL((psxVuw[startxy++] | 0x8000));
                        
                        uint16_t s=psxVuw[startxy++];
                       *ta++ =((((s<<19)&0xf80000)|((s<<6)&0xf800)|((s>>7)&0xf8))&0xffffff)|0xff000000;
                        
                    }
                    *ta++ = *(ta - 1);
                }

                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0 + 1;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        *ta++ = *(ta - dx);
                    }
                    *ta++ = *(ta - 1);
                }
            } else {
                for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
                    ta = (uint32_t*) (ptr + (column * movie_surf->width) + xrMovieArea.x0);
                    startxy = ((1024) * column) + xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        //      *ta++ = LTCOL(bswap_32(psxVuw[startxy++] | 0x8000));
                        //*ta++ = LTCOL((psxVuw[startxy++] | 0x8000));
                        
                        uint16_t s=psxVuw[startxy++];
                        *ta++ =((((s<<19)&0xf80000)|((s<<6)&0xf800)|((s>>7)&0xf8))&0xffffff)|0xff000000;
                    }
                }

                if (b_Y) {
                    dx = xrMovieArea.x1 - xrMovieArea.x0;
                    for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                        *ta++ = *(ta - dx);
                    }
                }
            }
        }
    }

    xrMovieArea.x1 += b_X;
    xrMovieArea.y1 += b_Y;
    //DefineTextureMovie();
    xrMovieArea.x1 -= b_X;
    xrMovieArea.y1 -= b_Y;

    gpuRenderer.TextureUnlock(movie_surf);

    return movie_surf;
}

















void DefineTextureMovie(void) {

    if (gTexMovieName == 0) {

        gTexMovieName =gpuRenderer.CreateTexture(256, 256, XE_FMT_8888 | XE_FMT_ARGB);

        xeGfx_setTextureData(gTexMovieName, texturepart);

        gTexMovieName->u_addressing = iClampType;
        gTexMovieName->v_addressing = iClampType;

        if (!bUseFastMdec) {
            gTexMovieName->use_filtering = XE_TEXF_LINEAR;
        } else {
            gTexMovieName->use_filtering = XE_TEXF_POINT;
        }
        gTexName = gTexMovieName;
    } else {
        gTexName = gTexMovieName;
    }

    XeTexSubImage(gTexName, 4, 4, 0, 0, (xrMovieArea.x1 - xrMovieArea.x0), (xrMovieArea.y1 - xrMovieArea.y0), texturepart);
    
//    printf("DefineTextureMovie\r\n");
//    printf("xrMovieArea.x1 =%d\r\n",xrMovieArea.x1);
//    printf("xrMovieArea.x0 =%d\r\n",xrMovieArea.x0);
//    printf("xrMovieArea.y1 =%d\r\n",xrMovieArea.y1);
//    printf("xrMovieArea.y0 =%d\r\n\r\n",xrMovieArea.y0);
}


GpuTex * _o_LoadTextureMovie(void) {
    short row, column, dx;
    unsigned int startxy;
    BOOL b_X, b_Y;

    b_X = FALSE;
    b_Y = FALSE;

    if ((xrMovieArea.x1 - xrMovieArea.x0) < 255) b_X = TRUE;
    if ((xrMovieArea.y1 - xrMovieArea.y0) < 255) b_Y = TRUE;

    {
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