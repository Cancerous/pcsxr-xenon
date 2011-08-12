/*
 * Copyright (C) 2010 Benoit Gschwind
 * Inspired by original author : Pete Bernert
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "cfg.h"


void ReadConfig(void) {
    /* set defaults */
    g_cfg.ResX = 640;
    g_cfg.ResY = 480;
    g_cfg.NoStretch = 0;
    g_cfg.Dithering = 1;
    g_cfg.FullScreen = 0;
    g_cfg.ShowFPS = 0;
    g_cfg.Maintain43 = 0;
    g_cfg.UseFrameLimit = 1;
    g_cfg.UseFrameSkip = 0;
    g_cfg.FPSDetection = 0;
    g_cfg.FrameRate = 200.0;
    g_cfg.CfgFixes = 0;
    g_cfg.UseFixes = 0;

}

void WriteConfig(void) {
    
}
