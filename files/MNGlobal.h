#ifndef _H_MNGlobal
#define _H_MNGlobal

#include "MNPrimaryType.h"

class MNObject;
class MNFiber;
class MNTable;
class MNCollectable;
class MNGlobal
{
public:
	
	MNGlobal(MNFiber* rootState);
	~MNGlobal();

	//MNGCObject* generateString(const tstring& str);
	void getString(MNObject& ret, const tstring& str);
	void GC();

	MNTable* m_stringTable;
	MNCollectable* m_heap;
	MNFiber* m_root;
};

#endif