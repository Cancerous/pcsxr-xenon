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
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h> 

#define CALLBACK /* */


//#include "gl_ext.h"
#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <console/console.h>

#include <ppc/timebase.h>
#include <time/time.h>
#include <time.h>

#include "swap.h"

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


#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}