// me_app.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MiniEngine.h"
#include "opengl/glew.h"
#include "opengl/glut.h"

int main()
{
	MNStart("../../../resource/script/test.mn");
	// ���÷��� ���۸� RGB����� ������۷� ���.
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	// ������ ũ�� �� ����.
	glutInitWindowSize((int)320, (int)480);
	glutCreateWindow("TSP");

	//glutMouseFunc(callbackMouse);
	//glutMotionFunc(callbackMouseMove);
	//glutDisplayFunc(callbackDisplay);
	//glutIdleFunc(callbackIdle);
	//glutReshapeFunc(callbackReshape);
	//glutTimerFunc(1, callbackTimer, 0);
	//glutKeyboardFunc(callbackKeyboard);
	//glutKeyboardUpFunc(callbackKeyboardUp);
	glewInit();

	glutMainLoop();
    return 0;
}

