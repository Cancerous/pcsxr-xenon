/* 
 * File:   XeRenderer.h
 * Author: cc
 *
 * Created on 11 octobre 2011, 16:51
 */

#ifndef GPURENDERER_H
#define	GPURENDERER_H

#include "gpu_types.h"

#define XE_TEXF_POINT 0
#define XE_TEXF_LINEAR 1

typedef struct XeColor {

    union {
        unsigned int color;

        struct {
            uint8_t a;
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint8_t s[4];
    };
} XeColor;

enum GpuPrimTypes {
    PRIM_TRIANGLE,
    PRIM_TRIANGLE_STRIP,
    PRIM_QUAD
};

enum GlExt {
    RGB_SCALE_EXT = 0,

};

class GpuRenderer {
private:

    /**
     * packed to 32bytes
     */
    typedef struct PACKED verticeformats {
        float x, y, z;
        float u, v;
        float u2, v2;
        uint32_t color;
    }
    verticeformats;

    verticeformats * pCurrentVertex;
    verticeformats * pFirstVertex;

    uint16_t * pCurrentIndice;
    uint16_t * pFirstIndice;

    GpuVB *pVb;
    GpuIB *pIb;

    GpuTex * pRenderSurface;

    GpuVS * g_pVertexShader;
    GpuPS * g_pPixelShader;

    GpuPS * g_pPixelShaderC;
    GpuPS * g_pPixelShaderF;
    GpuPS * g_pPixelShaderG;
    
    /**
     * Post process 
     */
    GpuVB *pVbPost;
    GpuVS * g_pVertexShaderPost;
    GpuPS * g_pPixelShaderPost;
    GpuTex * pPostRenderSurface;

    /**
     * Render states
     */
    struct RenderStates {
        // texture
        struct XenosSurface * surface;
        // clear color
        uint32_t clearcolor;
        // z / depth
        int32_t z_func;
        uint32_t z_write;
        uint32_t z_enable;

        // fillmode
        uint32_t fillmode_front;
        uint32_t fillmode_back;

        // blend
        int32_t blend_op;
        int32_t blend_src;
        int32_t blend_dst;

        // alpha blend
        int32_t alpha_blend_op;
        int32_t alpha_blend_src;
        int32_t alpha_blend_dst;

        // cull mode
        uint32_t cullmode;

        // alpha test
        uint32_t alpha_test_enable;
        int32_t alpha_test_func;
        float alpha_test_ref;

        // stencil
        uint32_t stencil_enable;
        uint32_t stencil_func;
        uint32_t stencil_op;
        uint32_t stencil_ref;
        uint32_t stencil_mask;
        uint32_t stencil_writemask;
    };

    RenderStates m_RenderStates;

public:
    int b_StatesChanged;


    /**
     * states changed
     */
    void StatesChanged();
private:
    void UpdatesStates();

    void SubmitVertices();

    void InitStates();
    void InitXe();
    
    /**
     * Post process
     */
    void InitPostProcess();
    void RenderPostProcess();
    void BeginPostProcess();
    void EndPostProcess();

    void Lock();
    void Unlock();
public:
    

    /**
     * texture
     */
    void SetTexture(GpuTex * s);
    void EnableTexture();
    void DisableTexture();

    /**
     * Clear
     */
    void Clear(uint32_t flags);
    // void ClearColor(float a, float r, float g, float b);
    void ClearColor(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
    // fillmode

    /**
     * blend
     */
    void DisableBlend();
    void EnableBlend();
    void SetBlendFunc(int src, int dst);
    void SetBlendOp(int op);

    /**
     * Alpha blend
     */
    void DisableAlphaBlend();
    void EnableAlphaBlend();
    void SetAlphaBlendFunc(int src, int dst);
    void SetAlphaBlendOp(int op);

    // cull mode

    /**
     * Alpha test
     */
    void SetAlphaFunc(int func, float ref);
    void EnableAlphaTest();
    void DisableAlphaTest();

    // stencil

    /**
     * z / depth
     */
    void EnableDepthTest();
    void DisableDepthTest();
    void DepthFunc(int func);

    // scissor
    void DisableScissor();
    void EnableScissor();

    /**
     * Init
     */
    void Init();
    void Close();

    /**
     * Render
     */
    void Render();

    /**
     * Gl like func
     */
    int m_PrimType;
    float textcoord_u;
    float textcoord_v;
    uint32_t m_primColor;

    void primBegin(int primType);
    void primEnd();
    void primTexCoord(float * st);
    void primVertex(float * v);
    void primColor(u8 *v);

    void TextEnv(int f, int v) {
        // nothing yet
    }

    void TextEnv(int f, float v) {
        switch (f) {
            case RGB_SCALE_EXT:
                break;
        }
    }

    int verticesCount();
    int indicesCount();
    int prevIndicesCount;
    int prevVerticesCount;

    // viewport
    void SetViewPort(int left, int top, int right, int bottom);
    void SetOrtho(float l, float r, float b, float t, float zn, float zf);

    // textures
    void DestroyTexture(GpuTex *surf);
    GpuTex * CreateTexture(unsigned int width, unsigned int height, int format);

    void SetTextureFiltering(int filtering_mode) {
#ifndef WIN32
        if (m_RenderStates.surface)
            m_RenderStates.surface->use_filtering = filtering_mode;
#endif
    }
    
    void NextVertice();
    void NextIndice();
};

extern GpuRenderer gpuRenderer;

// tex
void xeGfx_setTextureData(void * tex, void * buffer);
void XeTexSubImage(GpuTex * surf, int srcbpp, int dstbpp, int xoffset, int yoffset, int width, int height, const void * buffer);

static inline void DoBufferSwap() {
    gpuRenderer.Render();
}

#endif	/* GPURENDERER_H */

