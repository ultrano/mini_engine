#include "MNClass.h"
#include "MNTable.h"

MNClass::MNClass(tsize nmembers, MNClass* super)
{
	m_super   = super;
	m_members = new MNTable(nmembers);
	if (super)
	{
		tsize itor = 0;
		MNObject key, mem;
		while (super->iterate(itor, key, mem)) insert(key, mem);
	}
}

MNClass::~MNClass()
{
	m_members->finalize();
}

tboolean MNClass::insert(const MNObject& key, const MNObject& mem)
{
	return m_members->insert(key, mem);
}

tboolean MNClass::tryGet(const MNObject& key, MNObject& mem) const
{
	return m_members->tryGet(key, mem);
}

tboolean MNClass::hasKey(const MNObject& key)
{
	return m_members->hasKey(key);
}

tboolean MNClass::iterate(tsize& itor, MNObject& key, MNObject& mem) const
{
	return m_members->iterate(itor, key, mem);
}

void MNClass::travelMark()
{
	if (m_super != NULL) m_super->mark();
	m_members->mark();
}