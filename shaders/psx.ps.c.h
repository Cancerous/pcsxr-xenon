#if 0
//
// Generated by 2.0.20353.0
//
//   fxc /Fh psx.ps.c.h /Tps_3_0 ps_psx.hlsl /EpsC
//
// Shader type: pixel 

xps_3_0
config AutoSerialize=false
config AutoResource=false
config PsMaxReg=1
// PsExportColorCount=1
// PsSampleControl=both

dcl_texcoord r0.xy
dcl_color_centroid r1


    alloc colors
    exece
    mov oC0, r1

// PDB hint 00000000-00000000-00000000

#endif

// This microcode is in native DWORD byte order.

const DWORD g_xps_psC[] =
{
    0x102a1100, 0x00000080, 0x00000024, 0x00000000, 0x00000024, 0x00000000, 
    0x00000058, 0x00000000, 0x00000000, 0x00000030, 0x0000001c, 0x00000023, 
    0xffff0300, 0x00000000, 0x00000000, 0x00000000, 0x0000001c, 0x70735f33, 
    0x5f300032, 0x2e302e32, 0x30333533, 0x2e3000ab, 0x00000000, 0x00000024, 
    0x10000100, 0x00000008, 0x00000000, 0x00001842, 0x00010003, 0x00000001, 
    0x00003050, 0x0000f1a0, 0x00000000, 0x1001c400, 0x22000000, 0xc80f8000, 
    0x00000000, 0xe2010100, 0x00000000, 0x00000000, 0x00000000
};
