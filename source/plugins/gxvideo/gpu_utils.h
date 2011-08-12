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

#ifndef GPU_UTILS_H_
#define GPU_UTILS_H_

#include <stdint.h>

typedef struct {
	uint16_t rgb16;
} __attribute__((__packed__)) gxv_rgb16;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} __attribute__((__packed__)) gxv_rgb24;

/* for fast recast ^^ */
typedef union {
	uint8_t * u8;
	int8_t * s8;
	uint16_t * u16;
	int16_t * s16;
	uint32_t * u32;
	int32_t * s32;
	gxv_rgb16 * rgb16;
	gxv_rgb24 * rgb24;
} gxv_pointer_t;

typedef struct {
	int32_t x;
	int32_t y;
} gxv_point_t;

typedef struct {
	int16_t x;
	int16_t y;
} gxv_spoint_t;

typedef struct {
	int16_t x0;
	int16_t x1;
	int16_t y0;
	int16_t y1;
} gxv_rect_t;

typedef struct {
	gxv_point_t position;
	gxv_point_t mode;
	gxv_point_t DrawOffset;
	gxv_rect_t range;
} gxv_display_t;

typedef struct {
	gxv_rect_t Position;
} gxv_win_t;


#ifdef LIBXENON

#define GPUopen			PEOPS_GPUopen
#define GPUdisplayText		PEOPS_GPUdisplayText
#define GPUdisplayFlags		PEOPS_GPUdisplayFlags
#define GPUmakeSnapshot		PEOPS_GPUmakeSnapshot
#define GPUinit			PEOPS_GPUinit
#define GPUclose		PEOPS_GPUclose
#define GPUshutdown		PEOPS_GPUshutdown
#define GPUcursor		PEOPS_GPUcursor
#define GPUupdateLace		PEOPS_GPUupdateLace
#define GPUreadStatus		PEOPS_GPUreadStatus
#define GPUwriteStatus		PEOPS_GPUwriteStatus
#define GPUreadDataMem		PEOPS_GPUreadDataMem
#define GPUreadData		PEOPS_GPUreadData
#define GPUwriteDataMem		PEOPS_GPUwriteDataMem
#define GPUwriteData		PEOPS_GPUwriteData
#define GPUsetMode		PEOPS_GPUsetMode
#define GPUgetMode		PEOPS_GPUgetMode
#define GPUdmaChain		PEOPS_GPUdmaChain
#define GPUconfigure		PEOPS_GPUconfigure
#define GPUabout		PEOPS_GPUabout
#define GPUtest			PEOPS_GPUtest
#define GPUfreeze		PEOPS_GPUfreeze
#define GPUgetScreenPic		PEOPS_GPUgetScreenPic
#define GPUshowScreenPic	PEOPS_GPUshowScreenPic

#define PSEgetLibName           GPUPSEgetLibName
#define PSEgetLibType           GPUPSEgetLibType
#define PSEgetLibVersion        GPUPSEgetLibVersion


//new
#define GPUvBlank               PEOPS_GPUvBlank
#define GPUvisualVibration      PEOPS_GPUvisualVibration
#endif

#endif /* GPU_UTILS_H_ */
