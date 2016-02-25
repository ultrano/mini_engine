#include "MiniEngine.h"
#include "MNFiber.h"
#include "MNBasicLib.h"

#define MNContext (_MNContext::instance())
class _MNContext
{
public:
	static _MNContext& instance() { static _MNContext inst; return inst; }
	MNFiber* main;
};



void MNStart(const char* starterScript)
{
	MNBasicLib(MNContext.main = new MNFiber());
	if (starterScript != NULL) MNContext.main->dofile(starterScript);
}