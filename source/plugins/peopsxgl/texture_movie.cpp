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
        movie_surf = gpuRenderer.CreateTexture(G_W, G_H, FMT_A8R8G8B8);
        
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

    uint32_t * dest = (uint32_t *)gpuRenderer.TextureLock(movie_surf);
    uint32_t * ta = dest; 
    
    if (PSXDisplay.RGB24) {
        unsigned char * pD;

        startxy = ((1024) * xrMovieArea.y0) + xrMovieArea.x0;

        for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++, startxy += 1024, ta+=(movie_surf->wpitch/4)) {
            pD = (unsigned char *) &psxVuw[startxy];
            for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                uint32_t lu = *((uint32_t *) pD);
                ta[row-xrMovieArea.x0]=0xff000000 | (RED(lu) << 16) | (BLUE(lu) << 8) | (GREEN(lu));
                pD += 3;
            }
        }
    } else {
        uint32_t(*LTCOL)(uint32_t);

        LTCOL = XP8RGBA_0; //TCF[0];

        ubOpaqueDraw = 0;

        for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++, ta+=(movie_surf->wpitch/4)) {
            startxy = ((1024) * column) + xrMovieArea.x0;
            for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++){
                ta[row-xrMovieArea.x0]=LTCOL( bswap_32(psxVuw[startxy++] | 0x8000));
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

GpuTex * LoadTextureMovie(void) {
    return LoadTextureMovieFast();
}

