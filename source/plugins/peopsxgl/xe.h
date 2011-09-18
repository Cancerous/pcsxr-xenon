#define XE_TEXF_POINT 0
#define XE_TEXF_LINEAR 1


void XeOrtho(int l, int r, int b, int t, int zn, int zf);
void InitGlSurface();

// viewport
void XeViewport(int left, int top, int right, int bottom);

// tex
void xeGfx_setTextureData(void * tex, void * buffer);
void XeTexSubImage(struct XenosSurface * surf, int srcbpp, int dstbpp, int xoffset, int yoffset, int width, int height, const void * buffer);

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
void XeBlendFunc(int src, int dst);

// texture 
void XeTexWrap(int uwrap, int vwrap);
void XeTexUseFiltering(int enabled);


void XeClear();

struct XenosVertexBuffer *XeGetVb();

// prim
typedef struct __attribute__((__packed__)) PsxVerticeFormats {
    float x, y, z, w;
    float u, v;
    unsigned int color;
    //float r,g,b,a;
} PsxVerticeFormats;


typedef struct XeColor{
    union {
        unsigned int color;
        struct{
            uint8_t a;
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
    };
} XeColor;

extern PsxVerticeFormats * PsxVertex;

void iXeDrawTri(PsxVerticeFormats * psxvertices);
void iXeDrawTri2(PsxVerticeFormats * psxvertices);
void iXeDrawQuad(PsxVerticeFormats * psxvertices);
void LockVb();
void UnlockVb();


void XeOrtho(int l, int r, int b, int t, int zn, int zf);
void CloseDisplay();
void XeResetStates();
//void XeClear(uint32_t color);
void DoBufferSwap();


void XeSetCombinerC();
void XeSetCombinerF();
void XeSetCombinerG();


struct PSXUV{
    float top;
    float bottom;
    float left;
    float right;
};
extern struct PSXUV gPsxUV;//offset


unsigned long ulInitDisplay();