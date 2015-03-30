#include "MNClass.h"
#include "MNTable.h"
#include "MNInstance.h"

MNClass::MNClass(tsize nmembers, const MNObject& super)
	: m_super(NULL)
	, m_members(NULL)
{
	m_super = super.toClass();
	setMeta(super);

	if (m_super) m_members = new MNTable(nmembers + m_super->m_members->count());
	else m_members = new MNTable(nmembers);

	if (m_super)
	{
		m_initVals = m_super->m_initVals;
		m_methods  = m_super->m_methods;
		MNTable* superMems = m_super->m_members;
		tsize itor = 0;
		MNObject key, val;
		while (superMems->iterate(itor, key, val)) m_members->insert(key, val);
	}
}

MNClass::~MNClass()
{
	if (m_members) m_members->finalize();
	m_members = NULL;
}

void     MNClass::newInstance(MNObject& ret)
{
	MNInstance* inst = new MNInstance(MNObject::Referrer(this->getReferrer()));
	ret = MNObject::Referrer(inst->getReferrer());
}

tboolean MNClass::addField(const MNObject& key, const MNObject& initVal)
{
	if (initVal.isClosure()) return addMethod(key, initVal);
	Member mem;
	mem.prop.set(Prop::Field, true);
	mem.index = m_initVals.size();
	m_initVals.push_back(initVal);
	return m_members->insert(key, MNObject::Int(mem._int));
}

tboolean MNClass::addMethod(const MNObject& key, const MNObject& methodVal)
{
	if (!methodVal.isClosure()) return addField(key, methodVal);
	Member mem;
	mem.prop.set(Prop::Method, true);
	mem.index = m_methods.size();
	m_methods.push_back(methodVal);
	return m_members->insert(key, MNObject::Int(mem._int));
}

tboolean MNClass::queryMember(const MNObject& key, Member& mem) const
{
	MNObject obj;
	bool ret = m_members->tryGet(key, obj);
	mem._int = obj.toInt();
	return ret;
}

tboolean MNClass::tryGet(const MNObject& key, MNObject& val)
{
	MNClass::Member mem;
	if (!queryMember(key, mem)) return false;
	if (mem.prop.get(MNClass::Field))
	{
		val = m_initVals[mem.index];
	}
	else if (mem.prop.get(MNClass::Method))
	{
		val = m_methods[mem.index];
	}
	else return false;
	return true;
}

void MNClass::finalize()
{
	if (m_members) m_members->finalize();
	m_members = NULL;

	__super::finalize();
}

void MNClass::travelMark()
{
	if (m_super != NULL) m_super->mark();
	m_members->mark();
}