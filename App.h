#ifndef _H_App
#define _H_App

void exposeApp(MNFiber* fiber)
{
	fiber->push_string("rootPath");
	//fiber->push_string("C:/workspace/mini_engine/");
	fiber->push_string("D:/documents/workspace/mini_engine/");
	//fiber->push_string("E:/dev/mini_engine/");
	fiber->store_global();
}
#endif