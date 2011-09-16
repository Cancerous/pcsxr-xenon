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
#include "config.h"
#include "pad.h"


#define STICK_THRESHOLD 12000

long PadFlags = 0;
static void (*gpuVisualVibration)(unsigned long, unsigned long) = NULL;

GLOBALDATA g;


#define	STICK_DEAD_ZONE (32768*0.3)
#define HANDLE_STICK_DEAD_ZONE(x) ((((x)>-STICK_DEAD_ZONE) && (x)<STICK_DEAD_ZONE)?0:(x-x/abs(x)*STICK_DEAD_ZONE))

#define	TRIGGER_DEAD_ZONE (256*0.3)
#define HANDLE_TRIGGER_DEAD_ZONE(x) (((x)<TRIGGER_DEAD_ZONE)?0:(x-TRIGGER_DEAD_ZONE))


struct controller_data_s old[2];
int reset_time = 0;

void PSxInputReadPort(PadDataS* pad, int port) {
    unsigned short pad_status = 0xFFFF;
    int ls_x, ls_y, rs_x, rs_y;

    usb_do_poll();
    if (get_controller_data(&g.PadState[port].JoyDev, port)) {
       
    }

/*
    if (g.PadState[port].JoyDev.logo) {
       // exit(0);
        if(old[port].logo){
            reset_time++;
            if(reset_time>50){
                exit(0);//return to xell
            }
        }
        else{
            reset_time = 0;
        }
    }
*/
    if (g.PadState[port].JoyDev.rb && g.PadState[port].JoyDev.lb){
//        PEOPS_SPUclose();
        //xenon_sleep_thread(2);
        //xenon_set_single_thread_mode();
 //       exit(0);//return to xell
    }
        
    

    pad->controllerType = g.cfg.PadDef[port].Type; // Standard Pad

    if (g.PadState[port].JoyDev.a)
        pad_status &= PSX_BUTTON_CROSS;
    if (g.PadState[port].JoyDev.b)
        pad_status &= PSX_BUTTON_CIRCLE;
    if (g.PadState[port].JoyDev.y)
        pad_status &= PSX_BUTTON_TRIANGLE;
    if (g.PadState[port].JoyDev.x)
        pad_status &= PSX_BUTTON_SQUARE;
    if (g.PadState[port].JoyDev.up)
        pad_status &= PSX_BUTTON_DUP;
    if (g.PadState[port].JoyDev.down)
        pad_status &= PSX_BUTTON_DDOWN;
    if (g.PadState[port].JoyDev.left)
        pad_status &= PSX_BUTTON_DLEFT;
    if (g.PadState[port].JoyDev.right)
        pad_status &= PSX_BUTTON_DRIGHT;
    if (g.PadState[port].JoyDev.start)
        pad_status &= PSX_BUTTON_START;
    if (g.PadState[port].JoyDev.select)
        pad_status &= PSX_BUTTON_SELECT;
    if (g.PadState[port].JoyDev.rb)
        pad_status &= PSX_BUTTON_R1;
    if (g.PadState[port].JoyDev.lb)
        pad_status &= PSX_BUTTON_L1;
    if (g.PadState[port].JoyDev.rt > 100)
        pad_status &= PSX_BUTTON_R2;
    if (g.PadState[port].JoyDev.lt > 100)
        pad_status &= PSX_BUTTON_L2;

    if (g.cfg.PadDef[port].Type == PSE_PAD_TYPE_STANDARD) {
        if (g.PadState[port].JoyDev.s1_y > STICK_THRESHOLD)
            pad_status &= PSX_BUTTON_DUP;
        if (g.PadState[port].JoyDev.s1_y<-STICK_THRESHOLD)
            pad_status &= PSX_BUTTON_DDOWN;
        if (g.PadState[port].JoyDev.s1_x > STICK_THRESHOLD)
            pad_status &= PSX_BUTTON_DRIGHT;
        if (g.PadState[port].JoyDev.s1_x<-STICK_THRESHOLD)
            pad_status &= PSX_BUTTON_DLEFT;
    } else {
/*        
        ls_x= (int)(float(g.PadState[port].JoyDev.s1_x/0x500)*256)+128;
        ls_y= (int)(float(g.PadState[port].JoyDev.s1_y/0x500)*256)+128;
        
        rs_x= (int)(float(g.PadState[port].JoyDev.s2_x/0x500)*256)+128;
        rs_y= (int)(float(g.PadState[port].JoyDev.s2_y/0x500)*256)+128;
*/        
        
        ls_x= (int)((float)(g.PadState[port].JoyDev.s1_x/0x500)*256)+128;
        ls_y= (int)((float)(g.PadState[port].JoyDev.s1_y/0x500)*256)+128;
        
        rs_x= (int)((float)(g.PadState[port].JoyDev.s2_x/0x500)*256)+128;
        rs_y= (int)((float)(g.PadState[port].JoyDev.s2_y/0x500)*256)+128;
        
        pad->leftJoyX = ls_x;
        pad->leftJoyY = ls_y;
        pad->rightJoyX = rs_x;
        pad->rightJoyY = rs_y;
    }


    old[port] = g.PadState[port].JoyDev;
    // Copy Buttons
    pad->buttonStatus = pad_status;
};


long PAD__init(long flags) {
    g.cfg.PadDef[0].Type = PSE_PAD_TYPE_STANDARD;//PSE_PAD_TYPE_ANALOGPAD;
    g.cfg.PadDef[1].Type = PSE_PAD_TYPE_STANDARD;//PSE_PAD_TYPE_ANALOGPAD;

    gpuVisualVibration = NULL;
    return PSE_PAD_ERR_SUCCESS;
}

long PAD__shutdown(void) {
    return PSE_PAD_ERR_SUCCESS;
}

long PAD__open(unsigned long *Disp) {
    g.Opened = 1;
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
    return PSE_PAD_ERR_SUCCESS;
}

void PAD__registerVibration(void (*callback)(unsigned long, unsigned long)) {
    gpuVisualVibration = callback;
}

long PAD__query(void) {
    return PSE_PAD_USE_PORT1 | PSE_PAD_USE_PORT2;
}

