#include "MNInstance.h"
#include "MNClass.h"

MNInstance::MNInstance(const MNObject& _class)
{
	m_class = _class.toClass();
	setMeta(_class);

	m_fields = m_class->m_initVals;
}

MNInstance::~MNInstance()
{

}

tboolean MNInstance::trySet(const MNObject& key, const MNObject& val)
{
	MNClass::Member mem;
	if (!m_class->queryMember(key, mem)) return false;
	if (!mem.prop.get(MNClass::Field)) return false;
	m_fields[mem.index] = val;
	return true;
}

tboolean MNInstance::tryGet(const MNObject& key, MNObject& val) const
{
	MNClass::Member mem;
	if (!m_class->queryMember(key, mem)) return false;
	if (mem.prop.get(MNClass::Field))
	{
		val = m_fields[mem.index];
	}
	else if (mem.prop.get(MNClass::Method))
	{
		val = m_class->m_methods[mem.index];
	}
	return true;
}

void MNInstance::finalize()
{
	m_fields.clear();

	__super::finalize();
}

void MNInstance::travelMark()
{
	m_class->mark();
}