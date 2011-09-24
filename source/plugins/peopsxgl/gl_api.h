/* 
 * File:   gl_api.h
 * Author: cc
 *
 * Created on 14 septembre 2011, 14:01
 */

#ifndef GL_API_H
#define	GL_API_H

#define USE_GL_API 1

enum {
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_QUADS
};
void glBegin(int mode);
void glEnd();

void glInit();
void glReset();

void glSync();

void glTexCoord2fv(float * st);
void glVertex3fv(float * v);
void glColor4ubv (u8 *v);
#endif	/* GL_API_H */

