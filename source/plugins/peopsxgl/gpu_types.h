#pragma once

#ifndef WIN32
#include <xenos/xe.h>
#define FMT_A8R8G8B8 (XE_FMT_8888|XE_FMT_ARGB)
#define PACKED __attribute__((__packed__))
typedef struct XenosVertexBuffer GpuVB;
typedef struct XenosIndexBuffer GpuIB;
typedef struct XenosSurface GpuTex;
typedef struct XenosShader GpuPS;
typedef struct XenosShader GpuVS;
#else
#include <unistd.h>
#include <d3dx9.h>
#define PACKED
typedef IDirect3DVertexBuffer9 GpuVB;
typedef IDirect3DIndexBuffer9 GpuIB;
typedef IDirect3DTexture9 GpuTex;
typedef IDirect3DPixelShader9 GpuPS;
typedef IDirect3DVertexShader9 GpuVS;
#define FMT_A8R8G8B8 (D3DFMT_A8R8G8B8)
#endif


// xe enum to directx enum
#ifdef WIN32

#define XE_CLEAR_DS	D3DCLEAR_STENCIL
#define XE_CLEAR_COLOR D3DCLEAR_TARGET
#define XE_TEXADDR_CLAMP D3DTADDRESS_CLAMP         
#define XE_BLENDOP_ADD D3DBLENDOP_ADD  
#define XE_BLENDOP_REVSUBTRACT D3DBLENDOP_REVSUBTRACT

#define XE_BLEND_ZERO                 D3DBLEND_ZERO 
#define XE_BLEND_ONE                  D3DBLEND_ONE
#define XE_BLEND_SRCCOLOR             D3DBLEND_SRCCOLOR
#define XE_BLEND_INVSRCCOLOR          D3DBLEND_INVSRCCOLOR
#define XE_BLEND_SRCALPHA             D3DBLEND_SRCALPHA
#define XE_BLEND_INVSRCALPHA          D3DBLEND_INVSRCALPHA
#define XE_BLEND_DESTCOLOR            D3DBLEND_DESTCOLOR
#define XE_BLEND_INVDESTCOLOR         D3DBLEND_INVDESTCOLOR
#define XE_BLEND_DESTALPHA           D3DBLEND_DESTALPHA
#define XE_BLEND_INVDESTALPHA        D3DBLEND_INVDESTALPHA
#define XE_BLEND_BLENDFACTOR         D3DBLEND_BLENDFACTOR
#define XE_BLEND_INVBLENDFACTOR      D3DBLEND_INVBLENDFACTOR
//#define XE_BLEND_CONSTANTALPHA       14
//#define XE_BLEND_INVCONSTANTALPHA    15
#define XE_BLEND_SRCALPHASAT         D3DBLEND_SRCALPHASAT

#define XE_CMP_NEVER		D3DCMP_NEVER
#define XE_CMP_LESS			D3DCMP_LESS
#define XE_CMP_EQUAL		D3DCMP_EQUAL
#define XE_CMP_LESSEQUAL	D3DCMP_LESSEQUAL
#define XE_CMP_GREATER		D3DCMP_GREATER
#define XE_CMP_NOTEQUAL		D3DCMP_NOTEQUAL
#define XE_CMP_GREATEREQUAL D3DCMP_GREATEREQUAL
#define XE_CMP_ALWAYS		D3DCMP_ALWAYS

#define XE_FMT_ARGB			D3DFMT_A8R8G8B8
#define XE_FMT_5551			D3DFMT_A1R5G5B5

#define XE_FMT_8888			0
#define XE_FMT_BGRA			0

#define bswap_32 _byteswap_ulong
#endif