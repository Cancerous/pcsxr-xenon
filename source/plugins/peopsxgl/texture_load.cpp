#define _IN_TEXTURE

#include "stdafx.h"
#include "externals.h"
using namespace xegpu;
#include "texture.h"
#include "gpu.h"
#include "prim.h"
#include "GpuRenderer.h"
#include "texture_load.h"

namespace xegpu{
    
    BOOL bFakeFrontBuffer = FALSE;
    BOOL bIgnoreNextTile = FALSE;

    int iFTex = 512;
}

////////////////////////////////////////////////////////////////////////
// tex window: define
////////////////////////////////////////////////////////////////////////

#include "tex2Sai.h"

void DefineTextureWnd(void) {
    if (gTexName == 0)
        gTexName = gpuRenderer.CreateTexture( TWin.Position.x1, TWin.Position.y1, FMT_A8R8G8B8);
#ifndef WIN32
    gTexName->u_addressing = XE_TEXADDR_WRAP;
    gTexName->v_addressing = XE_TEXADDR_WRAP;

    if (iFilterType && iFilterType < 3 && iHiResTextures != 2) {
        gTexName->use_filtering = XE_TEXF_LINEAR;
    } else {
        gTexName->use_filtering = XE_TEXF_POINT;
    }
#endif
    xeGfx_setTextureData(gTexName, texturepart);
}

void DefineSubTextureSortHiRes(void) {
    int x, y, dx2;

    if (!gTexName) {

        gTexName = gpuRenderer.CreateTexture(512, 512, FMT_A8R8G8B8);
        xeGfx_setTextureData(gTexName, texturepart);
#ifndef WIN32
        gTexName->u_addressing = iClampType;
        gTexName->v_addressing = iClampType;

        if (iFilterType) {
            gTexName->use_filtering = XE_TEXF_LINEAR;
        } else {
            gTexName->use_filtering = XE_TEXF_POINT;
        }
#endif
    }

    if (bGLExt && (iTexQuality == 1 || iTexQuality == 2)) {
        if (DXTexS < 4 || DYTexS < 4 || iHiResTextures == 2) {
            unsigned short * pS, *pD1, *pD2;
            dx2 = (DXTexS << 1);
            pS = (unsigned short *) texturepart;
            pD1 = (unsigned short *) texturebuffer;
            pD2 = (unsigned short *) texturebuffer;
            pD2 += dx2;
            for (y = 0; y < DYTexS; y++) {
                for (x = 0; x < DXTexS; x++) {
                    *(pD2 + 1) = *pD2 = *(pD1 + 1) = *pD1 = *pS;
                    pS++;
                    pD1 += 2;
                    pD2 += 2;
                }
                pD1 += dx2;
                pD2 += dx2;
            }
        } else {
            if (iTexQuality == 1)
                Super2xSaI_ex4(texturepart, DXTexS << 1, texturebuffer, DXTexS, DYTexS);
            else
                Super2xSaI_ex5(texturepart, DXTexS << 1, texturebuffer, DXTexS, DYTexS);
        }
    } else {
        if (DXTexS < 4 || DYTexS < 4 || iHiResTextures == 2) {
            uint32_t * pS, *pD1, *pD2;
            dx2 = (DXTexS << 1);
            pS = (uint32_t *) texturepart;
            pD1 = (uint32_t *) texturebuffer;
            pD2 = (uint32_t *) texturebuffer;
            pD2 += dx2;
            for (y = 0; y < DYTexS; y++) {
                for (x = 0; x < DXTexS; x++) {
                    *(pD2 + 1) = *pD2 = *(pD1 + 1) = *pD1 = *pS;
                    pS++;
                    pD1 += 2;
                    pD2 += 2;
                }
                pD1 += dx2;
                pD2 += dx2;
            }
        } else
            if (bSmallAlpha)
            Super2xSaI_ex8_Ex(texturepart, DXTexS * 4, texturebuffer, DXTexS, DYTexS);
        else
            Super2xSaI_ex8(texturepart, DXTexS * 4, texturebuffer, DXTexS, DYTexS);
    }

    XeTexSubImage(gTexName, 4, 4, XTexS << 1, YTexS << 1, DXTexS << 1, DYTexS << 1, texturebuffer);
}

/////////////////////////////////////////////////////////////////////////////

void DefineSubTextureSort(void) {
    if (iHiResTextures) {
        DefineSubTextureSortHiRes();
        return;
    }
    if (!gTexName) {

        gTexName = gpuRenderer.CreateTexture(256, 256, FMT_A8R8G8B8);
        xeGfx_setTextureData(gTexName, texturepart);
#ifndef WIN32
        gTexName->u_addressing = iClampType;
        gTexName->v_addressing = iClampType;

        if (iFilterType) {
            gTexName->use_filtering = XE_TEXF_LINEAR;
        } else {
            gTexName->use_filtering = XE_TEXF_POINT;
        }
#endif
    }

    XeTexSubImage(gTexName, 4, 4, XTexS, YTexS, DXTexS, DYTexS, texturepart);
}

////////////////////////////////////////////////////////////////////////
// tex window
////////////////////////////////////////////////////////////////////////
void LoadStretchWndTexturePage(int pageid, int mode, short cx, short cy) {
    uint32_t start, row, column, j, sxh, sxm, ldx, ldy, ldxo, s;
    unsigned int palstart;
    uint32_t *px, *pa, *ta;
    unsigned char *cSRCPtr, *cOSRCPtr;
    unsigned short *wSRCPtr, *wOSRCPtr;
    uint32_t LineOffset;
    int pmult = pageid / 16;
    uint32_t(*LTCOL)(uint32_t);

    LTCOL = TCF[DrawSemiTrans];

    ldxo = TWin.Position.x1 - TWin.OPosition.x1;
    ldy = TWin.Position.y1 - TWin.OPosition.y1;

    pa = px = (uint32_t *) ubPaletteBuffer;
    ta = (uint32_t *) texturepart;
    palstart = cx + (cy * 1024);

    ubOpaqueDraw = 0;

    switch (mode) {
            //--------------------------------------------------//
            // 4bit texture load ..
        case 0:
            //------------------- ZN STUFF

            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 4;
                do {
                    *px = LTCOL(ptr32(wSRCPtr));
                    *(px + 1) = LTCOL(ptr32(wSRCPtr + 1));
                    *(px + 2) = LTCOL(ptr32(wSRCPtr + 2));
                    *(px + 3) = LTCOL(ptr32(wSRCPtr + 3));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                column = g_y2 - ldy;
                for (TXV = g_y1; TXV <= column; TXV++) {
                    ldx = ldxo;
                    for (TXU = g_x1; TXU <= g_x2 - ldxo; TXU++) {
                        n_xi = ((TXU >> 2) & ~0x3c) + ((TXV << 2) & 0x3c);
                        n_yi = (TXV & ~0xf) + ((TXU >> 4) & 0xf);

                        s = *(pa + ((ptr32(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x03) << 2)) & 0x0f));
                        *ta++ = s;

                        if (ldx) {
                            *ta++ = s;
                            ldx--;
                        }
                    }

                    if (ldy) {
                        ldy--;
                        for (TXU = g_x1; TXU <= g_x2; TXU++)
                            *ta++ = *(ta - (g_x2 - g_x1));
                    }
                }

                DefineTextureWnd();

                break;
            }

            //-------------------

            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;
            // convert CLUT to 32bits .. and then use THAT as a lookup table

            wSRCPtr = psxVuw + palstart;
            for (row = 0; row < 16; row++)
                *px++ = LTCOL(ptr32(wSRCPtr++));

            sxm = g_x1 & 1;
            sxh = g_x1 >> 1;
            if (sxm) j = g_x1 + 1;
            else j = g_x1;
            cSRCPtr = psxVub + start + (2048 * g_y1) + sxh;
            for (column = g_y1; column <= g_y2; column++) {
                cOSRCPtr = cSRCPtr;
                ldx = ldxo;
                if (sxm) *ta++ = *(pa + ((*cSRCPtr++ >> 4) & 0xF));

                for (row = j; row <= g_x2 - ldxo; row++) {
                    s = *(pa + (*cSRCPtr & 0xF));
                    *ta++ = s;
                    if (ldx) {
                        *ta++ = s;
                        ldx--;
                    }
                    row++;
                    if (row <= g_x2 - ldxo) {
                        s = *(pa + ((*cSRCPtr >> 4) & 0xF));
                        *ta++ = s;
                        if (ldx) {
                            *ta++ = s;
                            ldx--;
                        }
                    }
                    cSRCPtr++;
                }
                if (ldy && column & 1) {
                    ldy--;
                    cSRCPtr = cOSRCPtr;
                } else cSRCPtr = psxVub + start + (2048 * (column + 1)) + sxh;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // 8bit texture load ..
        case 1:
            //------------ ZN STUFF
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 64;
                do {
                    *px = (LTCOL(ptr32(wSRCPtr)));
                    *(px + 1) = (LTCOL(ptr32(wSRCPtr + 1)));
                    *(px + 2) = (LTCOL(ptr32(wSRCPtr + 2)));
                    *(px + 3) = (LTCOL(ptr32(wSRCPtr + 3)));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                column = g_y2 - ldy;
                for (TXV = g_y1; TXV <= column; TXV++) {
                    ldx = ldxo;
                    for (TXU = g_x1; TXU <= g_x2 - ldxo; TXU++) {
                        n_xi = ((TXU >> 1) & ~0x78) + ((TXU << 2) & 0x40) + ((TXV << 3) & 0x38);
                        n_yi = (TXV & ~0x7) + ((TXU >> 5) & 0x7);

                       s = *(pa + ((ptr32(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x01) << 3)) & 0xff));
                        *ta++ = s;
                        if (ldx) {
                            *ta++ = s;
                            ldx--;
                        }
                    }

                    if (ldy) {
                        ldy--;
                        for (TXU = g_x1; TXU <= g_x2; TXU++)
                            *ta++ = *(ta - (g_x2 - g_x1));
                    }

                }

                DefineTextureWnd();

                break;
            }
            //------------

            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;

            // not using a lookup table here... speeds up smaller texture areas
            cSRCPtr = psxVub + start + (2048 * g_y1) + g_x1;
            LineOffset = 2048 - (g_x2 - g_x1 + 1) + ldxo;

            for (column = g_y1; column <= g_y2; column++) {
                cOSRCPtr = cSRCPtr;
                ldx = ldxo;
                for (row = g_x1; row <= g_x2 - ldxo; row++) {
                    s = LTCOL(ptr32(&psxVuw[palstart + *cSRCPtr++]));
                    *ta++ = s;
                    if (ldx) {
                        *ta++ = s;
                        ldx--;
                    }
                }
                if (ldy && column & 1) {
                    ldy--;
                    cSRCPtr = cOSRCPtr;
                } else cSRCPtr += LineOffset;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // 16bit texture load ..
        case 2:
            start = ((pageid - 16 * pmult)*64) + 256 * 1024 * pmult;

            wSRCPtr = psxVuw + start + (1024 * g_y1) + g_x1;
            LineOffset = 1024 - (g_x2 - g_x1 + 1) + ldxo;

            for (column = g_y1; column <= g_y2; column++) {
                wOSRCPtr = wSRCPtr;
                ldx = ldxo;
                for (row = g_x1; row <= g_x2 - ldxo; row++) {
                    s = LTCOL(ptr32(wSRCPtr++));
                    *ta++ = s;
                    if (ldx) {
                        *ta++ = s;
                        ldx--;
                    }
                }
                if (ldy && column & 1) {
                    ldy--;
                    wSRCPtr = wOSRCPtr;
                } else wSRCPtr += LineOffset;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // others are not possible !
    }
}


////////////////////////////////////////////////////////////////////////
// tex window: load simple
////////////////////////////////////////////////////////////////////////
void LoadWndTexturePage(int pageid, int mode, short cx, short cy) {
    uint32_t start, row, column, j, sxh, sxm;
    unsigned int palstart;
    uint32_t *px, *pa, *ta;
    unsigned char *cSRCPtr;
    unsigned short *wSRCPtr;
    uint32_t LineOffset;
    int pmult = pageid / 16;
    uint32_t(*LTCOL)(uint32_t);

    LTCOL = TCF[DrawSemiTrans];

    pa = px = (uint32_t *) ubPaletteBuffer;
    ta = (uint32_t *) texturepart;
    palstart = cx + (cy * 1024);

    ubOpaqueDraw = 0;

    switch (mode) {
            //--------------------------------------------------//
            // 4bit texture load ..
        case 0:
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 4;
                do {
                    *px = LTCOL(ptr32(wSRCPtr));
                    *(px + 1) = LTCOL(ptr32(wSRCPtr + 1));
                    *(px + 2) = LTCOL(ptr32(wSRCPtr + 2));
                    *(px + 3) = LTCOL(ptr32(wSRCPtr + 3));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                for (TXV = g_y1; TXV <= g_y2; TXV++) {
                    for (TXU = g_x1; TXU <= g_x2; TXU++) {
                        n_xi = ((TXU >> 2) & ~0x3c) + ((TXV << 2) & 0x3c);
                        n_yi = (TXV & ~0xf) + ((TXU >> 4) & 0xf);

                        *ta++ = *(pa + ((ptr32(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x03) << 2)) & 0x0f));
                    }
                }

                DefineTextureWnd();

                break;
            }

            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;

            // convert CLUT to 32bits .. and then use THAT as a lookup table

            wSRCPtr = psxVuw + palstart;
            for (row = 0; row < 16; row++)
                *px++ = LTCOL(ptr32(wSRCPtr++));

            sxm = g_x1 & 1;
            sxh = g_x1 >> 1;
            if (sxm) j = g_x1 + 1;
            else j = g_x1;
            cSRCPtr = psxVub + start + (2048 * g_y1) + sxh;
            for (column = g_y1; column <= g_y2; column++) {
                cSRCPtr = psxVub + start + (2048 * column) + sxh;

                if (sxm) *ta++ = *(pa + ((*cSRCPtr++ >> 4) & 0xF));

                for (row = j; row <= g_x2; row++) {
                    *ta++ = *(pa + (*cSRCPtr & 0xF));
                    row++;
                    if (row <= g_x2) *ta++ = *(pa + ((*cSRCPtr >> 4) & 0xF));
                    cSRCPtr++;
                }
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // 8bit texture load ..
        case 1:
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 64;
                do {
                    *px = LTCOL(ptr32(wSRCPtr));
                    *(px + 1) = LTCOL(ptr32(wSRCPtr + 1));
                    *(px + 2) = LTCOL(ptr32(wSRCPtr + 2));
                    *(px + 3) = LTCOL(ptr32(wSRCPtr + 3));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                for (TXV = g_y1; TXV <= g_y2; TXV++) {
                    for (TXU = g_x1; TXU <= g_x2; TXU++) {
                        n_xi = ((TXU >> 1) & ~0x78) + ((TXU << 2) & 0x40) + ((TXV << 3) & 0x38);
                        n_yi = (TXV & ~0x7) + ((TXU >> 5) & 0x7);

                        //*ta++ = *(pa + ((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x01) << 3)) & 0xff));
                        *ta++ = *(pa + ((ptr32(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x01) << 3)) & 0xff));
                    }
                }

                DefineTextureWnd();

                break;
            }

            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;

            // not using a lookup table here... speeds up smaller texture areas
            cSRCPtr = psxVub + start + (2048 * g_y1) + g_x1;
            LineOffset = 2048 - (g_x2 - g_x1 + 1);

            for (column = g_y1; column <= g_y2; column++) {
                for (row = g_x1; row <= g_x2; row++)
                    //*ta++ = LTCOL(psxVuw[palstart + *cSRCPtr++]);
                    *ta++ = LTCOL(ptr32(&psxVuw[palstart + *cSRCPtr++]));
                cSRCPtr += LineOffset;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // 16bit texture load ..
        case 2:
            start = ((pageid - 16 * pmult)*64) + 256 * 1024 * pmult;

            wSRCPtr = psxVuw + start + (1024 * g_y1) + g_x1;
            LineOffset = 1024 - (g_x2 - g_x1 + 1);

            for (column = g_y1; column <= g_y2; column++) {
                for (row = g_x1; row <= g_x2; row++)
                    //*ta++ = LTCOL(*wSRCPtr++);
                    *ta++ = LTCOL(ptr32(wSRCPtr++));
                wSRCPtr += LineOffset;
            }

            DefineTextureWnd();
            break;
            //--------------------------------------------------//
            // others are not possible !
    }
}



////////////////////////////////////////////////////////////////////////
// tex window: main selecting, cache handler included
////////////////////////////////////////////////////////////////////////
GpuTex * LoadTextureWnd(int pageid, int TextureMode, uint32_t GivenClutId) {
    textureWndCacheEntry *ts, *tsx = NULL;
    int i;
    short cx, cy;
    EXLong npos;

    npos._3 = TWin.Position.x0;
    npos._2 = TWin.OPosition.x1;
    npos._1 = TWin.Position.y0;
    npos._0 = TWin.OPosition.y1;

    g_x1 = TWin.Position.x0;
    g_x2 = g_x1 + TWin.Position.x1 - 1;
    g_y1 = TWin.Position.y0;
    g_y2 = g_y1 + TWin.Position.y1 - 1;

    if (TextureMode == 2) {
        GivenClutId = 0;
        cx = cy = 0;
    } else {
        cx = ((GivenClutId << 4) & 0x3F0);
        cy = ((GivenClutId >> 6) & CLUTYMASK);
        GivenClutId = (GivenClutId & CLUTMASK) | (DrawSemiTrans << 30);

        // palette check sum
        {
            uint32_t l = 0, row;
            uint32_t *lSRCPtr = (uint32_t *) (psxVuw + cx + (cy * 1024));
            if (TextureMode == 1) for (row = 1; row < 129; row++) l += (GETLE32(lSRCPtr++) - 1) * row;
            else for (row = 1; row < 9; row++) l += (GETLE32(lSRCPtr++) - 1) << row;
            l = (l + HIWORD(l))&0x3fffL;
            GivenClutId |= (l << 16);
        }

    }

    ts = wcWndtexStore;

    for (i = 0; i < iMaxTexWnds; i++, ts++) {
        if (ts->used) {
            if (ts->pos.l == npos.l &&
                    ts->pageid == pageid &&
                    ts->textureMode == TextureMode) {
                if (ts->ClutID == GivenClutId) {
                    ubOpaqueDraw = ts->Opaque;
                    return ts->texname;
                }
            }
        } else tsx = ts;
    }

    if (!tsx) {
        if (iMaxTexWnds == iTexWndLimit) {
            tsx = wcWndtexStore + iTexWndTurn;
            iTexWndTurn++;
            if (iTexWndTurn == iTexWndLimit) iTexWndTurn = 0;
        } else {
            tsx = wcWndtexStore + iMaxTexWnds;
            iMaxTexWnds++;
        }
    }

    gTexName = tsx->texname;

    if (TWin.OPosition.y1 == TWin.Position.y1 &&
            TWin.OPosition.x1 == TWin.Position.x1) {

        LoadWndTexturePage(pageid, TextureMode, cx, cy);
    }
    else {
        LoadStretchWndTexturePage(pageid, TextureMode, cx, cy);
    }

    tsx->Opaque = ubOpaqueDraw;
    tsx->pos.l = npos.l;
    tsx->ClutID = GivenClutId;
    tsx->pageid = pageid;
    tsx->textureMode = TextureMode;
    tsx->texname = gTexName;
    tsx->used = 1;

    return gTexName;
}


/////////////////////////////////////////////////////////////////////////////
//
// load texture part (unpacked)
/////////////////////////////////////////////////////////////////////////////
void LoadSubTexturePageSort(int pageid, int mode, short cx, short cy) {
    uint32_t start, row, column, j, sxh, sxm;
    unsigned int palstart;
    uint32_t *px, *pa, *ta;
    unsigned char *cSRCPtr;
    unsigned short *wSRCPtr;
    uint32_t LineOffset;
    uint32_t x2a, xalign = 0;
    uint32_t x1 = gl_ux[7];
    uint32_t x2 = gl_ux[6];
    uint32_t y1 = gl_ux[5];
    uint32_t y2 = gl_ux[4];
    uint32_t dx = x2 - x1 + 1;
    uint32_t dy = y2 - y1 + 1;
    int pmult = pageid / 16;
    uint32_t(*LTCOL)(uint32_t);
    unsigned int a, r, g, b, cnt, h;
    uint32_t scol[8];

    LTCOL = TCF[DrawSemiTrans];

    pa = px = (uint32_t *) ubPaletteBuffer;
    ta = (uint32_t *) texturepart;
    palstart = cx + (cy << 10);

    ubOpaqueDraw = 0;

    if (YTexS) {
        ta += dx;
        if (XTexS) ta += 2;
    }
    if (XTexS) {
        ta += 1;
        xalign = 2;
    }

    switch (mode) {
            //--------------------------------------------------//
            // 4bit texture load ..
        case 0:
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 4;
                do {
                    *px = (LTCOL(ptr32(wSRCPtr)));
                    *(px + 1) = (LTCOL(ptr32(wSRCPtr + 1)));
                    *(px + 2) = (LTCOL(ptr32(wSRCPtr + 2)));
                    *(px + 3) = (LTCOL(ptr32(wSRCPtr + 3)));

                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                for (TXV = y1; TXV <= y2; TXV++) {
                    for (TXU = x1; TXU <= x2; TXU++) {
                        n_xi = ((TXU >> 2) & ~0x3c) + ((TXV << 2) & 0x3c);
                        n_yi = (TXV & ~0xf) + ((TXU >> 4) & 0xf);

                        *ta++ = *(pa + ((ptr32(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x03) << 2)) & 0x0f));
                    }
                    ta += xalign;
                }
                break;
            }

            start = ((pageid - 16 * pmult) << 7) + 524288 * pmult;
            // convert CLUT to 32bits .. and then use THAT as a lookup table

            wSRCPtr = psxVuw + palstart;

            row = 4;
            do {
                *px = (LTCOL(ptr32(wSRCPtr)));
                *(px + 1) = (LTCOL(ptr32(wSRCPtr + 1)));
                *(px + 2) = (LTCOL(ptr32(wSRCPtr + 2)));
                *(px + 3) = (LTCOL(ptr32(wSRCPtr + 3)));

                row--;
                px += 4;
                wSRCPtr += 4;
            } while (row);

            x2a = x2 ? (x2 - 1) : 0; //if(x2) x2a=x2-1; else x2a=0;
            sxm = x1 & 1;
            sxh = x1 >> 1;
            j = sxm ? (x1 + 1) : x1; //if(sxm) j=x1+1; else j=x1;
            for (column = y1; column <= y2; column++) {
                cSRCPtr = psxVub + start + (column << 11) + sxh;

                if (sxm) 
                    *ta++ = *(pa + ((*cSRCPtr++ >> 4) & 0xF));

                for (row = j; row < x2a; row += 2) {
                    *ta = *(pa + (*cSRCPtr & 0xF));
                    *(ta + 1) = *(pa + ((*cSRCPtr >> 4) & 0xF));
                    cSRCPtr++;
                    ta += 2;
                }

                if (row <= x2) {
                    *ta++ = *(pa + (*cSRCPtr & 0xF));
                    row++;
                    if (row <= x2) 
                        *ta++ = *(pa + ((*cSRCPtr >> 4) & 0xF));
                }

                ta += xalign;
            }

            break;
            //--------------------------------------------------//
            // 8bit texture load ..
        case 1:
            if (GlobalTextIL) {
                unsigned int TXV, TXU, n_xi, n_yi;

                wSRCPtr = psxVuw + palstart;

                row = 64;
                do {
                    *px = (LTCOL(ptr32(wSRCPtr)));
                    *(px + 1) = (LTCOL(ptr32(wSRCPtr + 1)));
                    *(px + 2) = (LTCOL(ptr32(wSRCPtr + 2)));
                    *(px + 3) = (LTCOL(ptr32(wSRCPtr + 3)));
                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                for (TXV = y1; TXV <= y2; TXV++) {
                    for (TXU = x1; TXU <= x2; TXU++) {
                        n_xi = ((TXU >> 1) & ~0x78) + ((TXU << 2) & 0x40) + ((TXV << 3) & 0x38);
                        n_yi = (TXV & ~0x7) + ((TXU >> 5) & 0x7);

                         *ta++ = *(pa + ((*(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x01) << 3)) & 0xff));
                        //*ta++ = *(pa + ((ptr32(psxVuw + ((GlobalTextAddrY + n_yi)*1024) + GlobalTextAddrX + n_xi) >> ((TXU & 0x01) << 3)) & 0xff));
                    }
                    ta += xalign;
                }

                break;
            }

            start = ((pageid - 16 * pmult) << 7) + 524288 * pmult;

            cSRCPtr = psxVub + start + (y1 << 11) + x1;
            LineOffset = 2048 - dx;

            if (dy * dx > 384) {
                wSRCPtr = psxVuw + palstart;

                row = 64;
                do {
                    *px = (LTCOL(ptr32(wSRCPtr)));
                    *(px + 1) = (LTCOL(ptr32(wSRCPtr + 1)));
                    *(px + 2) = (LTCOL(ptr32(wSRCPtr + 2)));
                    *(px + 3) = (LTCOL(ptr32(wSRCPtr + 3)));

                    row--;
                    px += 4;
                    wSRCPtr += 4;
                } while (row);

                column = dy;
                do {
                    row = dx;
                    do {
                        *ta++ = *(pa + (*cSRCPtr++));
                        row--;
                    } while (row);
                    ta += xalign;
                    cSRCPtr += LineOffset;
                    column--;
                } while (column);
            } else {
                wSRCPtr = psxVuw + palstart;

                column = dy;
                do {
                    row = dx;
                    do {
                        *ta++ = LTCOL(ptr32(wSRCPtr + *cSRCPtr++));
                        row--;
                    } while (row);
                    ta += xalign;
                    cSRCPtr += LineOffset;
                    column--;
                } while (column);
            }

            break;
            //--------------------------------------------------//
            // 16bit texture load ..
        case 2:
            start = ((pageid - 16 * pmult) << 6) + 262144 * pmult;

            wSRCPtr = psxVuw + start + (y1 << 10) + x1;
            LineOffset = 1024 - dx;

            column = dy;
            do {
                row = dx;
                do {
                    *ta++ = (LTCOL(ptr32(wSRCPtr)));

                    row--;
                } while (row);
                ta += xalign;
                wSRCPtr += LineOffset;
                column--;
            } while (column);

            break;
            //--------------------------------------------------//
            // others are not possible !
    }

    x2a = dx + xalign;

    if (YTexS) {
        ta = (uint32_t *) texturepart;
        pa = (uint32_t *) texturepart + x2a;
        row = x2a;
        do {
            *ta++ = *pa++;
            row--;
        } while (row);
        pa = (uint32_t *) texturepart + dy*x2a;
        ta = pa + x2a;
        row = x2a;
        do {
            *ta++ = *pa++;
            row--;
        } while (row);
        YTexS--;
        dy += 2;
    }

    if (XTexS) {
        ta = (uint32_t *) texturepart;
        pa = ta + 1;
        row = dy;
        do {
            *ta = *pa;
            ta += x2a;
            pa += x2a;
            row--;
        } while (row);
        pa = (uint32_t *) texturepart + dx;
        ta = pa + 1;
        row = dy;
        do {
            *ta = *pa;
            ta += x2a;
            pa += x2a;
            row--;
        } while (row);
        XTexS--;
        dx += 2;
    }

    DXTexS = dx;
    DYTexS = dy;

    if (!iFilterType) {
        DefineSubTextureSort();
        return;
    }
    if (iFilterType != 2 && iFilterType != 4 && iFilterType != 6) {
        DefineSubTextureSort();
        return;
    }
    if ((iFilterType == 4 || iFilterType == 6) && ly0 == ly1 && ly2 == ly3 && lx0 == lx3 && lx1 == lx2) {
        DefineSubTextureSort();
        return;
    }

    ta = (uint32_t *) texturepart;
    x1 = dx - 1;
    y1 = dy - 1;
#if 1
    if (bOpaquePass) {
        if (bSmallAlpha) {
            for (column = 0; column < dy; column++) {
                for (row = 0; row < dx; row++) {
                    if (*ta == 0x03000000) {
                        cnt = 0;

                        if (column && *(ta - dx) >> 24 != 0x03) scol[cnt++] = *(ta - dx);
                        if (row && *(ta - 1) >> 24 != 0x03) scol[cnt++] = *(ta - 1);
                        if (row != x1 && *(ta + 1) >> 24 != 0x03) scol[cnt++] = *(ta + 1);
                        if (column != y1 && *(ta + dx) >> 24 != 0x03) scol[cnt++] = *(ta + dx);

                        if (row && column && *(ta - dx - 1) >> 24 != 0x03) scol[cnt++] = *(ta - dx - 1);
                        if (row != x1 && column && *(ta - dx + 1) >> 24 != 0x03) scol[cnt++] = *(ta - dx + 1);
                        if (row && column != y1 && *(ta + dx - 1) >> 24 != 0x03) scol[cnt++] = *(ta + dx - 1);
                        if (row != x1 && column != y1 && *(ta + dx + 1) >> 24 != 0x03) scol[cnt++] = *(ta + dx + 1);

                        if (cnt) {
                            r = g = b = a = 0;
                            for (h = 0; h < cnt; h++) {
                                r += (scol[h] >> 16)&0xff;
                                g += (scol[h] >> 8)&0xff;
                                b += scol[h]&0xff;
                            }
                            r /= cnt;
                            b /= cnt;
                            g /= cnt;

                            *ta = (r << 16) | (g << 8) | b;
                            *ta |= 0x03000000;
                        }
                    }
                    ta++;
                }
            }
        } else {
            for (column = 0; column < dy; column++) {
                for (row = 0; row < dx; row++) {
                    if (*ta == 0x50000000) {
                        cnt = 0;

                        if (column && *(ta - dx) != 0x50000000 && *(ta - dx) >> 24 != 1) scol[cnt++] = *(ta - dx);
                        if (row && *(ta - 1) != 0x50000000 && *(ta - 1) >> 24 != 1) scol[cnt++] = *(ta - 1);
                        if (row != x1 && *(ta + 1) != 0x50000000 && *(ta + 1) >> 24 != 1) scol[cnt++] = *(ta + 1);
                        if (column != y1 && *(ta + dx) != 0x50000000 && *(ta + dx) >> 24 != 1) scol[cnt++] = *(ta + dx);

                        if (row && column && *(ta - dx - 1) != 0x50000000 && *(ta - dx - 1) >> 24 != 1) scol[cnt++] = *(ta - dx - 1);
                        if (row != x1 && column && *(ta - dx + 1) != 0x50000000 && *(ta - dx + 1) >> 24 != 1) scol[cnt++] = *(ta - dx + 1);
                        if (row && column != y1 && *(ta + dx - 1) != 0x50000000 && *(ta + dx - 1) >> 24 != 1) scol[cnt++] = *(ta + dx - 1);
                        if (row != x1 && column != y1 && *(ta + dx + 1) != 0x50000000 && *(ta + dx + 1) >> 24 != 1) scol[cnt++] = *(ta + dx + 1);

                        if (cnt) {
                            r = g = b = a = 0;
                            for (h = 0; h < cnt; h++) {
                                a += (scol[h] >> 24);
                                r += (scol[h] >> 16)&0xff;
                                g += (scol[h] >> 8)&0xff;
                                b += scol[h]&0xff;
                            }
                            r /= cnt;
                            b /= cnt;
                            g /= cnt;

                            //*ta =  SWAP32( (r << 16) | (g << 8) | b );
                            *ta =  ( (r << 16) | (g << 8) | b );
                            if (a) *ta |= 0x50000000;
                            else *ta |= 0x01000000;
                        }
                    }
                    ta++;
                }
            }
        }
    } else
        for (column = 0; column < dy; column++) {
            for (row = 0; row < dx; row++) {
                if (*ta == 0x00000000) {
                    cnt = 0;

                    if (row != x1 && *(ta + 1) != 0x00000000) scol[cnt++] = *(ta + 1);
                    if (column != y1 && *(ta + dx) != 0x00000000) scol[cnt++] = *(ta + dx);

                    if (cnt) {
                        r = g = b = 0;
                        for (h = 0; h < cnt; h++) {
                            r += (scol[h] >> 16)&0xff;
                            g += (scol[h] >> 8)&0xff;
                            b += scol[h]&0xff;
                        }
                        r /= cnt;
                        b /= cnt;
                        g /= cnt;
                        //*ta = SWAP32( (r << 16) | (g << 8) | b);
                        *ta = ( (r << 16) | (g << 8) | b);
                    }
                }
                ta++;
            }
        }
#endif
    DefineSubTextureSort();
}



/////////////////////////////////////////////////////////////////////////////
// main entry for searching/creating textures, called from prim.c
/////////////////////////////////////////////////////////////////////////////
GpuTex * SelectSubTextureS(int TextureMode, uint32_t GivenClutId) {
    unsigned char * OPtr;
    unsigned short iCache;
    short cx, cy;

    // sort sow/tow infos for fast access

    unsigned char ma1, ma2, mi1, mi2;
    if (gl_ux[0] > gl_ux[1]) {
        mi1 = gl_ux[1];
        ma1 = gl_ux[0];
    } else {
        mi1 = gl_ux[0];
        ma1 = gl_ux[1];
    }
    if (gl_ux[2] > gl_ux[3]) {
        mi2 = gl_ux[3];
        ma2 = gl_ux[2];
    } else {
        mi2 = gl_ux[2];
        ma2 = gl_ux[3];
    }
    if (mi1 > mi2) gl_ux[7] = mi2;
    else gl_ux[7] = mi1;
    if (ma1 > ma2) gl_ux[6] = ma1;
    else gl_ux[6] = ma2;

    if (gl_vy[0] > gl_vy[1]) {
        mi1 = gl_vy[1];
        ma1 = gl_vy[0];
    } else {
        mi1 = gl_vy[0];
        ma1 = gl_vy[1];
    }
    if (gl_vy[2] > gl_vy[3]) {
        mi2 = gl_vy[3];
        ma2 = gl_vy[2];
    } else {
        mi2 = gl_vy[2];
        ma2 = gl_vy[3];
    }
    if (mi1 > mi2) gl_ux[5] = mi2;
    else gl_ux[5] = mi1;
    if (ma1 > ma2) gl_ux[4] = ma1;
    else gl_ux[4] = ma2;

    // get clut infos in one 32 bit val

    if (TextureMode == 2) // no clut here
    {
        GivenClutId = CLUTUSED | (DrawSemiTrans << 30);
        cx = cy = 0;

        if (iFrameTexType && Fake15BitTexture())
            return gTexName;
    }
    else {
        cx = ((GivenClutId << 4) & 0x3F0); // but here
        cy = ((GivenClutId >> 6) & CLUTYMASK);
        GivenClutId = (GivenClutId & CLUTMASK) | (DrawSemiTrans << 30) | CLUTUSED;

        // palette check sum.. removed MMX asm, this easy func works as well
        {
            uint32_t l = 0, row;

            uint32_t *lSRCPtr = (uint32_t *) (psxVuw + cx + (cy * 1024));
            if (TextureMode == 1) for (row = 1; row < 129; row++) l += ((GETLE32(lSRCPtr++)) - 1) * row;
            else for (row = 1; row < 9; row++) l += ((GETLE32(lSRCPtr++)) - 1) << row;
            l = (l + HIWORD(l))&0x3fffL;
            GivenClutId |= (l << 16);
        }

    }

    // search cache
    iCache = 0;
    OPtr = CheckTextureInSubSCache(TextureMode, GivenClutId, &iCache);

    // cache full? compress and try again
    if (iCache == 0xffff) {
        CompressTextureSpace();
        OPtr = CheckTextureInSubSCache(TextureMode, GivenClutId, &iCache);
    }

    //printf("OPtr %p\r\n",OPtr);
    
    // found? fine
    usLRUTexPage = iCache;
    if (!OPtr) return uiStexturePage[iCache];

    // not found? upload texture and store infos in cache
    gTexName = uiStexturePage[iCache];
    LoadSubTexFn(GlobalTexturePage, TextureMode, cx, cy);
    uiStexturePage[iCache] = gTexName;
    *OPtr = ubOpaqueDraw;
    return gTexName;
}


/////////////////////////////////////////////////////////////////////////////
// Black texture ...
/////////////////////////////////////////////////////////////////////////////


GpuTex * BlackFake15BitTexture(void) {
    int pmult;
    short x1, x2, y1, y2;

    if (PSXDisplay.InterlacedTest) 
        return 0;

    pmult = GlobalTexturePage / 16;
    x1 = gl_ux[7];
    x2 = gl_ux[6] - gl_ux[7];
    y1 = gl_ux[5];
    y2 = gl_ux[4] - gl_ux[5];

    if (iSpriteTex) {
        if (x2 < 255) x2++;
        if (y2 < 255) y2++;
    }

    y1 += pmult * 256;
    x1 += ((GlobalTexturePage - 16 * pmult) << 6);

    if (FastCheckAgainstFrontScreen(x1, y1, x2, y2)
            || FastCheckAgainstScreen(x1, y1, x2, y2)) {
        if (!gTexFrameName) {

            gTexFrameName = gpuRenderer.CreateTexture(4, 4, FMT_A8R8G8B8);
            gTexFrameName->u_addressing = iClampType;
            gTexFrameName->v_addressing = iClampType;
            

            uint32_t *ta = (uint32_t *) texturepart;
            for (y1 = 0; y1 <= 4; y1++)
                for (x1 = 0; x1 <= 4; x1++)
                    *ta++ = 0xff000000;

            xeGfx_setTextureData(gTexFrameName, texturepart);
        } 
        else {
            gTexName=gTexFrameName;
        }

        ubOpaqueDraw = 0;

        return gTexName;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
GpuTex * Fake15BitTexture(void) {
    return NULL;
    int pmult;
    short x1, x2, y1, y2;
    int iYAdjust;
    float ScaleX, ScaleY;
    RECT rSrc;

    if (iFrameTexType == 1) return BlackFake15BitTexture();
    if (PSXDisplay.InterlacedTest) return 0;

    pmult = GlobalTexturePage / 16;
    x1 = gl_ux[7];
    x2 = gl_ux[6] - gl_ux[7];
    y1 = gl_ux[5];
    y2 = gl_ux[4] - gl_ux[5];

    y1 += pmult * 256;
    x1 += ((GlobalTexturePage - 16 * pmult) << 6);

    if (iFrameTexType == 3) {
        if (iFrameReadType == 4) return 0;

        if (!FastCheckAgainstFrontScreen(x1, y1, x2, y2) &&
                !FastCheckAgainstScreen(x1, y1, x2, y2))
            return 0;

        if (bFakeFrontBuffer) bIgnoreNextTile = TRUE;
        CheckVRamReadEx(x1, y1, x1 + x2, y1 + y2);
        return 0;
    }

    /////////////////////////

    if (FastCheckAgainstFrontScreen(x1, y1, x2, y2)) {
        x1 -= PSXDisplay.DisplayPosition.x;
        y1 -= PSXDisplay.DisplayPosition.y;
    } else
        if (FastCheckAgainstScreen(x1, y1, x2, y2)) {
        x1 -= PreviousPSXDisplay.DisplayPosition.x;
        y1 -= PreviousPSXDisplay.DisplayPosition.y;
    } else return 0;

    bDrawMultiPass = FALSE;

    if (!gTexFrameName)
    {
        void * p;

        if (iResX > 1280 || iResY > 1024) iFTex = 2048;
        else
            if (iResX > 640 || iResY > 480) iFTex = 1024;
        else iFTex = 512;

        gTexFrameName = gpuRenderer.CreateTexture(iFTex, iFTex, FMT_A8R8G8B8);
        gTexFrameName->u_addressing = iClampType;
        gTexFrameName->v_addressing = iClampType;
        
        p = Xe_Surface_LockRect(xe, gTexFrameName, 0, 0, 0, 0, XE_LOCK_WRITE);
        memset(p, 0, iFTex * iFTex * 4);
        Xe_Surface_Unlock(xe, gTexFrameName);
    } 
    else 
    {
        gTexName = gTexFrameName;
    }

    x1 += PreviousPSXDisplay.Range.x0;
    y1 += PreviousPSXDisplay.Range.y0;

    if (PSXDisplay.DisplayMode.x)
        ScaleX = (float) rRatioRect.right / (float) PSXDisplay.DisplayMode.x;
    else ScaleX = 1.0f;
    if (PSXDisplay.DisplayMode.y)
        ScaleY = (float) rRatioRect.bottom / (float) PSXDisplay.DisplayMode.y;
    else ScaleY = 1.0f;

    rSrc.left = max(x1*ScaleX, 0);
    rSrc.right = min((x1 + x2) * ScaleX + 0.99f, iResX - 1);
    rSrc.top = max(y1*ScaleY, 0);
    rSrc.bottom = min((y1 + y2) * ScaleY + 0.99f, iResY - 1);

    iYAdjust = (y1 + y2) - PSXDisplay.DisplayMode.y;
    if (iYAdjust > 0)
        iYAdjust = (int) ((float) iYAdjust * ScaleY) + 1;
    else iYAdjust = 0;

    gl_vy[0] = 255 - gl_vy[0];
    gl_vy[1] = 255 - gl_vy[1];
    gl_vy[2] = 255 - gl_vy[2];
    gl_vy[3] = 255 - gl_vy[3];

    y1 = min(gl_vy[0], min(gl_vy[1], min(gl_vy[2], gl_vy[3])));

    gl_vy[0] -= y1;
    gl_vy[1] -= y1;
    gl_vy[2] -= y1;
    gl_vy[3] -= y1;
    gl_ux[0] -= gl_ux[7];
    gl_ux[1] -= gl_ux[7];
    gl_ux[2] -= gl_ux[7];
    gl_ux[3] -= gl_ux[7];

    ScaleX *= 256.0f / ((float) (iFTex));
    ScaleY *= 256.0f / ((float) (iFTex));

    y1 = ((float) gl_vy[0] * ScaleY);
    if (y1 > 255) y1 = 255;
    gl_vy[0] = y1;
    y1 = ((float) gl_vy[1] * ScaleY);
    if (y1 > 255) y1 = 255;
    gl_vy[1] = y1;
    y1 = ((float) gl_vy[2] * ScaleY);
    if (y1 > 255) y1 = 255;
    gl_vy[2] = y1;
    y1 = ((float) gl_vy[3] * ScaleY);
    if (y1 > 255) y1 = 255;
    gl_vy[3] = y1;

    x1 = ((float) gl_ux[0] * ScaleX);
    if (x1 > 255) x1 = 255;
    gl_ux[0] = x1;
    x1 = ((float) gl_ux[1] * ScaleX);
    if (x1 > 255) x1 = 255;
    gl_ux[1] = x1;
    x1 = ((float) gl_ux[2] * ScaleX);
    if (x1 > 255) x1 = 255;
    gl_ux[2] = x1;
    x1 = ((float) gl_ux[3] * ScaleX);
    if (x1 > 255) x1 = 255;
    gl_ux[3] = x1;

    x1 = rSrc.right - rSrc.left;
    if (x1 <= 0) x1 = 1;
    if (x1 > iFTex) x1 = iFTex;

    y1 = rSrc.bottom - rSrc.top;
    if (y1 <= 0) y1 = 1;
    if (y1 + iYAdjust > iFTex) y1 = iFTex - iYAdjust;
    /*
     if(bFakeFrontBuffer) glReadBuffer(GL_FRONT);

     glCopyTexSubImage2D( GL_TEXTURE_2D, 0,
                          0,
                          iYAdjust,
                          rSrc.left+rRatioRect.left,
                          iResY-rSrc.bottom-rRatioRect.top,
                          x1,y1);

     if(glGetError())
      {
       char * p=(char *)malloc(iFTex*iFTex*4);
       memset(p,0,iFTex*iFTex*4);
       glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iFTex, iFTex,
                       GL_RGB, GL_UNSIGNED_BYTE, p);
       free(p);
      }

     if(bFakeFrontBuffer)
      {glReadBuffer(GL_BACK);bIgnoreNextTile=TRUE;}
     */
    
    bIgnoreNextTile=TRUE;
    
    ubOpaqueDraw = 0;

    if (iSpriteTex) {
        sprtW = gl_ux[1] - gl_ux[0];
        sprtH = -(gl_vy[0] - gl_vy[2]);
    }

    return gTexName;
}