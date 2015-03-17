#ifndef _H_OpenGL
#define _H_OpenGL

#include "open\glew.h"
#include "open\glut.h"

class MNFiber;
unsigned int glLoadTexture(const char* fileName, int& width, int& height);
unsigned int glLoadProgram(const char* source);
void exposeGL(MNFiber* fiber);;

#endif