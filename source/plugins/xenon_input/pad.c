/*
 * Copyright (c) 2009, Wei Mingzhi <whistler@openoffice.org>.
 * All Rights Reserved.
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

#include "pad.h"

/* Button Bits */
#define PSX_BUTTON_TRIANGLE ~(1 << 12)
#define PSX_BUTTON_SQUARE 	~(1 << 15)
#define PSX_BUTTON_CROSS	~(1 << 14)
#define PSX_BUTTON_CIRCLE	~(1 << 13)
#define PSX_BUTTON_L2		~(1 << 8)
#define PSX_BUTTON_R2		~(1 << 9)
#define PSX_BUTTON_L1		~(1 << 10)
#define PSX_BUTTON_R1		~(1 << 11)
#define PSX_BUTTON_SELECT	~(1 << 0)
#define PSX_BUTTON_START	~(1 << 3)
#define PSX_BUTTON_DUP		~(1 << 4)
#define PSX_BUTTON_DRIGHT	~(1 << 5)
#define PSX_BUTTON_DDOWN	~(1 << 6)
#define PSX_BUTTON_DLEFT	~(1 << 7)

#define PSX_CONTROLLER_TYPE_STANDARD 0
#define PSX_CONTROLLER_TYPE_ANALOG 1

int controllerType = PSX_CONTROLLER_TYPE_STANDARD; // 0 = standard, 1 = analog (analog fails on old games)

long PadFlags = 0;

void PSxInputReadPort(PadDataS* pad, int port) {

    unsigned short pad_status = 0xFFFF;
    int ls_x, ls_y, rs_x, rs_y;

    static struct controller_data_s c;
    usb_do_poll();

    if (get_controller_data(&c, port)) {
    }

    if (c.a)
        pad_status &= PSX_BUTTON_CROSS;
    if (c.b)
        pad_status &= PSX_BUTTON_CIRCLE;
    if (c.y)
        pad_status &= PSX_BUTTON_TRIANGLE;
    if (c.x)
        pad_status &= PSX_BUTTON_SQUARE;
    if (c.up)
        pad_status &= PSX_BUTTON_DUP;
    if (c.down)
        pad_status &= PSX_BUTTON_DDOWN;
    if (c.left)
        pad_status &= PSX_BUTTON_DLEFT;
    if (c.right)
        pad_status &= PSX_BUTTON_DRIGHT;
    if (c.start)
        pad_status &= PSX_BUTTON_START;
    if (c.select)
        pad_status &= PSX_BUTTON_SELECT;
    if (c.rb)
        pad_status &= PSX_BUTTON_R1;
    if (c.lb)
        pad_status &= PSX_BUTTON_L1;
    if (c.rt > 100)
        pad_status &= PSX_BUTTON_R2;
    if (c.lt > 100)
        pad_status &= PSX_BUTTON_L2;

    pad->leftJoyX = 0;
    pad->leftJoyY = 0;

    pad->rightJoyX = 0;
    pad->rightJoyY = 0;

    pad->controllerType = PSE_PAD_TYPE_STANDARD; // Standard Pad
    pad->buttonStatus = pad_status; //Copy Buttons
};

long PAD__init(long flags) {
    PadFlags |= flags;
    return PSE_PAD_ERR_SUCCESS;
}

long PAD__shutdown(void) {
    return PSE_PAD_ERR_SUCCESS;
}

long PAD__open(unsigned long *Disp) {
    return PSE_PAD_ERR_SUCCESS;
}

long PAD__close(void) {
    return PSE_PAD_ERR_SUCCESS;
}

long PAD__readPort1(PadDataS* pad) {
    PSxInputReadPort(pad, 0);
    return PSE_PAD_ERR_SUCCESS;
}

long PAD__readPort2(PadDataS* pad) {
    PSxInputReadPort(pad, 1);
    pad->buttonStatus = 0xffff;
    pad->controllerType = PSE_PAD_TYPE_STANDARD;
    return PSE_PAD_ERR_SUCCESS;
}
