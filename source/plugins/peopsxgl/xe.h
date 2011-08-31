void XeOrtho(int l, int r, int b, int t, int zn, int zf);
void InitGlSurface();

// viewport
void XeViewport(int left, int top, int right, int bottom);

// tex
void XeTexImage(int internalformat, int width, int height, int format, const void *pixels);
void XeSetTexture();
void XeBindTexture(int i);
struct XenosSurface * XeGetTextureInSlot();
void XeDeleteTextures(int nbr, int * id) ;
void XeGenTextures(int nbr, int * id);


// scissor
void XeDisableScissor();
void XeEnableScissor();

// texture
void XeDisableTexture();
void XeEnableTexture();

// blend
void XeEnableBlend();
void XeDisableBlend();
void XeAlphaFunc(int func, float ref);

// texture 
void XeTexWrap(int uwrap, int vwrap);
void XeTexUseFiltering(int enabled);


// prim
typedef struct PsxVerticeFormats {
    float x, y, z, w;
    float u, v;
    //unsigned int color;
    float r, g, b, a;
} PsxVerticeFormats;

void iXeDrawTri(PsxVerticeFormats * psxvertices);
void iXeDrawTri2(PsxVerticeFormats * psxvertices);
void iXeDrawQuad(PsxVerticeFormats * psxvertices);

void XeOrtho(int l, int r, int b, int t, int zn, int zf);
void CloseDisplay();
void XeResetStates();
void XeClear(uint32_t color);
void DoBufferSwap();


struct PSXUV{
    float top;
    float bottom;
    float left;
    float right;
};
extern struct PSXUV gPsxUV;//offset