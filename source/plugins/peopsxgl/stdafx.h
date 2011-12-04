/***************************************************************************
                          stdafx.h  -  description
                             -------------------
    begin                : Sun Mar 08 2009
    copyright            : (C) 1999-2009 by Pete Bernert
    web                  : www.pbernert.com   
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifdef LIBXENON
#include <sys/stat.h>
#include <sys/time.h>
//#include "gl_ext.h"
#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <console/console.h>

#include <ppc/timebase.h>
#include <time/time.h>
#include <time.h>
#else
#ifdef WIN32
#include <Windows.h>
#endif
#endif


#include "swap.h"

#include <math.h> 

#define CALLBACK /* */

#define EXTERN extern "C"

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;

#define SHADETEXBIT(x) ((x>>24) & 0x1)
#define SEMITRANSBIT(x) ((x>>25) & 0x1)



#ifdef LIBXENON

#define GPUopen			HW_GPUopen
#define GPUdisplayText		HW_GPUdisplayText
#define GPUdisplayFlags		HW_GPUdisplayFlags
#define GPUmakeSnapshot		HW_GPUmakeSnapshot
#define GPUinit			HW_GPUinit
#define GPUclose		HW_GPUclose
#define GPUshutdown		HW_GPUshutdown
#define GPUcursor		HW_GPUcursor
#define GPUupdateLace		HW_GPUupdateLace
#define GPUreadStatus		HW_GPUreadStatus
#define GPUwriteStatus		HW_GPUwriteStatus
#define GPUreadDataMem		HW_GPUreadDataMem
#define GPUreadData		HW_GPUreadData
#define GPUwriteDataMem		HW_GPUwriteDataMem
#define GPUwriteData		HW_GPUwriteData
#define GPUsetMode		HW_GPUsetMode
#define GPUgetMode		HW_GPUgetMode
#define GPUdmaChain		HW_GPUdmaChain
#define GPUconfigure		HW_GPUconfigure
#define GPUabout		HW_GPUabout
#define GPUtest			HW_GPUtest
#define GPUfreeze		HW_GPUfreeze
#define GPUgetScreenPic		HW_GPUgetScreenPic
#define GPUshowScreenPic	HW_GPUshowScreenPic

#define PSEgetLibName           HW_GPUPSEgetLibName
#define PSEgetLibType           HW_GPUPSEgetLibType
#define PSEgetLibVersion        HW_GPUPSEgetLibVersion

#define GPUsetfix               HW_GPUsetfix

#define GPUaddVertex            HW_GPUaddVertex

//new
#define GPUvBlank               HW_GPUvBlank
#define GPUvisualVibration      HW_GPUvisualVibration
#endif


#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}