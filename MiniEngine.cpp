// MiniEngine.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "files\MNGlobal.h"
#include "files\MNFiber.h"
#include "files\MNTable.h"
#include "files\MNCompiler.h"
#include "files\MNString.h"
#include "files\MNFunction.h"
#include "files\MNClosure.h"
#include "files\MNLexer.h"
#include "files\MNCompiler.h"
#include "files\MNUserData.h"

#include "OpenGL.h"
#include "App.h"

MNFiber* mainFiber()
{
	static MNFiber* fiber = new MNFiber();
	return fiber;
}

void callbackDisplay()
{
	//SW_GC.onRender();
	glDrawRegion(1,20,40,0,0,20,40);
	glutSwapBuffers();
}

void callbackIdle()
{
	MNFiber* fiber = mainFiber();
	fiber->push_string("idleCallback");
	fiber->load_global();
	fiber->load_stack(0);
	fiber->call(1,0);
}

void callbackTimer( int value )
{
	glutPostRedisplay();
	glutTimerFunc( (1.0f/60.0f)*1000 ,callbackTimer,0);
}

void callbackMouse( int button, int state, int x, int y )
{
	MNFiber* fiber = mainFiber();
	fiber->push_string("inputCallback");
	fiber->load_global();
	fiber->load_stack(0);
	fiber->push_int(state);
	fiber->push_int(x);
	fiber->push_int(y);
	fiber->call(4,0);
}

void callbackMouseMove( int x, int y )
{
	MNFiber* fiber = mainFiber();
	fiber->push_string("inputCallback");
	fiber->load_global();
	fiber->load_stack(0);
	fiber->push_int(2);
	fiber->push_int(x);
	fiber->push_int(y);
	fiber->call(4,0);
}

void callbackReshape( int width, int height )
{
	//SW_GC.onResize( width, height );
}

void callbackKeyboard( unsigned char key, int x, int y )
{
	//SW_GC.onKeyChange( key, true );
}

void callbackKeyboardUp( unsigned char key, int x, int y )
{
	//SW_GC.onKeyChange( key, false );
}

int _tmain(int argc, _TCHAR* argv[])
{
	//! opengl
	{
		float width = 320;
		float height = 480;
		glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );

		glutInitWindowSize( (int)width, (int)height);
		glutCreateWindow("TSP");

		float nearPlane = 1;
		float farPlane = 1000;
		//float w = 2*SWMath.tan( SWMath.pi/4 )*nearPlane;
		//float h = 2*SWMath.tan( SWMath.pi/4 )*nearPlane;

		glViewport(0,0,width,height);
		glutMouseFunc(callbackMouse);
		glutMotionFunc( callbackMouseMove );
		glutDisplayFunc(callbackDisplay);
		glutIdleFunc(callbackIdle);
		glutReshapeFunc( callbackReshape );
		glutTimerFunc( 1 ,callbackTimer,0);
		glutKeyboardFunc( callbackKeyboard );
		glutKeyboardUpFunc( callbackKeyboardUp );
		glewInit();
	}

	MNFiber* fiber = mainFiber();

	//! set global field
	{
		exposeApp(fiber);
		exposeGL(fiber);
	}

	//! compile test
	{
		fiber->dofile("init.mn");
	}

	glutMainLoop();

	fiber->global()->finalize();
	return 0;
}

