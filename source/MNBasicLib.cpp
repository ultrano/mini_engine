#include "MNBasicLib.h"
#include "MNFiber.h"
#include "MNGlobal.h"
#include "MNString.h"
#include "MNClosure.h"
#include "MNClass.h"
#include "MNInstance.h"
#include "MNTable.h"
#include "MNArray.h"

#include <stdlib.h>
#include <math.h>

struct CommonLib
{
	static bool print(MNFiber* fiber)
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

	static bool bind(MNFiber* fiber)
	{
		MNObject cls = fiber->get(1);
		MNObject obj = fiber->get(2);
		MNClosure* closure = cls.toClosure();
		if (closure) closure->bindThis(obj);
		return false;
	}

	static bool castFloat(MNFiber* fiber)
	{
		fiber->push_real(fiber->get(1).toReal());
		return true;
	}

	static bool castInt(MNFiber* fiber)
	{
		fiber->push_integer(fiber->get(1).toInt());
		return true;
	}

	static bool allocate(MNFiber* fiber)
	{
		const MNObject& size = fiber->get(1);
		if (!size.isInt()) return false;
		fiber->push_userdata((tsize)size.toInt());
		return true;
	}

	static bool _typeof(MNFiber* fiber)
	{
		const MNObject& val = fiber->get(1);
		switch (val.getType())
		{
		case TObjectType::Int      : fiber->push_string("int"); break;
		case TObjectType::Null     : fiber->push_string("null"); break;
		case TObjectType::Real     : fiber->push_string("float"); break;
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

	static bool dofile(MNFiber* fiber)
	{
		MNObject arg1 = fiber->get(1);
		if (!arg1.isString()) return false;

		MNString* path = arg1.toString();
		fiber->push_bool(fiber->dofile(path->ss().str()));

		return true;
	}

	static bool garbageCollect(MNFiber* fiber)
	{
		fiber->push_integer(fiber->global()->GC());
		return true;
	}

	static bool setmeta(MNFiber* fiber)
	{
		MNCollectable* obj = fiber->get(1).toCollectable();
		if (!obj) return false;
		obj->setMeta(fiber->get(2));
		fiber->load_stack(1);
		return true;
	}

	static bool getmeta(MNFiber* fiber)
	{
		MNCollectable* obj = fiber->get(1).toCollectable();
		if (!obj) return false;
		fiber->push(obj->getMeta());
		return true;
	}

	static bool delegator(MNFiber* fiber)
	{
		MNClosure* closure = fiber->get(1).toClosure();
		MNObject delegator;
		closure->clone(delegator);
		delegator.toClosure()->bindThis(fiber->get(2));
		fiber->push(delegator);
		return true;
	}

	static void expose(MNFiber* fiber)
	{
		fiber->push_string("print");
		fiber->push_closure(print);
		fiber->store_global();

		fiber->push_string("bind");
		fiber->push_closure(bind);
		fiber->store_global();

		fiber->push_string("float");
		fiber->push_closure(castFloat);
		fiber->store_global();

		fiber->push_string("int");
		fiber->push_closure(castInt);
		fiber->store_global();

		fiber->push_string("allocate");
		fiber->push_closure(allocate);
		fiber->store_global();

		fiber->push_string("typeof");
		fiber->push_closure(_typeof);
		fiber->store_global();

		fiber->push_string("dofile");
		fiber->push_closure(dofile);
		fiber->store_global();

		fiber->push_string("garbageCollect");
		fiber->push_closure(garbageCollect);
		fiber->store_global();

		fiber->push_string("setmeta");
		fiber->push_closure(setmeta);
		fiber->store_global();

		fiber->push_string("getmeta");
		fiber->push_closure(getmeta);
		fiber->store_global();

		fiber->push_string("delegator");
		fiber->push_closure(delegator);
		fiber->store_global();
	}
};

struct MathLib
{
	static bool sqrt(MNFiber* fiber)
	{
		float a = sqrtf((tfloat)fiber->get(1).toReal());
		fiber->push_real(a);
		return true;
	}

	static bool pow(MNFiber* fiber)
	{
		float a = powf((tfloat)fiber->get(1).toReal(), (tfloat)fiber->get(2).toReal());
		fiber->push_real(a);
		return true;
	}

	static void expose(MNFiber* fiber)
	{
		fiber->push_string("math");
		fiber->push_table();
		{
			fiber->load_stack(-1);
			fiber->push_string("sqrt");
			fiber->push_closure(sqrt);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("pow");
			fiber->push_closure(pow);
			fiber->store_field();
		}
		fiber->store_global();
	}
};

struct ClosureLib
{
	static bool call(MNFiber* fiber)
	{
		fiber->call(fiber->localSize() - 1, true);
		return true;
	}

	static bool compileFile(MNFiber* fiber)
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

	static void expose(MNFiber* fiber)
	{
		fiber->up(1, 0);
		fiber->push_string("closure");
		fiber->push_table();
		{
			fiber->load_stack(-1);
			fiber->push_string("type");
			fiber->push_string("closure");
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("call");
			fiber->push_closure(call);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("compileFile");
			fiber->push_closure(compileFile);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("->");
			fiber->load_stack(-2);
			fiber->store_field();
		}
		fiber->store_global();
	}
};

struct FiberLib
{
	static bool _new(MNFiber* fiber)
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

	static bool next(MNFiber* fiber)
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

	static bool reset(MNFiber* fiber)
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

	static bool value(MNFiber* fiber)
	{
		MNFiber* newFiber = fiber->get(0).toFiber();
		if (!newFiber) return false;
		fiber->push(newFiber->get(-1));
		return true;
	}

	static void expose(MNFiber* fiber)
	{
		fiber->up(1, 0);
		fiber->push_string("fiber");
		fiber->push_table();
		{
			fiber->load_stack(-1);
			fiber->push_string("type");
			fiber->push_string("fiber");
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("new");
			fiber->push_closure(_new);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("next");
			fiber->push_closure(next);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("reset");
			fiber->push_closure(reset);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("value");
			fiber->push_closure(value);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("->");
			fiber->load_stack(-2);
			fiber->store_field();
		}
		fiber->store_field();
	}
};

struct ArrayLib
{
	static bool count(MNFiber* fiber)
	{
		const MNObject& obj = fiber->get(0);
		MNArray* arr = obj.toArray();
		if (!arr) return false;
		fiber->push_integer(arr->count());
		return true;
	}

	static bool add(MNFiber* fiber)
	{
		const MNObject& obj = fiber->get(0);
		MNArray* arr = obj.toArray();
		if (!arr) return false;
		arr->add(fiber->get(1));
		return false;
	}

	static bool remove(MNFiber* fiber)
	{
		const MNObject& obj = fiber->get(0);
		MNArray* arr = obj.toArray();
		if (!arr) return false;
		fiber->push_bool(arr->remove(fiber->get(1)));
		return true;
	}

	static bool clear(MNFiber* fiber)
	{
		const MNObject& obj = fiber->get(0);
		MNArray* arr = obj.toArray();
		if (!arr) return false;
		arr->clear();
		return false;
	}

	static bool iterate(MNFiber* fiber)
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
			fiber->push_integer(itor - 1);
			fiber->push(val);
			fiber->call(3, true);
			bool ret = fiber->get(-1).toBool(true);
			fiber->pop(1);
			if (!ret) break;
		}
		return false;
	}

	static void expose(MNFiber* fiber)
	{
		fiber->push_string("array");
		fiber->push_table();
		{
			fiber->load_stack(-1);
			fiber->push_string("type");
			fiber->push_string("array");
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("add");
			fiber->push_closure(add);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("remove");
			fiber->push_closure(remove);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("clear");
			fiber->push_closure(clear);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("count");
			fiber->push_closure(count);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("iterate");
			fiber->push_closure(iterate);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("->");
			fiber->load_stack(-2);
			fiber->store_field();
		}
		fiber->store_global();
	}
};

struct TableLib
{
	static bool has(MNFiber* fiber)
	{
		const MNObject& obj = fiber->get(0);
		MNTable* tbl = obj.toTable();
		if (!tbl) return false;
		fiber->push_bool(tbl->hasKey(fiber->get(1)));
		return true;
	}

	static bool count(MNFiber* fiber)
	{
		const MNObject& obj = fiber->get(0);
		MNTable* tbl = obj.toTable();
		if (!tbl) return false;
		fiber->push_integer(tbl->count());
		return true;
	}

	static bool insert(MNFiber* fiber)
	{
		fiber->store_raw_field(true);
		return false;
	}

	static bool total(MNFiber* fiber)
	{
		const MNObject& obj = fiber->get(0);
		MNTable* tbl = obj.toTable();
		if (!tbl) return false;
		fiber->push_integer(tbl->total());
		return true;
	}

	static bool iterate(MNFiber* fiber)
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

	static void expose(MNFiber* fiber)
	{
		fiber->push_string("table");
		fiber->push_table();
		{
			fiber->load_stack(-1);
			fiber->push_string("type");
			fiber->push_string("table");
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("has");
			fiber->push_closure(has);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("count");
			fiber->push_closure(count);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("insert");
			fiber->push_closure(insert);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("total");
			fiber->push_closure(total);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("iterate");
			fiber->push_closure(iterate);
			fiber->store_field();

			fiber->load_stack(-1);
			fiber->push_string("->");
			fiber->load_stack(-2);
			fiber->store_field();
		}
		fiber->store_global();
	}
};

void MNBasicLib(MNFiber* fiber)
{
	CommonLib::expose(fiber);
	MathLib::expose(fiber);
	ClosureLib::expose(fiber);
	ArrayLib::expose(fiber);
	TableLib::expose(fiber);
}