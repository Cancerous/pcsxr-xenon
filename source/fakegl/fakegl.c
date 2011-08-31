#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fakegl.h"
#include <xenos/xe.h>

#define D3DTEXF_LINEAR 0
#define D3D_MAX_TMUS	8

void Sys_Error(char * str) {
    printf("%s\r\n", str);
    while (1) {

    }
}

typedef struct xenos_texture_s {
    // OpenGL number for glBindTexture/glGenTextures/glDeleteTextures/etc
    int glnum;

    // format the texture was uploaded as; 1 or 4 bytes.
    // glTexSubImage2D needs this.
    GLenum internalformat;

    // parameters
    uint32_t addressu;
    uint32_t addressv;
    uint32_t anisotropy;
    uint32_t magfilter;
    uint32_t minfilter;
    uint32_t mipfilter;

    // the texture image itself
    struct XenosSurface * teximg;

    struct xenos_texture_s *next;
} xenos_texture_t;

typedef struct gl_tmustate_s {
    xenos_texture_t *boundtexture;
    BOOL enabled;
    // BOOL combine;

    // gl_combine_t combstate;

    // these come from glTexEnv and are properties of the TMU; other D3DTEXTURESTAGESTATETYPEs 
    // come from glTexParameter and are properties of the individual texture.
    uint32_t colorop;
    uint32_t colorarg1;
    uint32_t colorarg2;
    uint32_t alphaop;
    uint32_t alphaarg1;
    uint32_t alphaarg2;
    uint32_t texcoordindex;

    BOOL texenvdirty;
    BOOL texparamdirty;
} gl_tmustate_t;


struct XenosDevice *xe = NULL;
int xenos_TextureExtensionNumber = 1;
gl_tmustate_t d3d_TMUs[D3D_MAX_TMUS];

int d3d_CurrentTMU = 0;


xenos_texture_t *xenos_Textures = NULL;

void xe_initTexture(xenos_texture_t *tex) {
    tex->glnum = 0;

    // note - we can't set the ->next pointer to NULL here as this texture may be a 
    // member of the linked list that has just become free...
    tex->addressu = XE_TEXADDR_WRAP;
    tex->addressv = XE_TEXADDR_WRAP;
    tex->magfilter = D3DTEXF_LINEAR;
    tex->minfilter = D3DTEXF_LINEAR;
    tex->mipfilter = D3DTEXF_LINEAR;
    tex->anisotropy = 1;

    Xe_DestroyTexture(xe, tex->teximg);
}

xenos_texture_t *xe_AllocTexture(void) {
    xenos_texture_t *tex;

    // find a free texture
    for (tex = xenos_Textures; tex; tex = tex->next) {
        // if either of these are 0 (or NULL) we just reuse it
        if (!tex->teximg) {
            xe_initTexture(tex);
            return tex;
        }

        if (!tex->glnum) {
            xe_initTexture(tex);
            return tex;
        }
    }

    // nothing to reuse so create a new one
    // clear to 0 is required so that D3D_SAFE_RELEASE is valid
    tex = (xenos_texture_t *) malloc(sizeof (xenos_texture_t));
    memset(tex, 0, sizeof (xenos_texture_t));
    xe_initTexture(tex);

    // link in
    tex->next = xenos_Textures;
    xenos_Textures = tex;

    // return the new one
    return tex;
}

void D3D_FillTextureLevel(struct XenosSurface * texture, int level, GLint internalformat, int width, int height, GLint format, const void *pixels) {

};

void xe_ReleaseTextures(void) {
    xenos_texture_t *tex;

    // explicitly NULL all textures and force texparams to dirty
    for (tex = xenos_Textures; tex; tex = tex->next) {
        Xe_DestroyTexture(xe, tex->teximg);
    }
}

void glGenTextures(GLsizei n, GLuint *textures) {
    int i;

    for (i = 0; i < n; i++) {
        // either take a free slot or alloc a new one
        xenos_texture_t *tex = xe_AllocTexture();

        tex->glnum = textures[i] = xenos_TextureExtensionNumber;
        xenos_TextureExtensionNumber++;
    }
}

void glDeleteTextures(GLsizei n, const GLuint *textures) {
    int i;
    xenos_texture_t *tex;

    for (tex = xenos_Textures; tex; tex = tex->next) {
        for (i = 0; i < n; i++) {
            if (tex->glnum == textures[i]) {
                xe_initTexture(tex);
                break;
            }
        }
    }
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {

}

void glTexParameteri(GLenum target, GLenum pname, GLint param) {
    if (target != GL_TEXTURE_2D) return;
    if (!d3d_TMUs[d3d_CurrentTMU].boundtexture) return;

    glTexParameterf(target, pname, param);
}

void glBindTexture(GLenum target, GLuint texture) {
    xenos_texture_t *tex;

    if (target != GL_TEXTURE_2D) return;

    // use no texture
    if (texture == 0) {
        d3d_TMUs[d3d_CurrentTMU].boundtexture = NULL;
        return;
    }

    // initially nothing
    d3d_TMUs[d3d_CurrentTMU].boundtexture = NULL;

    // find a texture
    // these searches could be optimised with another lookup table, but we don't know how big it would need to be
    for (tex = xenos_Textures; tex; tex = tex->next) {
        if (tex->glnum == texture) {
            d3d_TMUs[d3d_CurrentTMU].boundtexture = tex;
            break;
        }
    }

    // did we find it?
    if (!d3d_TMUs[d3d_CurrentTMU].boundtexture) {
        // nope, so fill in a new one (this will make it work with texture_extension_number)
        // (i don't know if the spec formally allows this but id seem to have gotten away with it...)
        d3d_TMUs[d3d_CurrentTMU].boundtexture = xe_AllocTexture();

        // reserve this slot
        d3d_TMUs[d3d_CurrentTMU].boundtexture->glnum = texture;

        // ensure that it won't be reused
        if (texture > xenos_TextureExtensionNumber) xenos_TextureExtensionNumber = texture;
    }

    // this should never happen
    if (!d3d_TMUs[d3d_CurrentTMU].boundtexture)
        Sys_Error("glBindTexture: out of textures!!!");

    // dirty the params
    d3d_TMUs[d3d_CurrentTMU].texparamdirty = TRUE;
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    uint32_t texformat = XE_FMT_ARGB;

    // validate format
    switch (internalformat) {
        case 1:
        case GL_LUMINANCE:
            texformat = XE_FMT_8;
            break;

        case 3:
        case GL_RGB:
            texformat = XE_FMT_ARGB;
            break;

        case 4:
        case GL_RGBA:
            texformat = XE_FMT_ARGB;
            break;

        default:
            Sys_Error("invalid texture internal format");
    }

    if (type != GL_UNSIGNED_BYTE) Sys_Error("glTexImage2D: Unrecognised pixel format");

    // ensure that it's valid to create textures
    if (target != GL_TEXTURE_2D) return;
    if (!d3d_TMUs[d3d_CurrentTMU].boundtexture) return;

    if (level == 0) {
        // in THEORY an OpenGL texture can have different formats and internal formats for each miplevel
        // in practice I don't think anyone uses it...
        d3d_TMUs[d3d_CurrentTMU].boundtexture->internalformat = internalformat;

        // overwrite an existing texture - just release it so that we can recreate
        //D3D_SAFE_RELEASE(d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg);
        Xe_DestroyTexture(xe, d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg);

        // create the texture from the data
        // initially just create a single mipmap level (assume that the texture isn't mipped)
        /*
                hr = IDirect3DDevice8_CreateTexture
                        (
                        d3d_Device,
                        width,
                        height,
                        1,
                        0,
                        texformat,
                        D3DPOOL_MANAGED,
                        &d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg
                        );
         */

        Xe_CreateTexture(xe, width, height, level, texformat, 0);

        if (d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg == NULL)
            Sys_Error("glTexImage2D: unable to create a texture");

        D3D_FillTextureLevel(d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg, 0, internalformat, width, height, format, pixels);
    } else {
        // the texture has already been created so no need to do any more
        D3D_FillTextureLevel(d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg, level, internalformat, width, height, format, pixels);
    }
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {
    int x, y;
    int srcbytes = 0;
    int dstbytes = 0;
    unsigned char *srcdata;
    unsigned char *dstdata;
    unsigned char *pBits;
    //D3DLOCKED_RECT lockrect;

    if (format == 1 || format == GL_LUMINANCE)
        srcbytes = 1;
    else if (format == 3 || format == GL_RGB)
        srcbytes = 3;
    else if (format == 4 || format == GL_RGBA)
        srcbytes = 4;
    else Sys_Error("D3D_FillTextureLevel: illegal format");

    // d3d doesn't have an internal RGB only format
    // (neither do most OpenGL implementations, they just let you specify it as RGB but expand internally to 4 component)
    if (d3d_TMUs[d3d_CurrentTMU].boundtexture->internalformat == 1 || d3d_TMUs[d3d_CurrentTMU].boundtexture->internalformat == GL_LUMINANCE)
        dstbytes = 1;
    else if (d3d_TMUs[d3d_CurrentTMU].boundtexture->internalformat == 3 || d3d_TMUs[d3d_CurrentTMU].boundtexture->internalformat == GL_RGB)
        dstbytes = 4;
    else if (d3d_TMUs[d3d_CurrentTMU].boundtexture->internalformat == 4 || d3d_TMUs[d3d_CurrentTMU].boundtexture->internalformat == GL_RGBA)
        dstbytes = 4;
    else Sys_Error("D3D_FillTextureLevel: illegal internalformat");

    //IDirect3DTexture8_LockRect(d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg, level, &lockrect, NULL, 0);

    pBits = (unsigned char*)Xe_Surface_LockRect(xe, d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg, 0,0,0,0,XE_LOCK_WRITE);
    
    srcdata = (unsigned char *) pixels;
    dstdata = pBits;
    dstdata += (yoffset * width + xoffset) * dstbytes;

    for (y = yoffset; y < (yoffset + height); y++) {
        for (x = xoffset; x < (xoffset + width); x++) {
            if (srcbytes == 1) {
                if (dstbytes == 1)
                    dstdata[0] = srcdata[0];
                else if (dstbytes == 4) {
                    dstdata[0] = srcdata[0];
                    dstdata[1] = srcdata[0];
                    dstdata[2] = srcdata[0];
                    dstdata[3] = srcdata[0];
                }
            } else if (srcbytes == 3) {
                if (dstbytes == 1)
                    dstdata[0] = ((int) srcdata[0] + (int) srcdata[1] + (int) srcdata[2]) / 3;
                else if (dstbytes == 4) {
                    dstdata[0] = srcdata[2];
                    dstdata[1] = srcdata[1];
                    dstdata[2] = srcdata[0];
                    dstdata[3] = 255;
                }
            } else if (srcbytes == 4) {
                if (dstbytes == 1)
                    dstdata[0] = ((int) srcdata[0] + (int) srcdata[1] + (int) srcdata[2]) / 3;
                else if (dstbytes == 4) {
                    dstdata[0] = srcdata[2];
                    dstdata[1] = srcdata[1];
                    dstdata[2] = srcdata[0];
                    dstdata[3] = srcdata[3];
                }
            }

            // advance
            srcdata += srcbytes;
            dstdata += dstbytes;
        }
    }

    Xe_Surface_Unlock(xe,d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg);
    //IDirect3DTexture8_UnlockRect(d3d_TMUs[d3d_CurrentTMU].boundtexture->teximg, level);
}