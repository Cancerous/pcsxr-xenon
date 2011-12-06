/***************************************************************************
cfg.c  -  description
-------------------
begin                : Wed May 15 2002
copyright            : (C) 2002 by Pete Bernert
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

//*************************************************************************//
// History of changes:
//
// 2003/06/07 - Pete
// - added Linux NOTHREADLIB define
//
// 2003/02/28 - Pete
// - added option for kode54's interpolation and linuzappz's mono mode
//
// 2003/01/19 - Pete
// - added Neill's reverb
//
// 2002/08/04 - Pete
// - small linux bug fix: now the cfg file can be in the main emu directory as well
//
// 2002/06/08 - linuzappz
// - Added combo str for SPUasync, and MAXMODE is now defined as 2
//
// 2002/05/15 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#include "stdafx.h"

#define _IN_CFG

#include "externals.h"

#include <unistd.h>

extern int iZincEmu;



char * pConfigFile = NULL;


////////////////////////////////////////////////////////////////////////
// START EXTERNAL CFG TOOL
////////////////////////////////////////////////////////////////////////

void StartCfgTool(char * pCmdLine) {

}


/////////////////////////////////////////////////////////
// READ CONFIG called by spu funcs
/////////////////////////////////////////////////////////

void ReadConfig(void) {
    iVolume = 10;
    iUseXA = 1;
    iXAPitch = 0;
    iSPUIRQWait = 1;
    iUseTimer = 0;
    iUseReverb = 2;
    iUseInterpolation = 2;
    iDisStereo = 0;
    iUseDBufIrq = 0;
    iFreqResponse = 0;

    if (iZincEmu) {
        iVolume = 1; // with ZINC, max volume is needed (or qsound will be too loud)
        iDisStereo = 0; // with ZINC, no mono possible (or qsound mixing troubles)
    }

}

