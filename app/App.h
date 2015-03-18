#ifndef _H_App
#define _H_App

void exposeApp(MNFiber* fiber)
{
	fiber->push_string("rootPath");
	fiber->push_string("../resource/script/");
	fiber->store_global();
}
#endif