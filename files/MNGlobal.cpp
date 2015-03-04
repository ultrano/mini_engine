#include "MNGlobal.h"
#include "MNCollectable.h"
#include "MNTable.h"
#include "MNFiber.h"
#include "MNArray.h"
#include "MNClosure.h"

#include <stdlib.h>

tboolean array_add(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNArray* arr = obj.toArray();
	if (!arr) return false;

	const MNObject& arg1 = fiber->get(1);
	arr->add(arg1);

	return false;
}

tboolean array_get_field(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNArray* arr = obj.toArray();
	if (!arr) return false;

	const MNObject& arg1 = fiber->get(1);
	MNObject val;
	
	arr->tryGet(arg1, val);
	fiber->push(val);

	return true;
}

tboolean array_set_field(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNArray* arr = obj.toArray();
	if (!arr) return false;

	const MNObject& arg1 = fiber->get(1);
	const MNObject& arg2 = fiber->get(2);
	arr->trySet(arg1, arg2);

	return false;
}

MNGlobal::MNGlobal(MNFiber* rootState)
	: m_heap(NULL)
	, m_root(rootState)
	, m_stringTable(new MNTable())
{
	m_stringTable->link(this);

	MNTable* globalTable = new MNTable();
	globalTable->link(this);
	rootState->push(MNObject(TObjectType::Table, globalTable->getReferrer()));

	//! array meta table
	{
		MNClosure* closure = NULL;

		MNTable* meta = new MNTable();
		meta->link(this);

		closure = new MNClosure(MNObject::CFunction(array_add));
		meta->insert(MNObject::String("add"), MNObject(TObjectType::Closure, closure->link(this)->getReferrer()));

		closure = new MNClosure(MNObject::CFunction(array_get_field));
		meta->insert(MNObject::String("->"), MNObject(TObjectType::Closure, closure->link(this)->getReferrer()));

		closure = new MNClosure(MNObject::CFunction(array_set_field));
		meta->insert(MNObject::String("-<"), MNObject(TObjectType::Closure, closure->link(this)->getReferrer()));

		globalTable->insert(MNObject::String("array"), MNObject(TObjectType::Table, meta->getReferrer()));
	}
}

MNGlobal::~MNGlobal()
{
	m_stringTable->finalize();
	m_stringTable = NULL;
}

void MNGlobal::getString(MNObject& ret, const tstring& str)
{
	MNObject key = MNObject::String(str);
	if (!m_stringTable->tryGet(key, ret))
	{
		m_stringTable->insert(key, key);
		ret = key;
	}
}

//
//MNGCObject* MNGlobal::generateString(const tstring& str)
//{
//	tsolidstring ss(str);
//	MNObject strVal;
//	
//	StringTable::iterator itor = m_stringTable.find(ss.hash());
//	if (itor != m_stringTable.end()) strVal = itor->second;
//
//	if (!strVal.isObject())
//	{
//		MNString* strObj = (MNString*)(new MNString(str))->linkToState(this);
//		strVal = MNObject(strObj, true);
//		m_stringTable.insert(std::make_pair(ss.hash(), strVal));
//	}
//
//	return strVal.toObject();
//}

void MNGlobal::GC()
{
	//! clear marks in heap
	{
		MNCollectable* gct = m_heap;
		for (; gct != NULL && gct != m_heap; gct = gct->m_next) gct->unmark();
	}

	//! make mark travel from root
	{
		m_root->mark();
		m_stringTable->mark();
	}

	//! delete unmarked objects and fold link
	{
		while (m_heap != NULL && !m_heap->isMarked()) m_heap->finalize();

		MNCollectable* marked = m_heap;
		while (marked != NULL)
		{
			MNCollectable* unknown = marked->m_next;
			if (unknown != NULL && !unknown->isMarked())
			{
				unknown->finalize();
				continue;
			}
			marked = unknown;
		}
	}
}