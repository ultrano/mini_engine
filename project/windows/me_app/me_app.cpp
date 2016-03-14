// me_app.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MiniEngine.h"
#include "opengl/glew.h"
#include "opengl/glut.h"

int main()
{
	MNStart("../../../resource/script/test.mn");
	// 디스플레이 버퍼를 RGB색상과 더블버퍼로 사용.
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	// 윈도우 크기 및 생성.
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

