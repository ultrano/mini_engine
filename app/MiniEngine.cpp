// MiniEngine.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
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

