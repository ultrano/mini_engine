#include "MNInstance.h"
#include "MNClass.h"

MNInstance::MNInstance(const MNObject& _class)
{
	m_class = _class.toClass();
	setMeta(_class);
}

MNInstance::~MNInstance()
{

}

tboolean MNInstance::trySet(const MNObject& key, const MNObject& val)
{
	MNClass::Member mem;
	if (!m_class->tryGet(key, mem)) return false;
	if (!mem.prop.get(MNClass::Field)) return false;
	m_fields[mem.index] = val;
	return true;
}

tboolean MNInstance::tryGet(const MNObject& key, MNObject& val) const
{
	MNClass::Member mem;
	if (!m_class->tryGet(key, mem)) return false;
	val = m_fields[mem.index];
	return true;
}

void MNInstance::travelMark()
{
	m_class->mark();
}