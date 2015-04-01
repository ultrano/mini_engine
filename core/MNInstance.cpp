#include "MNInstance.h"
#include "MNClass.h"

MNInstance::MNInstance(const MNObject& _classObj)
{
	m_class = _classObj;
	MNClass* _class = _classObj.toClass();
	setMeta(_class->getMeta());

	m_fields = _class->m_initVals;
}

MNInstance::~MNInstance()
{

}

tboolean MNInstance::trySet(const MNObject& key, const MNObject& val)
{
	MNClass* _class = m_class.toClass();
	MNClass::Member mem;
	if (!_class->queryMember(key, mem)) return false;
	else if (mem.prop.get(MNClass::Static))
	{
		_class->m_statics[mem.index] = val;
	}
	else if (mem.prop.get(MNClass::Field))
	{
		m_fields[mem.index] = val;
	}
	else return false;
	return true;
}

tboolean MNInstance::tryGet(const MNObject& key, MNObject& val) const
{
	MNClass* _class = m_class.toClass();
	MNClass::Member mem;
	val = MNObject::Null();
	if (!_class->queryMember(key, mem)) return false;
	else if (mem.prop.get(MNClass::Static))
	{
		val = _class->m_statics[mem.index];
	}
	else if (mem.prop.get(MNClass::Field))
	{
		val = m_fields[mem.index];
	}
	else if (mem.prop.get(MNClass::Method))
	{
		val = _class->m_methods[mem.index];
	}
	else return false;
	return true;
}

void MNInstance::finalize()
{
	m_fields.clear();

	__super::finalize();
}

void MNInstance::travelMark()
{
	MNClass* _class = m_class.toClass();
	_class->mark();

	for (tsize i = 0; i < m_fields.size(); ++i)
	{
		MNCollectable* col = m_fields[i].toCollectable();
		if (col) col->mark();
	}
}