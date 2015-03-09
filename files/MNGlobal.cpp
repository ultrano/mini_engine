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
tboolean object_garbage_collect(MNFiber* fiber)
{
	fiber->push_int(fiber->global()->GC());
	return true;
}
tboolean object_setmeta(MNFiber* fiber)
{
	MNCollectable* obj = fiber->get(1).toCollectable();
	if (!obj) return false;
	obj->setMeta(fiber->get(2));
	return false;
}

tboolean object_delegator(MNFiber* fiber)
{
	MNClosure* closure = fiber->get(1).toClosure();
	MNObject delegator;
	closure->clone(delegator);
	delegator.toClosure()->bindThis(fiber->get(2));
	fiber->push(delegator);
	return true;
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

tboolean array_count(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNArray* arr = obj.toArray();
	if (!arr) return false;
	fiber->push_int(arr->count());
	return true;
}

tboolean array_add(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNArray* arr = obj.toArray();
	if (!arr) return false;
	arr->add(fiber->get(1));
	return false;
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

tboolean table_count(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNTable* tbl = obj.toTable();
	if (!tbl) return false;
	fiber->push_int(tbl->count());
	return true;
}

tboolean table_total(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNTable* tbl = obj.toTable();
	if (!tbl) return false;
	fiber->push_int(tbl->total());
	return true;
}

tboolean table_traverse(MNFiber* fiber)
{
	MNTable* tbl  = fiber->get(0).toTable();
	if (!tbl) return false;

	tsize index = fiber->get(1).toInt();
	MNObject key, val;
	bool ret = tbl->traverse(index, key, val);

	fiber->up(1,0);
	fiber->push_string("key");
	fiber->push(key);
	fiber->store_field();

	fiber->up(1,0);
	fiber->push_string("val");
	fiber->push(val);
	fiber->store_field();

	fiber->push(MNObject::Bool(ret));
	return true;
}

MNGlobal::MNGlobal(MNFiber* root)
	: m_heap(NULL)
	, m_root(root)
	, m_stringTable(new MNTable())
	, m_shared(NULL)
{
	root->m_global = this;
	m_stringTable->link(this);

	//! global table
	root->push_table();
	m_shared = root->get(-1).toTable();

	//! common function
	{
		root->up(1, 0);
		root->push_string("print");
		root->push_closure(common_print);
		root->store_field();

		root->up(1, 0);
		root->push_string("bind");
		root->push_closure(common_bind);
		root->store_field();
	}

	//! math function
	{
		root->up(1, 0);
		root->push_string("sqrt");
		root->push_closure(math_sqrt);
		root->store_field();
	}

	//! object function
	{
		root->up(1, 0);
		root->push_string("setmeta");
		root->push_closure(object_setmeta);
		root->store_field();

		root->up(1, 0);
		root->push_string("delegator");
		root->push_closure(object_delegator);
		root->store_field();

		root->up(1, 0);
		root->push_string("GC");
		root->push_closure(object_garbage_collect);
		root->store_field();
	}

	//! array meta table
	{
		root->up(1, 0);
		root->push_string("array");
		root->push_table();

		{
			root->up(1, 0);
			root->push_string("add");
			root->push_closure(array_add);
			root->store_field();

			root->up(1, 0);
			root->push_string("-<");
			root->push_closure(array_set_field);
			root->store_field();

			root->up(1, 0);
			root->push_string("->");
			root->push_closure(array_get_field);
			root->store_field();

			root->up(1, 0);
			root->push_string("count");
			root->push_closure(array_count);
			root->store_field();
		}

		root->store_field();
	}

	//! table meta table
	{
		root->up(1, 0);
		root->push_string("table");
		root->push_table();

		{
			root->up(1, 0);
			root->push_string("count");
			root->push_closure(table_count);
			root->store_field();

			root->up(1, 0);
			root->push_string("total");
			root->push_closure(table_total);
			root->store_field();

			root->up(1, 0);
			root->push_string("traverse");
			root->push_closure(table_traverse);
			root->store_field();
		}

		root->store_field();
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

tsize MNGlobal::GC()
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

	tsize count = 0;
	//! delete unmarked objects and fold link
	{
		while (m_heap != NULL && !m_heap->isMarked())
		{
			count += 1;
			m_heap->finalize();
		}

		MNCollectable* marked = m_heap;
		while (marked != NULL)
		{
			MNCollectable* unknown = marked->m_next;
			if (unknown != NULL && !unknown->isMarked())
			{
				count += 1;
				unknown->finalize();
				continue;
			}
			marked = unknown;
		}
	}
	return count;
}