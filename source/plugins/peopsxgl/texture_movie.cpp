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

void DefineTextureMovie(void) {

    if (gTexMovieName == 0) {

        gTexMovieName =gpuRenderer.CreateTexture(256, 256, FMT_A8R8G8B8);

        xeGfx_setTextureData(gTexMovieName, texturepart);
#ifndef WIN32
        gTexMovieName->u_addressing = iClampType;
        gTexMovieName->v_addressing = iClampType;

        if (!bUseFastMdec) {
            gTexMovieName->use_filtering = XE_TEXF_LINEAR;
        } else {
            gTexMovieName->use_filtering = XE_TEXF_POINT;
        }
#endif
        gTexName = gTexMovieName;
    } else {
        gTexName = gTexMovieName;
    }

    XeTexSubImage(gTexName, 4, 4, 0, 0, (xrMovieArea.x1 - xrMovieArea.x0), (xrMovieArea.y1 - xrMovieArea.y0), texturepart);
}

////////////////////////////////////////////////////////////////////////
// movie texture: load
////////////////////////////////////////////////////////////////////////
unsigned char * LoadDirectMovieFast(void) {
    int row, column;
    unsigned int startxy;

    uint32_t *ta = (uint32_t *) texturepart;

    if (PSXDisplay.RGB24) {
        unsigned char * pD;

        startxy = ((1024) * xrMovieArea.y0) + xrMovieArea.x0;

        for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++, startxy += 1024) {
            pD = (unsigned char *) &psxVuw[startxy];
            for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++) {
                uint32_t lu = *((uint32_t *) pD);
                *ta++ = 0xff000000 | (RED(lu) << 16) | (BLUE(lu) << 8) | (GREEN(lu));
                pD += 3;
            }
        }
    } else {
        uint32_t(*LTCOL)(uint32_t);

        LTCOL = XP8RGBA_0; //TCF[0];

        ubOpaqueDraw = 0;

        for (column = xrMovieArea.y0; column < xrMovieArea.y1; column++) {
            startxy = ((1024) * column) + xrMovieArea.x0;
            for (row = xrMovieArea.x0; row < xrMovieArea.x1; row++)
                *ta++ = LTCOL( bswap_32(psxVuw[startxy++] | 0x8000));
        }
    }

    return texturepart;
}

////////////////////////////////////////////////////////////////////////

GpuTex * LoadTextureMovieFast(void) {
    LoadDirectMovieFast();
    DefineTextureMovie();
    return gTexName;
}

////////////////////////////////////////////////////////////////////////

GpuTex * LoadTextureMovie(void) {
    return LoadTextureMovieFast();
}
