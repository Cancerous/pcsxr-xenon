/*
 * Copyright (c) 2010, Wei Mingzhi <whistler@openoffice.org>.
 * All Rights Reserved.
 *
 * Based on: Cdrom for Psemu Pro like Emulators
 * By: linuzappz <linuzappz@hotmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 */

#ifndef __CDR_H__
#define __CDR_H__

#include "psemu_plugin_defs.h"

int OpenCdHandle();
void CloseCdHandle();
int IsCdHandleOpen();
long GetTN(unsigned char *buffer);
long GetTD(unsigned char track, unsigned char *buffer);
long GetTE(unsigned char track, unsigned char *m, unsigned char *s, unsigned char *f);
long ReadSector(void *cr);
long PlayCDDA(unsigned char *sector);
long StopCDDA();
long GetStatus(int playing, struct CdrStat *stat);
unsigned char *ReadSub(const unsigned char *time);

#endif
