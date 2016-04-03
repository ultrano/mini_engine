#include "MiniEngine.h"
#include "MNArray.h"
#include "MNBasicLib.h"
#include "MNClosure.h"
#include "MNCollectable.h"
#include "MNCommand.h"
#include "MNCompiler.h"
#include "MNCountable.h"
#include "MNFiber.h"
#include "MNFunction.h"
#include "MNGlobal.h"
#include "MNLexer.h"
#include "MNList.h"
#include "MNMemory.h"
#include "MNObject.h"
#include "MNPrimaryType.h"
#include "MNReferrer.h"
#include "MNRtti.h"
#include "MNString.h"
#include "MNTable.h"
#include "MNUserData.h"
#include "MNFileSystem.h"

#include "MNOpenGL.h"



#define MNContext (_MNContext::instance())
class _MNContext
{
public:
	static _MNContext& instance() { static _MNContext inst; return inst; }
	MNFiber* main;
};



tuint32& texID()
{
    static tuint32 id;
    return id;
}

void MNStart(const char* resourceFolder)
{
    MNResourceFolderPath(resourceFolder);
    MNContext.main = new MNFiber();
    MNContext.main->dofile("script/main.mn");

}

void MNUpdate()
{
	MNContext.main->push_string("onUpdate");
	MNContext.main->load_global();
	MNContext.main->load_stack(0);
	MNContext.main->call(1, false);
}

void MNRender()
{
    MNContext.main->push_string("onRender");
    MNContext.main->load_global();
    MNContext.main->load_stack(0);
    MNContext.main->call(1, false);
}

void MNTouch(int type, int posX, int posY)
{
	MNContext.main->push_string("onTouch");
	MNContext.main->load_global();
	MNContext.main->load_stack(0);
	MNContext.main->push_integer(type);
	MNContext.main->push_integer(posX);
	MNContext.main->push_integer(posY);
	MNContext.main->call(4, false);
}