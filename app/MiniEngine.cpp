// MiniEngine.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "MNGlobal.h"
#include "MNFiber.h"
#include "MNTable.h"
#include "MNCompiler.h"
#include "MNString.h"
#include "MNFunction.h"
#include "MNClosure.h"
#include "MNLexer.h"
#include "MNCompiler.h"
#include "MNUserData.h"
#include "OpenGL.h"

MNFiber* mainFiber()
{
	static MNFiber* fiber = new MNFiber();
	return fiber;
}


void callbackDisplay()
{
	MNFiber* fiber = mainFiber();
	fiber->push_string("displayCallback");
	fiber->load_global();
	fiber->load_stack(0);
	fiber->call(1, 0);
	glutSwapBuffers();
}

void callbackIdle()
{
	MNFiber* fiber = mainFiber();
	fiber->push_string("idleCallback");
	fiber->load_global();
	fiber->load_stack(0);
	fiber->call(1, 0);
}

void callbackTimer(int value)
{
	glutPostRedisplay();
	glutTimerFunc((1.0f / 60.0f) * 1000, callbackTimer, 0);
}

void callbackMouse(int button, int state, int x, int y)
{
	MNFiber* fiber = mainFiber();
	fiber->push_string("inputCallback");
	fiber->load_global();
	fiber->load_stack(0);
	fiber->push_integer(state);
	fiber->push_integer(x);
	fiber->push_integer(y);
	fiber->call(4, 0);
}

void callbackMouseMove(int x, int y)
{
	MNFiber* fiber = mainFiber();
	fiber->push_string("inputCallback");
	fiber->load_global();
	fiber->load_stack(0);
	fiber->push_integer(2);
	fiber->push_integer(x);
	fiber->push_integer(y);
	fiber->call(4, 0);
}

void callbackReshape(int width, int height)
{
	//SW_GC.onResize( width, height );
}

void callbackKeyboard(unsigned char key, int x, int y)
{
	//SW_GC.onKeyChange( key, true );
}

void callbackKeyboardUp(unsigned char key, int x, int y)
{
	//SW_GC.onKeyChange( key, false );
}

int main()
{
	MNFiber* fiber = mainFiber();

	//! set global field
	{
		float width = 320;
		float height = 480;
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

		glutInitWindowSize((int)width, (int)height);
		glutCreateWindow("TSP");

		float nearPlane = 1;
		float farPlane = 1000;
		//float w = 2*SWMath.tan( SWMath.pi/4 )*nearPlane;
		//float h = 2*SWMath.tan( SWMath.pi/4 )*nearPlane;

		glViewport(0, 0, width, height);
		glutMouseFunc(callbackMouse);
		glutMotionFunc(callbackMouseMove);
		glutDisplayFunc(callbackDisplay);
		glutIdleFunc(callbackIdle);
		glutReshapeFunc(callbackReshape);
		glutTimerFunc(1, callbackTimer, 0);
		glutKeyboardFunc(callbackKeyboard);
		glutKeyboardUpFunc(callbackKeyboardUp);
		glewInit();
		exposeGL(fiber);
	}

	//! compile test
	{
		fiber->dofile("../resource/script/init.mn");
	}

	fiber->global()->finalize();
	return 0;
}

