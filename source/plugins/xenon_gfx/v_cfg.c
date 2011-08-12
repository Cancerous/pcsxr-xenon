/***************************************************************************
                          cfg.c  -  description
                             -------------------
    begin                : Sun Oct 28 2001
    copyright            : (C) 2001 by Pete Bernert
    email                : BlackDove@addcom.de
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

#define _IN_CFG

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#undef FALSE
#undef TRUE
#define MAKELONG(low,high)     ((unsigned long)(((unsigned short)(low)) | (((unsigned long)((unsigned short)(high))) << 16)))

#include "externals.h"
#include "cfg.h"
#include "gpu.h"

void ReadConfig(void) {
    printf("ReadGpuConfig\r\n");
    // defaults
    iResX = 640;
    iResY = 480;
    iWinSize = MAKELONG(iResX, iResY);
    iColDepth = 32;
    iWindowMode = 1;
    iMaintainAspect = 0;
    UseFrameLimit = 1;
    UseFrameSkip = 0;
    iFrameLimit = 2;
    fFrameRate = 200.0f;
    dwCfgFixes = 0x401;
    iUseFixes = 1;
    iUseNoStretchBlt = 0;
    iUseDither = 0;
    iShowFPS = 0;

    // additional checks
    if (!iColDepth) 
        iColDepth = 32;
    if (iUseFixes) 
        dwActFixes = dwCfgFixes;
    SetFixes();
}

void WriteConfig(void) {
    
}
