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

tboolean common_bind(MNFiber* fiber)
{
	MNObject cls = fiber->get(1);
	MNObject obj = fiber->get(2);
	MNClosure* closure = cls.toClosure();
	if (closure) closure->bindThis(obj);
	return false;
}

tboolean common_float(MNFiber* fiber)
{
	fiber->push_float(fiber->get(1).toFloat());
	return true;
}

tboolean common_int(MNFiber* fiber)
{
	fiber->push_int(fiber->get(1).toInt());
	return true;
}

tboolean common_typeof(MNFiber* fiber)
{
	const MNObject& val = fiber->get(1);

	MNObject ret;
	switch (val.getType())
	{
	case TObjectType::Int      : fiber->push_string("int"); break;
	case TObjectType::Null     : fiber->push_string("null"); break;
	case TObjectType::Float    : fiber->push_string("float"); break;
	case TObjectType::String   : fiber->push_string("string"); break;
	case TObjectType::Pointer  : fiber->push_string("pointer"); break;
	case TObjectType::Function : fiber->push_string("function"); break;
	case TObjectType::CFunction: fiber->push_string("cfunction"); break;
	default:
	{
		fiber->load_stack(1);
		fiber->push_string("type");
		fiber->load_field();
	}
	break;
	}
	return true;
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

tboolean table_count(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNTable* tbl = obj.toTable();
	if (!tbl) return false;
	fiber->push_int(tbl->count());
	return true;
}

tboolean table_insert(MNFiber* fiber)
{
	fiber->store_raw_field(true);
	return false;
}

tboolean table_total(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNTable* tbl = obj.toTable();
	if (!tbl) return false;
	fiber->push_int(tbl->total());
	return true;
}

tboolean fiber_new(MNFiber* fiber)
{
	MNFiber* newFiber = new MNFiber(fiber->global());
	newFiber->setMeta(fiber->get(0));
	newFiber->push(fiber->get(1));
	newFiber->load_stack(0);
	newFiber->enterCall(1, true);
	newFiber->push_null();
	fiber->push(MNObject(TObjectType::Fiber, newFiber->getReferrer()));
	return true;
}

tboolean fiber_next(MNFiber* fiber)
{
	MNFiber* newFiber = fiber->get(0).toFiber();
	if (!newFiber) return false;
	newFiber->pop(1);
	newFiber->push(fiber->get(1));
	printf("stack size: %d\n", newFiber->stackSize());
	tbyte cmd = newFiber->excuteCall();
	fiber->push_bool(cmd == cmd_yield);
	return true;
}

tboolean fiber_value(MNFiber* fiber)
{
	MNFiber* newFiber = fiber->get(0).toFiber();
	if (!newFiber) return false;
	fiber->push(newFiber->get(-1));
	return true;
}

tboolean closure_call(MNFiber* fiber)
{
	fiber->call(fiber->stackSize()-1, true);
	return true;
}

tboolean table_traverse(MNFiber* fiber)
{
	MNTable* tbl  = fiber->get(0).toTable();
	if (!tbl) return false;

	MNTable* itor  = fiber->get(1).toTable();
	if (!itor) return false;

	tsize index = 0;
	fiber->load_stack(1);
	fiber->push_string("index");
	if (fiber->load_raw_field())
	{
		index = fiber->get(-1).toInt() + 1;
	}
	else
	{
		fiber->load_stack(1);
		fiber->push_string("index");
		fiber->push_int(0);
		fiber->store_raw_field(true);
	}

	MNObject key, val;
	bool ret = false;
	while (ret = tbl->traverse(index, key, val))
	{
		if (!key.isNull()) break;
		index += 1;
	}
	if (ret)
	{
		fiber->load_stack(1);
		fiber->push_string("index");
		fiber->push_int(index);
		fiber->store_field();

		fiber->load_stack(1);
		fiber->push_string("key");
		fiber->push(key);
		fiber->store_field();

		fiber->load_stack(1);
		fiber->push_string("val");
		fiber->push(val);
		fiber->store_field();
	}

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
	m_shared = new MNTable();
	m_shared->link(this);
	root->push(MNObject(TObjectType::Table, m_shared->getReferrer()));

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

		root->up(1, 0);
		root->push_string("float");
		root->push_closure(common_float);
		root->store_field();

		root->up(1, 0);
		root->push_string("int");
		root->push_closure(common_int);
		root->store_field();

		root->up(1, 0);
		root->push_string("typeof");
		root->push_closure(common_typeof);
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

	//! closure meta table
	{
		root->up(1, 0);
		root->push_string("closure");
		root->push_table();

		{
			root->up(1, 0);
			root->push_string("type");
			root->push_string("closure");
			root->store_field();

			root->up(1, 0);
			root->push_string("call");
			root->push_closure(closure_call);
			root->store_field();

			root->up(1, 0);
			root->push_string("->");
			root->load_stack(-2);
			root->store_field();
		}

		root->store_field();
	}

	//! fiber meta table
	{
		root->up(1, 0);
		root->push_string("fiber");
		root->push_table();

		{
			root->up(1, 0);
			root->push_string("type");
			root->push_string("fiber");
			root->store_field();

			root->up(1, 0);
			root->push_string("new");
			root->push_closure(fiber_new);
			root->store_field();

			root->up(1, 0);
			root->push_string("next");
			root->push_closure(fiber_next);
			root->store_field();

			root->up(1, 0);
			root->push_string("value");
			root->push_closure(fiber_value);
			root->store_field();

			root->up(1, 0);
			root->push_string("->");
			root->load_stack(-2);
			root->store_field();
		}

		root->store_field();
	}

	//! array meta table
	{
		root->up(1, 0);
		root->push_string("array");
		root->push_table();

		{
			root->up(1, 0);
			root->push_string("type");
			root->push_string("array");
			root->store_field();

			root->up(1, 0);
			root->push_string("add");
			root->push_closure(array_add);
			root->store_field();

			root->up(1, 0);
			root->push_string("count");
			root->push_closure(array_count);
			root->store_field();

			root->up(1, 0);
			root->push_string("->");
			root->load_stack(-2);
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
			root->push_string("type");
			root->push_string("table");
			root->store_field();

			root->up(1, 0);
			root->push_string("count");
			root->push_closure(table_count);
			root->store_field();

			root->up(1, 0);
			root->push_string("insert");
			root->push_closure(table_insert);
			root->store_field();

			root->up(1, 0);
			root->push_string("total");
			root->push_closure(table_total);
			root->store_field();

			root->up(1, 0);
			root->push_string("traverse");
			root->push_closure(table_traverse);
			root->store_field();

			root->up(1, 0);
			root->push_string("->");
			root->load_stack(-2);
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