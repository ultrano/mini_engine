#include "MNGlobal.h"
#include "MNCollectable.h"
#include "MNTable.h"
#include "MNFiber.h"
#include "MNArray.h"
#include "MNClosure.h"
#include "MNString.h"

#include <stdlib.h>
#include <math.h>

tboolean common_print(MNFiber* fiber)
{
	tsize sz = fiber->stackSize();
	for (tsize i = 1; i < sz; ++i)
	{
		fiber->tostring();
		fiber->load_stack(i);
		fiber->tostring();
		MNObject obj = fiber->get(-1);
		fiber->pop(1);
		MNString* str = obj.toString();
		printf(str->ss().c_str());
	}
	printf("\n");
	return false;
}

tboolean object_setmeta(MNFiber* fiber)
{
	MNCollectable* obj = fiber->get(1).toCollectable();
	if (!obj) return false;
	obj->setMeta(fiber->get(2));
	return false;
}

tboolean math_sqrt(MNFiber* fiber)
{
	float a = sqrtf(fiber->get(1).toFloat());
	fiber->push_float(a);
	return true;
}

tboolean common_bind(MNFiber* fiber)
{
	MNObject cls = fiber->get(1);
	MNObject obj = fiber->get(2);
	MNClosure* closure = cls.toClosure();
	if (closure) closure->bindThis(obj);
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
	m_root->m_global = this;
	m_stringTable->link(this);

	//! global table
	m_root->push_table();

	//! common function
	{
		m_root->up(1, 0);
		m_root->push_string("print");
		m_root->push_closure(common_print);
		m_root->store_field();

		m_root->up(1, 0);
		m_root->push_string("bind");
		m_root->push_closure(common_bind);
		m_root->store_field();
	}

	//! math function
	{
		m_root->up(1, 0);
		m_root->push_string("sqrt");
		m_root->push_closure(math_sqrt);
		m_root->store_field();
	}

	//! object function
	{
		m_root->up(1, 0);
		m_root->push_string("setmeta");
		m_root->push_closure(object_setmeta);
		m_root->store_field();
	}

	//! array meta table
	{
		m_root->up(1, 0);
		m_root->push_string("array");
		m_root->push_table();

		{
			m_root->up(1, 0);
			m_root->push_string("-<");
			m_root->push_closure(array_set_field);
			m_root->store_field();

			m_root->up(1, 0);
			m_root->push_string("->");
			m_root->push_closure(array_get_field);
			m_root->store_field();
		}

		m_root->store_field();
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