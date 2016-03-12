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

#define MNContext (_MNContext::instance())
class _MNContext
{
public:
	static _MNContext& instance() { static _MNContext inst; return inst; }
	MNFiber* main;
};



void MNStart(const char* resourceFolder, const char* starterScript)
{
    MNResourceFolderPath(resourceFolder);
	MNBasicLib(MNContext.main = new MNFiber());
	if (starterScript != NULL) MNContext.main->dofile(starterScript);
}