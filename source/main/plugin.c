/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2003  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include "plugins.h"
#include <time.h>
#include <stdio.h>
#include "r3000a.h"
#include "misc.h"
#include "sio.h"

int ShowPic = 0;

const char * PcsxrDir = "uda:/";
extern void LidInterrupt();

void gpuShowPic() {
    
}

void GetStateFilename(char *out, int i) {
    char trimlabel[33];
    int j;

    strncpy(trimlabel, CdromLabel, 32);
    trimlabel[32] = 0;
    for (j = 31; j >= 0; j--)
        if (trimlabel[j] == ' ')
            trimlabel[j] = '\0';

    sprintf(out, "sstates\\%.32s-%.9s.%3.3d", trimlabel, CdromId, i);
}

void PADhandleKey(int key) {
        return;
}

void CALLBACK SPUirq(void);

char charsTable[4] = {"|/-\\"};


#define PARSEPATH(dst, src) \
	ptr = src + strlen(src); \
	while (*ptr != '\\' && ptr != src) ptr--; \
	if (ptr != src) { \
		strcpy(dst, ptr+1); \
	}

int _OpenPlugins() {
    int ret;

    GPU_clearDynarec(clearDynarec);

    ret = CDR_open();
    if (ret < 0) {
        SysMessage(_("Error Opening CDR Plugin"));
        return -1;
    }
    
    ret = GPU_open(NULL, "PCSXR", NULL);
    if (ret < 0) {
        SysMessage(_("Error Opening GPU Plugin (%d)"), ret);
        return -1;
    }
    //GPU_registerCallback(GPUbusy);
    ret = SPU_open();
    if (ret < 0) {
        SysMessage(_("Error Opening SPU Plugin (%d)"), ret);
        return -1;
    }
    SPU_registerCallback(SPUirq);
    ret = PAD1_open(NULL);
    if (ret < 0) {
        SysMessage(_("Error Opening PAD1 Plugin (%d)"), ret);
        return -1;
    }
    //PAD1_registerVibration(GPU_visualVibration);
    //PAD1_registerCursor(GPU_cursor);
    ret = PAD2_open(NULL);
    if (ret < 0) {
        SysMessage(_("Error Opening PAD2 Plugin (%d)"), ret);
        return -1;
    }
    //PAD2_registerVibration(GPU_visualVibration);
    //PAD2_registerCursor(GPU_cursor);

    return 0;
}

int OpenPlugins() {
    int ret;

    while ((ret = _OpenPlugins()) == -2) {
        ReleasePlugins();
        LoadMcds(Config.Mcd1, Config.Mcd2);
        if (LoadPlugins() == -1) return -1;
    }
    return ret;
}

void ClosePlugins() {
    int ret;

    // PAD plugins have to be closed first, otherwise some plugins like
    // LilyPad will mess up the window handle and cause crash.
    // Also don't check return value here, as LilyPad uses void.
    PAD1_close();
    PAD2_close();

    ret = CDR_close();
    if (ret < 0) {
        SysMessage(_("Error Closing CDR Plugin"));
        return;
    }
    ret = GPU_close();
    if (ret < 0) {
        SysMessage(_("Error Closing GPU Plugin"));
        return;
    }
    ret = SPU_close();
    if (ret < 0) {
        SysMessage(_("Error Closing SPU Plugin"));
        return;
    }

    if (Config.UseNet) {
        NET_pause();
    }
}

void ResetPlugins() {
    int ret;

    CDR_shutdown();
    GPU_shutdown();
    SPU_shutdown();
    PAD1_shutdown();
    PAD2_shutdown();

    ret = CDR_init();
    if (ret != 0) {
        SysMessage(_("CDRinit error: %d"), ret);
        return;
    }
    ret = GPU_init();
    if (ret != 0) {
        SysMessage(_("GPUinit error: %d"), ret);
        return;
    }
    ret = SPU_init();
    if (ret != 0) {
        SysMessage(_("SPUinit error: %d"), ret);
        return;
    }
    ret = PAD1_init(1);
    if (ret != 0) {
        SysMessage(_("PAD1init error: %d"), ret);
        return;
    }
    ret = PAD2_init(2);
    if (ret != 0) {
        SysMessage(_("PAD2init error: %d"), ret);
        return;
    }
    NetOpened = FALSE;
}
