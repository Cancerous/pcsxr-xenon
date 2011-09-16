#if 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <console/console.h>

#include <ppc/timebase.h>
#include <time/time.h>
#include <time.h>

#include <debug.h>

#include "xe.h"
#include "gl_api.h"

extern struct XenosDevice *xe;

#define MAX_VERTEX_COUNT 16384
#define MAX_INDICE_COUNT 65536

uint32_t gl_color = 0;
float gl_u = 0;
float gl_v = 0;
int gl_mode = 0;
int off_v = 0;

static struct XenosVertexBuffer *vb = NULL;
static struct XenosIndexBuffer *ib_tri = NULL;
static struct XenosIndexBuffer *ib_quads = NULL;

static uint16_t prim_tri_strip[] = {0, 1, 2, 2, 1, 3}; // GL_TRIANGLE_STRIP
static uint16_t prim_quads[] = {1, 0, 2, 2, 0, 3}; // GL_QUADS

int indiceCount=0;
int vertexCount=0;

typedef struct __attribute__((__packed__)) glVerticeFormats {
    float x, y, z, w;
    float u, v;
    unsigned int color;
    //float r,g,b,a;
} glVerticeFormats;

glVerticeFormats * currentVertex = NULL;


static void glLockVb(int size) {
    // se deplace dans le vb
    Xe_SetStreamSource(xe, 0, vb, vertexCount, 4);
    currentVertex = (glVerticeFormats *) Xe_VB_Lock(xe, vb, vertexCount, size * sizeof (glVerticeFormats), XE_LOCK_WRITE);
}


static void glUnlockVb() {
    if (vb->lock.start)
        Xe_VB_Unlock(xe, vb);
    else
        printf("glUnlockVb\r\n");
}


static void XeGlPrepare(int size) {
    glLockVb(size);
}

// appeler apres chaques frames
void glReset(){
    vertexCount = 0;
}


void glInit() {
    vb = Xe_CreateVertexBuffer(xe, MAX_VERTEX_COUNT * sizeof (glVerticeFormats));
    
    ib_tri = Xe_CreateIndexBuffer(xe, 6 * sizeof (uint16_t), XE_FMT_INDEX16);
    ib_quads = Xe_CreateIndexBuffer(xe, 6 * sizeof (uint16_t), XE_FMT_INDEX16);
    
    // lock
    void *vt = Xe_IB_Lock(xe,ib_tri,0,6 * sizeof (uint16_t),XE_LOCK_WRITE);
    void *vq = Xe_IB_Lock(xe,ib_quads,0,6 * sizeof (uint16_t),XE_LOCK_WRITE);
    
    memcpy(vt,prim_tri_strip,6 * sizeof (uint16_t));
    memcpy(vq,prim_quads,6 * sizeof (uint16_t));
    
    Xe_IB_Unlock(xe,ib_tri);
    Xe_IB_Unlock(xe,ib_quads);
    
    glReset();
}

int glGetModelSize() {
    switch (gl_mode) {
        case GL_TRIANGLE_STRIP:
        {
            return 4;
        }
        case GL_TRIANGLES:
        {
            return 3;
        }
        case GL_QUADS:
        {
            return 4;
        }
    }
}

void glBegin(int mode) {
    XeGlPrepare(glGetModelSize());
    gl_mode = mode;
    off_v = 0;
}

void glEnd() {
    glUnlockVb();
    switch (gl_mode) {
        case GL_TRIANGLE_STRIP:
        {
            Xe_SetIndices(xe,ib_tri);
            //Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLESTRIP, 0, 2);
            Xe_DrawIndexedPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 0, 6, 0, 2);

            break;
        }
        case GL_TRIANGLES:
        {
            Xe_SetIndices(xe,NULL);
            Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLEFAN, 0, 1);
            break;
        }
        case GL_QUADS:
        {

            Xe_SetIndices(xe,ib_quads);
            Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLEFAN, 0, 2);
            Xe_DrawIndexedPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 0, 6, 0, 2);

            break;
        }
    }
    
    // +++
    vertexCount += off_v * sizeof (glVerticeFormats);
    
    //glUnlockIb();
    
    // Reset color
   // gl_color = 0;
}

void glTexCoord2fv(float * st) {
    gl_u = st[0];
    gl_v = st[1];
}

//xe_display.c
extern float screen[2];

void glVertex3fv(float * v) {
    currentVertex[off_v].x = ((v[0] / screen[0])*2.f) - 1.0f;
    currentVertex[off_v].y = ((v[1] / screen[1])*2.f) - 1.0f;

    currentVertex[off_v].z = -v[2];
    currentVertex[off_v].w = 1.f;
    currentVertex[off_v].u = gl_u;
    currentVertex[off_v].v = gl_v;
    currentVertex[off_v].color = gl_color;
    off_v++;
}

void glColor4ubv(u8 *v) {
    uint32_t c = *(uint32_t*) v;
    gl_color = c;
}
#endif