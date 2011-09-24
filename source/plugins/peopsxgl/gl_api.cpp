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

#define MAX_VERTEX_COUNT 65536
#define MAX_INDICE_COUNT 65536


typedef union 
{
    uint32_t u;
    float f;
} gl_color_t;

gl_color_t gl_color;
float gl_u = 0;
float gl_v = 0;
int gl_mode = 0;
int off_v = 0;

static struct XenosVertexBuffer *vb = NULL;

static struct XenosVertexBuffer *vb_3 = NULL;
static struct XenosVertexBuffer *vb_4 = NULL;
static struct XenosVertexBuffer *vb_3s = NULL;

static struct XenosIndexBuffer *ib_tri = NULL;
static struct XenosIndexBuffer *ib_quads = NULL;

static uint16_t prim_tri_strip[] = {0, 1, 2, 2, 1, 3}; // GL_TRIANGLE_STRIP
static uint16_t prim_quads[] = {0, 1, 2, 3, 0, 3}; // GL_QUADS // not used

int indiceCount=0;
//int vertexCount=0;


typedef struct __attribute__((__packed__)) glVerticeFormats {
    float x, y, z;
   // float u, v;
    float color;
    //float r,g,b,a;
} glVerticeFormats;

//glVerticeFormats * currentVertex = NULL;
float * currentVertex = NULL;


static int texture_combiner_enabled = 0;
static int color_combiner_enabled = 0;


int vertexCount_3=0;
int vertexCount_3s=0;
int vertexCount_4=0;

int vertexCountZero(){
    vertexCount_3=0;
    vertexCount_3s=0;
    vertexCount_4=0;
}

int vertexCount(){
    if(vb){
        if(vb==vb_3)
            return vertexCount_3;
        if(vb==vb_3s)
            return vertexCount_3s;
        if(vb==vb_4)
            return vertexCount_4;
    }
    return 0;
}

void vertexCountAdd(int nb){
    if(vb){
        if(vb==vb_3)
            vertexCount_3+=nb;
        if(vb==vb_3s)
            vertexCount_3s+=nb;
        if(vb==vb_4)
            vertexCount_4+=nb;
    }
}

//------------------------------------------------------------------------------
// Changes gl states
//------------------------------------------------------------------------------
void XeSetTexture(struct XenosSurface * s ){
    Xe_SetTexture(xe, 0, s);
}

void XeEnableTexture() {
    texture_combiner_enabled = 1;
}

void XeDisableTexture() {
   texture_combiner_enabled = 0;
   Xe_SetTexture(xe, 0, NULL);
}


//------------------------------------------------------------------------------
// Lock Unlock Prepare VB
//------------------------------------------------------------------------------
static void glLockVb(int count) {
//    if(vertexCount()>2000)
//        vertexCountZero();
    // se deplace dans le vb
    Xe_SetStreamSource(xe, 0, vb, vertexCount(), 4);
   // currentVertex = (glVerticeFormats *) Xe_VB_Lock(xe, vb, vertexCount(), count * sizeof(glVerticeFormats), XE_LOCK_WRITE);
     currentVertex = (float *) Xe_VB_Lock(xe, vb, vertexCount(), count * sizeof(glVerticeFormats), XE_LOCK_WRITE);
}


static void glUnlockVb() {
    if (vb->lock.start)
        Xe_VB_Unlock(xe, vb);
    else
        printf("glUnlockVb - vb not locked\r\n");
}


static void glPrepare(int count) {
    glLockVb(count);
       
    // Zero it
    memset(currentVertex,0,count * sizeof(glVerticeFormats));
}

//------------------------------------------------------------------------------
// Called from xe_display.cpp
//------------------------------------------------------------------------------
void glReset(){
    vertexCountZero();
}

void glSync(){
    
}


void glInit() {
    // Create VB and IB
    vb_3 = Xe_CreateVertexBuffer(xe, MAX_VERTEX_COUNT * sizeof(glVerticeFormats));
    vb_4 = Xe_CreateVertexBuffer(xe, MAX_VERTEX_COUNT * sizeof(glVerticeFormats));
    vb_3s = Xe_CreateVertexBuffer(xe, MAX_VERTEX_COUNT * sizeof(glVerticeFormats));
    
    ib_tri = Xe_CreateIndexBuffer(xe, 6 * sizeof (uint16_t), XE_FMT_INDEX16);
    ib_quads = Xe_CreateIndexBuffer(xe, 6 * sizeof (uint16_t), XE_FMT_INDEX16);
    
    // lock
    void *vt = Xe_IB_Lock(xe,ib_tri,0,6 * sizeof (uint16_t),XE_LOCK_WRITE);
    void *vq = Xe_IB_Lock(xe,ib_quads,0,6 * sizeof (uint16_t),XE_LOCK_WRITE);
    
    // Copy
    memcpy(vt,prim_tri_strip,6 * sizeof (uint16_t));
    memcpy(vq,prim_quads,6 * sizeof (uint16_t));
    
    // Unlock
    Xe_IB_Unlock(xe,ib_tri);
    Xe_IB_Unlock(xe,ib_quads);
    
    glReset();
}


//------------------------------------------------------------------------------
// Gl Draw
//------------------------------------------------------------------------------
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
    printf("Unknow primitive\r\n");
    return 0;
}


void glBegin(int mode) {  
    switch (gl_mode) {
        case GL_TRIANGLE_STRIP:
        {
            vb = vb_3s;
        }
        case GL_TRIANGLES:
        {
            vb = vb_3;
        }
        case GL_QUADS:
        {
            vb = vb_4;
        }
    }
    
    
    // Lock and zero current vb
    glPrepare(glGetModelSize());
   
    gl_mode = mode;
    off_v = 0;
}

void glEnd() {
    
    glUnlockVb();
    // Select shader
    if(texture_combiner_enabled){
//        if(color_combiner_enabled){
//            XeSetCombinerG();
//        }
//        else{
//            XeSetCombinerF();
//        }
        XeSetCombinerG();
    }
    else{
        XeSetCombinerC();
    }
    
    switch (gl_mode) {
        case GL_TRIANGLE_STRIP:
        {
            Xe_SetIndices(xe,ib_tri);
            Xe_DrawIndexedPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 0, 6, 0, 2);
            break;
        }
        case GL_TRIANGLES:
        {
            Xe_SetIndices(xe,NULL);
            Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 1);
            break;
        }
        case GL_QUADS:
        {
            Xe_SetIndices(xe,NULL);
            Xe_DrawPrimitive(xe, XE_PRIMTYPE_QUADLIST, 0, 1);
            break;
        }
    }
    
    // todo: better alignement fixe
    //vertexCount += glGetModelSize() * ( sizeof (glVerticeFormats) );
    vertexCountAdd(glGetModelSize() * ( sizeof (glVerticeFormats) ));

    // Reset color
    // gl_color = 0;
    color_combiner_enabled = 0;
}

//xe_display.c
extern float screen[2];

void glTexCoord2fv(float * st) {
    gl_u = st[0];
    gl_v = st[1];
}

void glVertex3fv(float * v) {
//    currentVertex[0].x = v[0];
//    currentVertex[0].y = v[1];
//    currentVertex[0].z = v[2];
//    currentVertex[0].u = gl_u;
//    currentVertex[0].v = gl_v;
//    currentVertex[0].color = gl_color.f;
//    currentVertex++;
    
    currentVertex[0] = v[0];
    currentVertex[1] = v[1];
    currentVertex[2] = v[2];
    currentVertex[3] = gl_color.f;
    
    currentVertex+=4;
}
void glColor4ubv(u8 *v) {

    uint32_t c = *(uint32_t*) v;
    gl_color.u = c;
    color_combiner_enabled = 1;
}
