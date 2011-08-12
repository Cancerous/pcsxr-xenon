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
static int padDataLenght[] = {0, 2, 3, 1, 1, 3, 3, 3};
static void (*gpuVisualVibration)(unsigned long, unsigned long) = NULL;

GLOBALDATA g;

static uint8_t stdpar[2][8] = {
    {0xFF, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80},
    {0xFF, 0x5A, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80}
};

static uint8_t unk46[2][8] = {
    {0xFF, 0x5A, 0x00, 0x00, 0x01, 0x02, 0x00, 0x0A},
    {0xFF, 0x5A, 0x00, 0x00, 0x01, 0x02, 0x00, 0x0A}
};

static uint8_t unk47[2][8] = {
    {0xFF, 0x5A, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00},
    {0xFF, 0x5A, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00}
};

static uint8_t unk4c[2][8] = {
    {0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static uint8_t unk4d[2][8] = {
    {0xFF, 0x5A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0x5A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

static uint8_t stdcfg[2][8] = {
    {0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static uint8_t stdmode[2][8] = {
    {0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFF, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static uint8_t stdmodel[2][8] = {
    {
        0xFF,
        0x5A,
        0x01, // 03 - dualshock2, 01 - dualshock
        0x02, // number of modes
        0x01, // current mode: 01 - analog, 00 - digital
        0x02,
        0x01,
        0x00
    },
    {
        0xFF,
        0x5A,
        0x01, // 03 - dualshock2, 01 - dualshock
        0x02, // number of modes
        0x01, // current mode: 01 - analog, 00 - digital
        0x02,
        0x01,
        0x00
    }
};

void PAD__setMode(const int pad, const int mode) {
    g.PadState[pad].PadMode = mode;

    if (g.cfg.PadDef[pad].Type == PSE_PAD_TYPE_ANALOGPAD) {
        g.PadState[pad].PadID = mode ? 0x73 : 0x41;
    } else {
        g.PadState[pad].PadID = (g.cfg.PadDef[pad].Type << 4) |
                padDataLenght[g.cfg.PadDef[pad].Type];
    }

    g.PadState[pad].Vib0 = 0;
    g.PadState[pad].Vib1 = 0;
    g.PadState[pad].VibF[0] = 0;
    g.PadState[pad].VibF[1] = 0;
}

void PSxInputReadPort(PadDataS* pad, int port) {
    unsigned short pad_status = 0xFFFF;
    int ls_x, ls_y, rs_x, rs_y;

    usb_do_poll();
    if (get_controller_data(&g.PadState[port].JoyDev, port)) {
        /*
        printf("\r\n");
        printf("s1_y %04x - %d\r\n",c.s1_y);
        printf("s1_x %04x - %d\r\n",c.s1_x);
        printf("s2_y %04x - %d\r\n",c.s2_y);
        printf("s2_x %04x - %d\r\n",c.s2_x);
        printf("\r\n");
         */
    }

    if (&g.PadState[port].JoyDev.logo) {
        g.PadState[port].PadModeSwitch = 0;
        PAD__setMode(port, 1 - g.PadState[port].PadMode);
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
        rs_x = (g.PadState[port].JoyDev.s1_x + 127)&0xFF;
        rs_y = (g.PadState[port].JoyDev.s1_y + 127)&0xFF;

        ls_x = (g.PadState[port].JoyDev.s2_x + 127)&0xFF;
        ls_y = (g.PadState[port].JoyDev.s2_y + 127)&0xFF;

        pad->leftJoyX = ls_x;
        pad->leftJoyY = ls_y;
        pad->rightJoyX = rs_x;
        pad->rightJoyY = rs_y;
    }


    // Copy Buttons
    pad->buttonStatus = pad_status;
};

static uint8_t CurPad = 0, CurByte = 0, CurCmd = 0, CmdLen = 0;

unsigned char PAD__startPoll(int pad) {
    CurPad = pad - 1;
    CurByte = 0;

    return 0xFF;
}

unsigned char PAD__poll(unsigned char value) {
    static uint8_t *buf = NULL;
    uint16_t n;

    if (CurByte == 0) {
        CurByte++;

        // Don't enable Analog/Vibration for a standard pad
        if (g.cfg.PadDef[CurPad].Type != PSE_PAD_TYPE_ANALOGPAD) {
            CurCmd = CMD_READ_DATA_AND_VIBRATE;
        } else {
            CurCmd = value;
        }

        switch (CurCmd) {
            case CMD_CONFIG_MODE:
                CmdLen = 8;
                buf = stdcfg[CurPad];
                if (stdcfg[CurPad][3] == 0xFF) return 0xF3;
                else return g.PadState[CurPad].PadID;

            case CMD_SET_MODE_AND_LOCK:
                CmdLen = 8;
                buf = stdmode[CurPad];
                return 0xF3;

            case CMD_QUERY_MODEL_AND_MODE:
                CmdLen = 8;
                buf = stdmodel[CurPad];
                buf[4] = g.PadState[CurPad].PadMode;
                return 0xF3;

            case CMD_QUERY_ACT:
                CmdLen = 8;
                buf = unk46[CurPad];
                return 0xF3;

            case CMD_QUERY_COMB:
                CmdLen = 8;
                buf = unk47[CurPad];
                return 0xF3;

            case CMD_QUERY_MODE:
                CmdLen = 8;
                buf = unk4c[CurPad];
                return 0xF3;

            case CMD_VIBRATION_TOGGLE:
                CmdLen = 8;
                buf = unk4d[CurPad];
                return 0xF3;

            case CMD_READ_DATA_AND_VIBRATE:
            default:
                n = g.PadState[CurPad].KeyStatus;
                n &= g.PadState[CurPad].JoyKeyStatus;

                stdpar[CurPad][2] = n & 0xFF;
                stdpar[CurPad][3] = n >> 8;

                if (g.PadState[CurPad].PadMode == 1) {
                    CmdLen = 8;

                    stdpar[CurPad][4] = g.PadState[CurPad].AnalogStatus[ANALOG_RIGHT][0];
                    stdpar[CurPad][5] = g.PadState[CurPad].AnalogStatus[ANALOG_RIGHT][1];
                    stdpar[CurPad][6] = g.PadState[CurPad].AnalogStatus[ANALOG_LEFT][0];
                    stdpar[CurPad][7] = g.PadState[CurPad].AnalogStatus[ANALOG_LEFT][1];
                } else if (g.PadState[CurPad].PadID == 0x12) {
                    CmdLen = 6;

                    stdpar[CurPad][4] = g.PadState[0].MouseAxis[0][0];
                    stdpar[CurPad][5] = g.PadState[0].MouseAxis[0][1];
                } else {
                    CmdLen = 4;
                }

                buf = stdpar[CurPad];
                return g.PadState[CurPad].PadID;
        }
    }

    switch (CurCmd) {
        case CMD_READ_DATA_AND_VIBRATE:
            if (g.cfg.PadDef[CurPad].Type == PSE_PAD_TYPE_ANALOGPAD) {
                if (CurByte == g.PadState[CurPad].Vib0) {
                    g.PadState[CurPad].VibF[0] = value;

                    if (g.PadState[CurPad].VibF[0] != 0 || g.PadState[CurPad].VibF[1] != 0) {
                        gpuVisualVibration(g.PadState[CurPad].VibF[0], g.PadState[CurPad].VibF[1]);
                    }
                }

                if (CurByte == g.PadState[CurPad].Vib1) {
                    g.PadState[CurPad].VibF[1] = value;

                    if (g.PadState[CurPad].VibF[0] != 0 || g.PadState[CurPad].VibF[1] != 0) {
                        gpuVisualVibration(g.PadState[CurPad].VibF[0], g.PadState[CurPad].VibF[1]);
                    }
                }
            }
            break;

        case CMD_CONFIG_MODE:
            if (CurByte == 2) {
                switch (value) {
                    case 0:
                        buf[2] = 0;
                        buf[3] = 0;
                        break;

                    case 1:
                        buf[2] = 0xFF;
                        buf[3] = 0xFF;
                        break;
                }
            }
            break;

        case CMD_SET_MODE_AND_LOCK:
            if (CurByte == 2) {
                PAD__setMode(CurPad, value);
            }
            break;

        case CMD_QUERY_ACT:
            if (CurByte == 2) {
                switch (value) {
                    case 0: // default
                        buf[5] = 0x02;
                        buf[6] = 0x00;
                        buf[7] = 0x0A;
                        break;

                    case 1: // Param std conf change
                        buf[5] = 0x01;
                        buf[6] = 0x01;
                        buf[7] = 0x14;
                        break;
                }
            }
            break;

        case CMD_QUERY_MODE:
            if (CurByte == 2) {
                switch (value) {
                    case 0: // mode 0 - digital mode
                        buf[5] = PSE_PAD_TYPE_STANDARD;
                        break;

                    case 1: // mode 1 - analog mode
                        buf[5] = PSE_PAD_TYPE_ANALOGPAD;
                        break;
                }
            }
            break;

        case CMD_VIBRATION_TOGGLE:
            if (CurByte >= 2 && CurByte < CmdLen) {
                if (CurByte == g.PadState[CurPad].Vib0) {
                    buf[CurByte] = 0;
                }
                if (CurByte == g.PadState[CurPad].Vib1) {
                    buf[CurByte] = 1;
                }

                if (value == 0) {
                    g.PadState[CurPad].Vib0 = CurByte;
                    if ((g.PadState[CurPad].PadID & 0x0f) < (CurByte - 1) / 2) {
                        g.PadState[CurPad].PadID = (g.PadState[CurPad].PadID & 0xf0) + (CurByte - 1) / 2;
                    }
                } else if (value == 1) {
                    g.PadState[CurPad].Vib1 = CurByte;
                    if ((g.PadState[CurPad].PadID & 0x0f) < (CurByte - 1) / 2) {
                        g.PadState[CurPad].PadID = (g.PadState[CurPad].PadID & 0xf0) + (CurByte - 1) / 2;
                    }
                }
            }
            break;
    }

    if (CurByte >= CmdLen) return 0;
    return buf[CurByte++];
}

long PAD__init(long flags) {
    PAD__setMode(0, 0);
    PAD__setMode(1, 0);

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

