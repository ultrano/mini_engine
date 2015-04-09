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

int main()
{
	MNFiber* fiber = mainFiber();

	//! set global field
	{
		exposeGL(fiber);
	}

	//! compile test
	{
		fiber->dofile("../resource/script/init.mn");
	}

	fiber->global()->finalize();
	return 0;
}

