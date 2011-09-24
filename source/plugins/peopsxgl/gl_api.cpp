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

void glDraw();

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

static struct XenosIndexBuffer *ib = NULL;

static uint16_t prim_tri_strip[] = {0, 1, 2, 2, 1, 3}; // GL_TRIANGLE_STRIP
static uint16_t prim_quads[] = {0, 1, 2, 0, 1, 3}; // GL_QUADS // not used

int indiceCount=0;
//int vertexCount=0;


typedef struct __attribute__((__packed__)) glVerticeFormats {
    float x, y, z;
    float u, v;
    float color;
    //float r,g,b,a;
} glVerticeFormats;

glVerticeFormats * currentVertex = NULL;
glVerticeFormats * firstVertex = NULL;

uint16_t * currentIndice = NULL;
uint16_t * firstIndice = NULL;

int prevInCount = 0;

static int texture_combiner_enabled = 0;
static int color_combiner_enabled = 0;

int vertexCount(){
    return currentVertex-firstVertex;
}

int indicesCount(){
    return currentIndice - firstIndice;
}

//------------------------------------------------------------------------------
// Changes gl states
//------------------------------------------------------------------------------
void glStatesChanged();
static XenosSurface * gl_old_s = NULL;

void XeSetTexture(struct XenosSurface * s ){
    if(gl_old_s !=s)
    {
        gl_old_s = s;
        glStatesChanged();
        Xe_SetTexture(xe, 0, s);
    }
}

void XeEnableTexture() {
    texture_combiner_enabled = 1;
}

void XeDisableTexture() {
   texture_combiner_enabled = 0;
   XeSetTexture(NULL);
}

// Draw
void glStatesChanged(){
    //glSync();
    //glReset();
    glDraw();
}

//------------------------------------------------------------------------------
// Lock Unlock Prepare VB
//------------------------------------------------------------------------------
static void glLock() {
    // VB
    Xe_SetStreamSource(xe, 0, vb, 0, 4);
    firstVertex=currentVertex = (glVerticeFormats *) Xe_VB_Lock(xe, vb,0, MAX_VERTEX_COUNT * sizeof(glVerticeFormats), XE_LOCK_WRITE);
    
    // IB
    Xe_SetIndices(xe,ib);
    firstIndice=currentIndice=(uint16_t *)Xe_IB_Lock(xe,ib,0,MAX_INDICE_COUNT*sizeof(uint16_t),XE_LOCK_WRITE);
}


static void glUnlock() {
    Xe_VB_Unlock(xe, vb);
    Xe_IB_Unlock(xe, ib);
}


static void glPrepare(int count) {     
    // Zero it
    memset(currentVertex,0,count * sizeof(glVerticeFormats));
}

//------------------------------------------------------------------------------
// Called from xe_display.cpp
//------------------------------------------------------------------------------
void glReset(){
    glLock();
    prevInCount = 0;
}

void glDraw(){
    if (indicesCount()>prevInCount){
        //Xe_DrawIndexedPrimitive(xe,XE_PRIMTYPE_TRIANGLELIST,0,0,vertexCount(),0,indicesCount()/3);
        Xe_DrawIndexedPrimitive(xe,XE_PRIMTYPE_TRIANGLELIST,0,0,vertexCount(),prevInCount,(indicesCount()-prevInCount)/3);

        prevInCount = indicesCount();
    }
}

void glSync(){
    
    glDraw();
    
    glUnlock();
}


void glInit() {
    // Create VB and IB
    vb = Xe_CreateVertexBuffer(xe, MAX_VERTEX_COUNT * sizeof(glVerticeFormats));
    ib = Xe_CreateIndexBuffer(xe, MAX_VERTEX_COUNT * sizeof (uint16_t), XE_FMT_INDEX16);
    
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
      
    // Lock and zero current vb
    glPrepare(glGetModelSize());
   
    gl_mode = mode;
    off_v = 0;
}

void glEnd() {
   
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
            for(int i =0 ;i<6;i++){
                currentIndice[0]=vertexCount()+prim_tri_strip[i];
                currentIndice++;
            }
            break;
        }
        case GL_TRIANGLES:
        {
            for(int i =0 ;i<3;i++){
                currentIndice[0]=vertexCount()+prim_tri_strip[i];
                currentIndice++;
            }
            break;
        }
        case GL_QUADS:
        {
            for(int i =0 ;i<6;i++){
                currentIndice[0]=vertexCount()+prim_quads[i];
                currentIndice++;
            }
            break;
        }
    }


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
    currentVertex[0].x = v[0];
    currentVertex[0].y = v[1];
    currentVertex[0].z = v[2];
    currentVertex[0].u = gl_u;
    currentVertex[0].v = gl_v;
    currentVertex[0].color = gl_color.f;
    currentVertex++;

}
void glColor4ubv(u8 *v) {

    uint32_t c = *(uint32_t*) v;
    gl_color.u = c;
    color_combiner_enabled = 1;
}
