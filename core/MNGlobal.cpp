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
	tsize sz = fiber->localSize();
	for (tsize i = 1; i < sz; ++i)
	{
		fiber->load_stack(i);
		fiber->tostring();
		MNString* str = fiber->get(-1).toString();
		printf(str->ss().c_str());
		fiber->pop(1);
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

tboolean common_alloc(MNFiber* fiber)
{
	const MNObject& size = fiber->get(1);
	if (!size.isInt()) return false;
	fiber->push_userdata(size.toInt());
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

tboolean common_dofile(MNFiber* fiber)
{
	MNObject arg1 = fiber->get(1);
	if (!arg1.isString()) return false;

	MNString* path = arg1.toString();
	fiber->push_bool(fiber->dofile(path->ss().str()));

	return true;
}

tboolean common_stackSize(MNFiber* fiber)
{
	fiber->push_int(fiber->stackSize());
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
	fiber->load_stack(1);
	return true;
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

tboolean array_remove(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNArray* arr = obj.toArray();
	if (!arr) return false;
	fiber->push_bool(arr->remove(fiber->get(1)));
	return true;
}

tboolean array_clear(MNFiber* fiber)
{
	const MNObject& obj = fiber->get(0);
	MNArray* arr = obj.toArray();
	if (!arr) return false;
	arr->clear();
	return false;
}

tboolean array_iterate(MNFiber* fiber)
{
	MNArray* arr = fiber->get(0).toArray();
	if (!arr) return false;

	if (!fiber->get(1).isClosure()) return false;

	tsize itor = 0;
	MNObject val;
	while (arr->iterate(itor, val))
	{
		fiber->load_stack(1);
		fiber->load_stack(0);
		fiber->push_int(itor - 1);
		fiber->push(val);
		fiber->call(3, true);
		bool ret = fiber->get(-1).toBool(true);
		fiber->pop(1);
		if (!ret) break;
	}
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

tboolean table_iterate(MNFiber* fiber)
{
	MNTable* tbl = fiber->get(0).toTable();
	if (!tbl) return false;

	if (!fiber->get(1).isClosure()) return false;

	tsize itor = 0;
	MNObject key, val;
	while (tbl->iterate(itor, key, val))
	{
		if (key.isNull()) continue;
		fiber->load_stack(1);
		fiber->load_stack(0);
		fiber->push(key);
		fiber->push(val);
		fiber->call(3, true);
		bool ret = fiber->get(-1).toBool(true);
		fiber->pop(1);
		if (!ret) break;
	}
	return false;
}

tboolean fiber_new(MNFiber* fiber)
{
	MNFiber* newFiber = new MNFiber(fiber->global());
	newFiber->setMeta(fiber->get(0));
	newFiber->setStatus(MNFiber::Suspend);

	newFiber->push(fiber->get(1));
	newFiber->push(fiber->get(1));
	newFiber->load_stack(0);
	newFiber->enterCall(1, true);
	fiber->push(MNObject(TObjectType::Fiber, newFiber->getReferrer()));
	return true;
}

tboolean fiber_next(MNFiber* fiber)
{
	MNFiber* newFiber = fiber->get(0).toFiber();
	if (!newFiber) return false;
	else if (newFiber->getStatus() == MNFiber::Stop)   return false;
	else if (newFiber->getStatus() == MNFiber::Resume) return false;

	newFiber->setStatus(MNFiber::Resume);

	newFiber->set(-1, fiber->get(1));
	tint status = newFiber->excuteCall();

	newFiber->setStatus(status);
	fiber->push_bool(status == MNFiber::Suspend);
	return true;
}

tboolean fiber_reset(MNFiber* fiber)
{
	MNFiber* newFiber = fiber->get(0).toFiber();
	if (!newFiber) return false;
	else if (newFiber->getStatus() != MNFiber::Stop)
	{
		MNFiber::CallInfo* info = NULL;
		do 
		{
			info = newFiber->returnCall(false);
		} while (info->closure != NULL);
	}

	newFiber->setStatus(MNFiber::Suspend);
	newFiber->pop(1);
	newFiber->push(newFiber->getAt(1));
	newFiber->load_stack(0);
	newFiber->enterCall(1, true);
	return false;
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
	fiber->call(fiber->localSize() - 1, true);
	return true;
}

tboolean closure_compileFile(MNFiber* fiber)
{
	fiber->load_stack(1);
	fiber->tostring();
	MNString* str = fiber->get(-1).toString();
	
	MNObject func;
	if (!fiber->compileFile(func, str->ss().str())) return false;

	fiber->push_closure(NULL); //! [closure]
	MNClosure* closure = fiber->get(-1).toClosure();
	closure->setFunc(func);
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

		root->up(1, 0);
		root->push_string("dofile");
		root->push_closure(common_dofile);
		root->store_field();

		root->up(1, 0);
		root->push_string("stackSize");
		root->push_closure(common_stackSize);
		root->store_field();
		
		root->up(1, 0);
		root->push_string("alloc");
		root->push_closure(common_alloc);
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
			root->push_string("compileFile");
			root->push_closure(closure_compileFile);
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
			root->push_string("reset");
			root->push_closure(fiber_reset);
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
			root->push_string("remove");
			root->push_closure(array_remove);
			root->store_field();

			root->up(1, 0);
			root->push_string("clear");
			root->push_closure(array_clear);
			root->store_field();

			root->up(1, 0);
			root->push_string("count");
			root->push_closure(array_count);
			root->store_field();

			root->up(1, 0);
			root->push_string("iterate");
			root->push_closure(array_iterate);
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
		root->up(1, 0); //! [global]
		root->up(1, 0); //! [global global]
		root->push_string("table"); //! [global global "table"]
		root->push_table(); //! [global global "table" table]
		root->up(1, 2); //! [global table global "table" table]

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
			root->push_string("iterate");
			root->push_closure(table_iterate);
			root->store_field();

			root->up(1, 0);
			root->push_string("->");
			root->load_stack(-2);
			root->store_field();
		}

		root->store_field();
		root->set_meta();
	}

	//! cache table
	{
		root->up(1, 0);
		root->push_string("cache");
		root->push_table();
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

void MNGlobal::finalize()
{
	while (m_heap != NULL)
	{
		if (m_heap == m_root) m_heap = m_heap->m_next;
		else m_heap->finalize();
	}
	m_root->finalize();
	delete this;
}

tsize MNGlobal::GC()
{
	//! clear marks in heap
	{
		for (MNCollectable* gct = m_heap; gct != NULL; gct = gct->m_next) gct->unmark();
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