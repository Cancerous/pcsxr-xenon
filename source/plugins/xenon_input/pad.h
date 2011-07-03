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

#ifndef PAD_H_
#define PAD_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "psemu_plugin_defs.h"

enum {
	DKEY_SELECT = 0,
	DKEY_L3,
	DKEY_R3,
	DKEY_START,
	DKEY_UP,
	DKEY_RIGHT,
	DKEY_DOWN,
	DKEY_LEFT,
	DKEY_L2,
	DKEY_R2,
	DKEY_L1,
	DKEY_R1,
	DKEY_TRIANGLE,
	DKEY_CIRCLE,
	DKEY_CROSS,
	DKEY_SQUARE,
	DKEY_ANALOG,

	DKEY_TOTAL
};

enum {
	ANALOG_LEFT = 0,
	ANALOG_RIGHT,

	ANALOG_TOTAL
};

enum { NONE = 0, AXIS, HAT, BUTTON };

typedef struct tagKeyDef {
	uint8_t			JoyEvType;
	union {
		int16_t		d;
		int16_t		Axis;   // positive=axis+, negative=axis-, abs(Axis)-1=axis index
		uint16_t	Hat;	// 8-bit for hat number, 8-bit for direction
		uint16_t	Button; // button number
	} J;
	uint16_t		Key;
} KEYDEF;

enum { ANALOG_XP = 0, ANALOG_XM, ANALOG_YP, ANALOG_YM };

typedef struct tagPadDef {
	int8_t			DevNum;
	uint16_t		Type;
	KEYDEF			KeyDef[DKEY_TOTAL];
	KEYDEF			AnalogDef[ANALOG_TOTAL][4];
} PADDEF;

typedef struct tagConfig {
	uint8_t			Threaded;
    uint8_t         HideCursor;
	PADDEF			PadDef[2];
} CONFIG;


enum {
	CMD_READ_DATA_AND_VIBRATE = 0x42,
	CMD_CONFIG_MODE = 0x43,
	CMD_SET_MODE_AND_LOCK = 0x44,
	CMD_QUERY_MODEL_AND_MODE = 0x45,
	CMD_QUERY_ACT = 0x46, // ??
	CMD_QUERY_COMB = 0x47, // ??
	CMD_QUERY_MODE = 0x4C, // QUERY_MODE ??
	CMD_VIBRATION_TOGGLE = 0x4D,
};

// cfg.c functions...
void LoadPADConfig();
void SavePADConfig();

// sdljoy.c functions...
void InitSDLJoy();
void DestroySDLJoy();
void CheckJoy();

// xkb.c functions...
void InitKeyboard();
void DestroyKeyboard();
void CheckKeyboard();

// analog.c functions...
void InitAnalog();
void CheckAnalog();
int AnalogKeyPressed(uint16_t Key);
int AnalogKeyReleased(uint16_t Key);

// pad.c functions...
char *PSEgetLibName(void);
uint32_t PSEgetLibType(void);
uint32_t PSEgetLibVersion(void);
long PAD__init(long flags);
long PAD__shutdown(void);
long PAD__open(unsigned long *Disp);
long PAD__close(void);
long PAD__query(void);
unsigned char PAD__startPoll(int pad);
unsigned char PAD__poll(unsigned char value);
long PAD__readPort1(PadDataS *pad);
long PAD__readPort2(PadDataS *pad);
long PAD__keypressed(void);
long PAD__configure(void);
void PAD__about(void);
long PAD__test(void);

#ifdef __cplusplus
}
#endif

#endif
