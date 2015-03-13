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

#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <WinSock.h>

tboolean app_printxy(MNFiber* fiber)
{
	fiber->load_stack(3);
	fiber->tostring();
	MNString* str = fiber->get(-1).toString();
	if (!str) return false;

	tint x = fiber->get(1).toInt();
	tint y = fiber->get(2).toInt();

	COORD coord = {x,y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
	printf(str->ss().c_str());
}

tboolean app_kbhit(MNFiber* fiber)
{
	fiber->push_bool(_kbhit());
	return true;
}

tboolean app_getch(MNFiber* fiber)
{
	int a = -1;
	if (_kbhit()) a = getch();
	fiber->push_int(a);
	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	MNFiber* fiber = new MNFiber();

	//! set global field
	{
		fiber->push_string("printxy");
		fiber->push_closure(app_printxy);
		fiber->store_global();

		fiber->push_string("kbhit");
		fiber->push_closure(app_kbhit);
		fiber->store_global();

		fiber->push_string("getch");
		fiber->push_closure(app_getch);
		fiber->store_global();

		fiber->push_string("rootPath");
		//fiber->push_string("C:/workspace/mini_engine/");
		fiber->push_string("D:/documents/workspace/mini_engine/");
		fiber->store_global();
	}

	//! compile test
	{
		fiber->dofile("test2.txt");
	}


	fiber->global()->finalize();
	return 0;
}

