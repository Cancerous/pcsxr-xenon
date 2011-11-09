/***************************************************************************
                           cfg.c  -  description
                             -------------------
    begin                : Sun Mar 08 2009
    copyright            : (C) 1999-2009 by Pete Bernert
    web                  : www.pbernert.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

//*************************************************************************//
// History of changes:
//
// 2009/03/08 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#define _IN_CFG

#include "stdafx.h"
#include "externals.h"
#include "cfg.h"
BOOL bGteAccuracy; 
void ReadConfig(void) // read config (linux file)
{
    iResX = 1280;
    iResY = 720;
    bGteAccuracy = TRUE;
    bFullVRam = FALSE;
    iFilterType = 5; //0: None , 5: Standard + smoothed Sprites, 6: Extended + smoothed Sprites
    bDrawDither = FALSE;
    bUseLines = FALSE;
    bUseFrameLimit = TRUE;
    bUseFrameSkip = FALSE;
    iFrameLimit = 2;
    fFrameRate = 200.0f;
    iOffscreenDrawing = 2; //2: Standard - OK for most games
    bOpaquePass = FALSE;//TRUE
    bUseAntiAlias = FALSE;
    iTexQuality = 4; // 4 - B8 G8 R8 A8 - Slightly faster with some cards | 3 - R8 G8 B8 A8 - Best colors, more ram needed
    iUseMask = 1;
    iZBufferDepth = 16;
    bUseFastMdec = TRUE;
    dwCfgFixes = 0;
    bUseFixes = FALSE;
    iFrameTexType = 0;//  0: Emulated vram - effects need FVP | 1: Black - Fast but no special effects | 2: Gfx card buffer - Can be slow | 4: Full software drawing (FVP)
    iFrameReadType = 0;
    bUse15bitMdec = FALSE;
    iShowFPS = 0;
    bKeepRatio = FALSE;
    iScanBlend = 0;
    iVRamSize = 128;
    iTexGarbageCollection = 1;
    iBlurBuffer = 0;
    iHiResTextures = 1; //0: None (standard) ,1: 2xSaI (much vram needed) ,2: Stretched (filtering needed)
    
    if (iUseMask) iZBufferDepth = 16; // set zbuffer depth
    else iZBufferDepth = 0;
    if (bUseFixes) dwActFixes = dwCfgFixes; // init game fix global
}
