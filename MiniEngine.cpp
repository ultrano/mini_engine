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

bool cfunction(MNFiber* fiber)
{
	const tchar* msg = fiber->get(0).toString()->ss().str().c_str();
	printf(msg);
	return false;
}

bool __get(MNFiber* fiber)
{
	const tchar* msg = fiber->get(1).toString()->ss().str().c_str();
	printf(msg);
	fiber->push_string("fuck");
	return true;
}

bool __equals(MNFiber* fiber)
{
	fiber->push_bool(true);
	return true;
}

bool __test(MNFiber* fiber)
{
	fiber->tostring();
	thashstring msg = fiber->get(-1).toString()->ss();
	printf("msg: %s\n", msg.str().c_str());
	return false;
}

int _tmain(int argc, _TCHAR* argv[])
{
	MNFiber* fiber = new MNFiber();

	//! set global field
	{
		int i = 0;
		fiber->push_string("foo");
		fiber->push_closure(__test);
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

	//! garbage collect test
	{
		fiber->push_table(); //! A
		fiber->push_table(); //! B
		fiber->load_stack(-2); //! A B A
		fiber->load_stack(-2); //! A B A B
		fiber->push_string("test"); //! A B A B "test"
		fiber->swap(); //! A B A "test" B
		fiber->store_field(); //! A B
		fiber->push_string("test"); //! A B "test"
		fiber->swap(); //! A "test" B
		fiber->store_field(); //! there is circular referencing and that is garbage now.
		fiber->global()->GC();
	}

	fiber->push_int(1);
	fiber->push_int(2);
	fiber->push_int(3);
	fiber->push_int(4);
	fiber->up(2, 2);
	fiber->swap();
	fiber->add();
	fiber->sub();
	fiber->mod();
	fiber->mul();
	fiber->div();
	fiber->equals();
	fiber->push_closure(cfunction);
	fiber->push_string("test");
	fiber->call(1, false);

	fiber->global()->GC();
	//! short, long string test
	{
		fiber->push_string("test"); //< this is short string
		fiber->push_string("testaasdfsfsafdsaf");
		fiber->push_string("testaasdfsfsafdsaf");
	}
	
	//! make meta
	{
		fiber->push_table();

		fiber->load_stack(-1);
		fiber->push_string("->");
		fiber->push_closure(__get);
		fiber->store_field();

		fiber->load_stack(-1);
		fiber->push_string("-<");
		fiber->push_closure(__get);
		fiber->store_field();

		fiber->load_stack(-1);
		fiber->push_string("==");
		fiber->push_closure(__equals);
		fiber->store_field();
	}

	//! table test
	/*{
		const tchar* val = NULL;

		fiber->push_table();
		fiber->up(1, 1);
		fiber->swap();
		fiber->set_meta();

		fiber->load_stack(-1);
		fiber->push_string("test");
		fiber->load_field();

		val = fiber->get(-1).toString()->ss().str().c_str();
		fiber->pop(1);

		fiber->load_stack(-1);
		fiber->push_string("test");
		fiber->push_string("test");
		fiber->store_field();

		fiber->load_stack(-1);
		fiber->push_string("test");
		fiber->load_field();

		val = fiber->get(-1).toString()->ss().str().c_str();
		fiber->pop(1);

		fiber->load_stack(-1);
		fiber->load_stack(-1);
		fiber->equals();
	}*/

	//! array test
	{
		fiber->push_array();

		fiber->load_stack(-1);

		fiber->load_stack(-1);
		fiber->push_string("add");
		fiber->load_field();

		fiber->swap();
		fiber->push_string("test");
		fiber->call(2, false);

		fiber->load_stack(-1);
		fiber->push_int(0);
		fiber->load_field();
	}

	fiber->global()->finalize();
	return 0;
}

