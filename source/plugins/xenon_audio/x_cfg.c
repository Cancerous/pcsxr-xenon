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

#include "stdafx.h"

#define _IN_CFG

#include "externals.h"

////////////////////////////////////////////////////////////////////////
// LINUX CONFIG/ABOUT HANDLING
////////////////////////////////////////////////////////////////////////

#include <unistd.h>

void PEOPS_SPUsetConfigFile(char * pCfg) {


}
///////////////////////////////////////////
// READ CONFIG called by spu funcs
/////////////////////////////////////////////////////////

void ReadConfig(void) {
    printf("ReadSpuConfig\r\n");
    
    iVolume = 1;
    iXAPitch = 0;
    iSPUIRQWait = 0;
    iUseTimer = 2;// USE THREAD
    iUseReverb = 2;
    iUseInterpolation = 2;
    iDisStereo = 0;
    iFreqResponse = 0;
}
