// me_app.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../source/MiniEngine.h"
#include "glew.h"
#include "glut.h"


void callbackIdle()
{
	MNUpdate();
}

void callbackTimer(int value)
{
	glutPostRedisplay();
	glutTimerFunc((1.0f / 60.0f) * 1000, callbackTimer, 0);
}

void callbackDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutSwapBuffers();
}

int main()
{
	int width  = 320;
	int height = 480;
	// ���÷��� ���۸� RGB����� ������۷� ���.
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	// ������ ũ�� �� ����.
	glutInitWindowSize(width, height);
	glutCreateWindow("TSP");

	//glutMouseFunc(callbackMouse);
	//glutMotionFunc(callbackMouseMove);
	glutDisplayFunc(callbackDisplay);
	glutIdleFunc(callbackIdle);
	//glutReshapeFunc(callbackReshape);
	glutTimerFunc(1, callbackTimer, 0);
	//glutKeyboardFunc(callbackKeyboard);
	//glutKeyboardUpFunc(callbackKeyboardUp);
	glewInit();
	glViewport(0, 0, width, height);
	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	MNStart("../../../resource/", "script/test.mn");
	glutMainLoop();
    return 0;
}

