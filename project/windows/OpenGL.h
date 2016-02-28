#ifndef _H_OpenGL
#define _H_OpenGL

#include "glew.h"
#include "glut.h"

class MNFiber;
GLuint glLoadTexture(const char* fileName, int& width, int& height);
GLuint glLoadProgram(const char* source);
void   glDrawRegion(int id, int srcWidth, int srcHeight, int offX, int offY, int width, int height);
void   exposeGL(MNFiber* fiber);;

#endif