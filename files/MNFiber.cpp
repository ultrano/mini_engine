#include "MNFiber.h"
#include "MNGlobal.h"
#include "MNObject.h"
#include "MNString.h"
#include "MNClosure.h"
#include "MNTable.h"
#include "MNArray.h"
#include "MNFunction.h"

void binomalOp(MNFiber* fiber, const tstring& opStr, const MNObject& left, const MNObject& right, MNObject& ret)
{
	MNCollectable* collectable = left.toCollectable();
	MNObject meta = collectable->getMeta();
	MNObject op = MNObject::String(opStr);
	ret = MNObject::Null();
	while (true)
	{
		if (meta.isTable())
		{
			meta.toTable()->tryGet(op, ret);
			if (ret.isClosure()) meta = ret;
		}

		if (meta.isClosure())
		{
			fiber->push(meta);
			fiber->push(left);
			fiber->push(right);
			fiber->call(2, true);
			ret = fiber->get(-1);
			fiber->pop(1);
			break;
		}

		collectable = meta.toCollectable();
		if (collectable) meta = collectable->getMeta();
		else break;
	}
}

MNFiber::MNFiber(MNGlobal* global)
{
	m_info = new CallInfo();
	m_info->prev = NULL;
	m_info->pc = NULL;
	m_info->begin = 0; //! index zero is place for 'this'
	m_info->end = 0;
	m_info->ret = false;

	if (global == NULL)
	{
		global = new MNGlobal(this);
	}
	m_global = global;

	link(global);
}

MNFiber::~MNFiber()
{
	//setMeta(MNValue());
	m_stack.clear();
	while (m_info)
	{
		CallInfo* info = m_info->prev;
		m_info->closure = NULL;
		delete m_info;
		m_info = info;
	}

	if (this == m_global->m_root)
	{
		m_global->GC();
		delete m_global;
	}
}

MNGlobal* MNFiber::global() const
{
	return m_global;
}

void MNFiber::setAt(tint32 idx, const MNObject& val)
{
	if (idx < 0) return;
	if (idx < (tint32)m_stack.size()) m_stack[idx] = val;
}

const MNObject& MNFiber::getAt(tint32 idx) const
{
	if (idx >= 0 && idx < (tint32)m_stack.size()) return m_stack[idx];
	return MNObject::Null();
}

void MNFiber::set(tint32 idx, const MNObject& val)
{
	if (idx == 0) return;
	idx = ((idx < 0) ? m_info->end : m_info->begin) + idx;
	if (idx > m_info->begin && idx < m_info->end) setAt(idx, val);
}

const MNObject& MNFiber::get(tint32 idx) const
{
	idx = ((idx < 0) ? m_info->end : m_info->begin) + idx;
	if (idx >= m_info->begin && idx < m_info->end) return getAt(idx);
	return MNObject::Null();
}

const MNObject& MNFiber::getConst(tsize idx) const
{
	if (!m_info) return MNObject::Null();
	if (!m_info->closure) return MNObject::Null();
	
	MNFunction* func = m_info->closure->getFunc().toFunction();
	if (!func) return MNObject::Null();

	return func->getConst(idx);
}

void MNFiber::setUpval(tuint32 idx, const MNObject& val)
{
	//MNClosure* closure = (MNClosure*)m_info->closure.toObject();
	//if (closure) closure->setUpval(idx, val);
}

const MNObject& MNFiber::getUpval(tuint32 idx)
{
	//MNClosure* closure = (MNClosure*)m_info->closure.toObject();
	//if (closure) return closure->getUpval(idx);
	return MNObject::Null();
}

void MNFiber::push_null()
{
	push(MNObject::Null());
}

void MNFiber::push_int(tint32 val)
{
	push(MNObject::Int(val));
}

void MNFiber::push_float(tfloat val)
{
	push(MNObject::Float(val));
}

void MNFiber::push_string(const tstring& val)
{
	const tsize shortStrLen = 4;
	if (val.length() > shortStrLen)
	{
		MNObject ret;
		global()->getString(ret, val);
		push(ret);
	}
	else
	{
		push(MNObject::String(val));
	}
}

void MNFiber::push_pointer(void* val)
{
	push(MNObject::Pointer(val));
}

void MNFiber::push_bool(tboolean val)
{
	push(MNObject::Bool(val));
}

void MNFiber::push_closure(TCFunction val)
{
	MNClosure* closure = new MNClosure(MNObject::CFunction(val));
	MNReferrer* ref = closure->link(global())->getReferrer();
	MNObject obj(TObjectType::Closure, ref);
	push(obj);
}

void MNFiber::push_table()
{
	MNTable* table = new MNTable();
	MNReferrer* ref = table->link(global())->getReferrer();
	MNObject obj(TObjectType::Table, ref);
	push(obj);
}

void MNFiber::push_array()
{
	push_string("array");
	load_global();

	MNArray* array = new MNArray();
	array->link(global());
	array->setMeta(get(-1));
	pop(1);

	MNObject obj(TObjectType::Array, array->getReferrer());
	push(obj);
}

void MNFiber::push_const(tsize idx)
{
	push(getConst(idx));
}

void MNFiber::push(const MNObject& val)
{
	const tsize expand = 32;
	if (m_info->end >= (tint32)m_stack.size()) m_stack.resize(m_info->end + expand);
	setAt(m_info->end++, val);
}

void MNFiber::pop(tuint32 count)
{
	count = ((tint32)count < m_info->end) ? count : m_info->end;
	while (count--)
	{
		if ((m_info->end-1) > m_info->begin) setAt(--(m_info->end), MNObject::Null());
	}
}

void MNFiber::load_stack(tint32 idx)
{
	push(get(idx));
}

void MNFiber::store_stack(tint32 idx)
{
	set(idx, get(-1));
	pop(1);
}

void MNFiber::load_raw_field()
{
	MNObject obj = get(-2);
	MNObject key = get(-1);
	pop(2);

	MNObject val;
	switch (obj.getType())
	{
	case TObjectType::Table: obj.toTable()->tryGet(key, val); break;
	case TObjectType::Array: obj.toArray()->tryGet(key, val); break;
	}

	push(val);
}

void MNFiber::store_raw_field()
{
	MNObject obj = get(-3);
	MNObject key = get(-2);
	MNObject val = get(-1);
	pop(3);

	switch (obj.getType())
	{
	case TObjectType::Table: obj.toTable()->insert(key, val); break;
	case TObjectType::Array: obj.toArray()->trySet(key, val); break;
	}
}

void MNFiber::load_field()
{
	MNObject obj = get(-2);
	MNObject key = get(-1);
	pop(2);

	MNObject ret;
	tboolean succeed = false;
	switch (obj.getType())
	{
	case TObjectType::Table:
	{
		MNTable* table = obj.toTable();
		succeed = table->tryGet(key, ret);
	}
	break;
	}

	MNCollectable* collectable = obj.toCollectable();
	if (!succeed && collectable)
	{
		MNObject meta = collectable->getMeta();
		MNObject op = MNObject::String("->");
		while (true)
		{
			if (meta.isTable())
			{
				MNTable* metaTable = meta.toTable();
				if (metaTable->tryGet(key, ret)) break;
				metaTable->tryGet(op, ret);
				if (ret.isClosure()) meta = ret;
				ret = MNObject::Null();
			}

			if (meta.isClosure())
			{
				push(meta);
				push(obj);
				push(key);
				call(2, true);
				ret = get(-1);
				pop(1);
				break;
			}

			collectable = meta.toCollectable();
			if (collectable) meta = collectable->getMeta();
			else break;
		}
	}

	push(ret);
}

void MNFiber::store_field()
{
	MNObject obj = get(-3);
	MNObject key = get(-2);
	MNObject val = get(-1);
	pop(3);

	MNObject ret;
	tboolean succeed = false;
	switch (obj.getType())
	{
	case TObjectType::Table:
	{
		MNTable* table = obj.toTable();
		succeed = table->insert(key, val);
	}
	break;
	}

	MNCollectable* collectable = obj.toCollectable();
	if (!succeed && collectable)
	{
		MNObject meta = collectable->getMeta();
		MNObject op = MNObject::String("-<");
		while (true)
		{
			if (meta.isTable())
			{
				MNTable* metaTable = meta.toTable();
				if (metaTable->trySet(key, ret)) break;
				metaTable->tryGet(op, ret);
				if (ret.isClosure()) meta = ret;
				ret = MNObject::Null();
			}

			if (meta.isClosure())
			{
				push(meta);
				push(obj);
				push(key);
				push(val);
				call(3, false);
				break;
			}

			collectable = meta.toCollectable();
			if (collectable) meta = collectable->getMeta();
			else break;
		}
	}
}

void MNFiber::load_global()
{
	push(getAt(0));
	swap();
	load_raw_field();
}

void MNFiber::store_global()
{
	push(getAt(0));
	up(1, 2);
	pop(1);
	store_raw_field();
}

void MNFiber::set_meta()
{
	MNObject obj  = get(-2);
	MNObject meta = get(-1);
	pop(2);

	MNCollectable* collectable = obj.toCollectable();
	if (collectable) collectable->setMeta(meta);
}

void MNFiber::get_meta()
{
	MNObject obj = get(-1);
	pop(1);

	MNObject meta;
	MNCollectable* collectable = obj.toCollectable();
	if (collectable) meta = collectable->getMeta();
	
	push(meta);
}

void MNFiber::up(tsize n, tsize x)
{
	if (n == 0) return;

	tsize nx = n + x;
	for (tint i = 0; i < (tint32)n; ++i) push(MNObject::Null());
	for (tint i = 1; i <= (tint32)nx; ++i) set(-i, get(-i - n));
	for (tint i = 1; i <= (tint32)n; ++i) set(-i - nx, get(-i));
}

void MNFiber::swap()
{
	MNObject top = get(-1);
	set(-1, get(-2));
	set(-2, top);
}

void MNFiber::equals()
{
	MNObject left  = get(-2);
	MNObject right = get(-1);
	pop(2);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Null     : ret = MNObject::Bool(right.isNull()); break;
	case TObjectType::Pointer  : ret = MNObject::Bool(left.toPointer() == right.toPointer()); break;
	case TObjectType::Boolean  : ret = MNObject::Bool(left.toBool() == right.toBool()); break;
	case TObjectType::CFunction: ret = MNObject::Bool(left.toCFunction() == right.toCFunction()); break;
	case TObjectType::String   : if (right.isString()) ret = left.toString()->ss() == right.toString()->ss(); break;
	case TObjectType::Int:
	case TObjectType::Float:
	{
		if (right.isInt()) ret = MNObject::Bool(left.toInt() == right.toInt());
		else if (right.isFloat()) ret = MNObject::Bool(left.toFloat() == right.toFloat());
	}
	break;
	case TObjectType::Table:
	{
		binomalOp(this, "==", left, right, ret);
	}
	break;
	}
	push(ret);
}

void MNFiber::less_than()
{
	MNObject left  = get(-2);
	MNObject right = get(-1);
	pop(2);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int:
	case TObjectType::Float:
		{
			if (right.isInt()) ret = MNObject::Bool(left.toInt() < right.toInt());
			else if (right.isFloat()) ret = MNObject::Bool(left.toFloat() < right.toFloat());
		}
		break;
	case  TObjectType::Table:
		{
			binomalOp(this, "<", left, right, ret);
		}
		break;
	}
	push(ret);
}

void MNFiber::tostring()
{
	MNObject object = get(-1);
	pop(1);

	MNObject str = MNObject::String("[null:null]");
	switch (object.getType())
	{
	case TObjectType::Pointer  : str = MNObject::Format("[pointer: %p]", object.toPointer()); break;
	case TObjectType::Boolean  : str = MNObject::Format("[boolean: %s]", object.toBool() ? "true" : "false"); break;
	case TObjectType::CFunction: str = MNObject::Format("[cfunction: %p]", object.toCFunction()); break;
	case TObjectType::String   : str = object; break;
	case TObjectType::Int      : str = MNObject::Format("%d", object.toInt()); break;
	case TObjectType::Float    : str = MNObject::Format("%f", object.toFloat()); break;
	}

	push(str);
}

void MNFiber::add()
{
	MNObject left  = get(-2);
	MNObject right = get(-1);
	pop(2);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int   : if (right.isFloat() || right.isInt()) ret = MNObject::Int(left.toInt() + right.toInt()); break;
	case TObjectType::Float : if (right.isFloat() || right.isInt()) ret = MNObject::Float(left.toFloat() + right.toFloat()); break;
	case TObjectType::String:
	{
		push(right);
		tostring();
		MNObject strObj1 = get(-1);
		pop(1);

		MNString* str1 = left.toString();
		MNString* str2 = strObj1.toString();
		ret = MNObject::Format("%s%s", str1->ss().str().c_str(), str2->ss().str().c_str());
	}
	break;
	case  TObjectType::Table:
	{
		binomalOp(this, "+", left, right, ret);
	}
	break;
	}

	push(ret);
}

void MNFiber::sub()
{
	MNObject left  = get(-2);
	MNObject right = get(-1);
	pop(2);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int  : if (right.isFloat() || right.isInt()) ret = MNObject::Int(left.toInt() - right.toInt()); break;
	case TObjectType::Float: if (right.isFloat() || right.isInt()) ret = MNObject::Float(left.toFloat() - right.toFloat()); break;
	case TObjectType::Table:
	{
		binomalOp(this, "-", left, right, ret);
	}
	break;
	}

	push(ret);
}

void MNFiber::mul()
{
	MNObject left  = get(-2);
	MNObject right = get(-1);
	pop(2);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int: if (right.isFloat() || right.isInt()) ret = MNObject::Int(left.toInt() * right.toInt()); break;
	case TObjectType::Float: if (right.isFloat() || right.isInt()) ret = MNObject::Float(left.toFloat() * right.toFloat()); break;
	case TObjectType::Table:
	{
		binomalOp(this, "*", left, right, ret);
	}
	break;
	}

	push(ret);
}

void MNFiber::div()
{
	MNObject left  = get(-2);
	MNObject right = get(-1);
	pop(2);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int:
	{
		if (right.isFloat() || right.isInt())
		{
			if (right.toInt() != 0) ret = MNObject::Int(left.toInt() / right.toInt());
		}
	}
	break;
	case TObjectType::Float:
	{
		if (right.isFloat() || right.isInt())
		{
			if (right.toFloat() != 0.0f) ret = MNObject::Float(left.toFloat() / right.toFloat());
		}
	}
	break;
	case TObjectType::Table:
	{
		binomalOp(this, "/", left, right, ret);
	}
	break;
	}

	push(ret);
}

void MNFiber::mod()
{
	MNObject left  = get(-2);
	MNObject right = get(-1);
	pop(2);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int:
	{
		if (right.isFloat() || right.isInt())
		{
			if (right.toInt() != 0) ret = MNObject::Int(left.toInt() % right.toInt());
		}
	}
	break;
	case TObjectType::Float:
	{
		if (right.isFloat() || right.isInt())
		{
			if (right.toFloat() != 0.0f) ret = MNObject::Float(fmod(left.toFloat(), right.toFloat()));
		}
	}
	break;
	case TObjectType::Table:
	{
		binomalOp(this, "%", left, right, ret);
	}
	break;
	}

	push(ret);
}

MNFiber::CallInfo* MNFiber::enterCall(tuint nargs, bool ret)
{
	tuint clsIndex = m_info->end - (nargs + 1);
	MNObject clsVal = getAt(clsIndex);
	MNClosure* cls = clsVal.toClosure();
	if (!cls)
	{
		pop(nargs + ret);
		if (ret) push(MNObject::Null());
		return NULL;
	}

	MNFunction* func = cls->getFunc().toFunction();

	CallInfo* info = new CallInfo();
	info->prev    = m_info;
	info->closure = cls;
	info->begin   = clsIndex + 1;
	info->end     = m_info->end;
	info->pc      = func? func->getCode() : NULL;
	info->ret     = ret;

	m_info->end   = clsIndex;
	m_info        = info;

	if (func)
	{
		tsize sz = m_info->end - m_info->begin;
		while (sz++ < func->getVarCount()) push_null();
	}

	return m_info;
}

MNFiber::CallInfo* MNFiber::returnCall(bool retOnTop)
{
	bool needRet = m_info->ret;
	MNObject obj;
	if (needRet && retOnTop) obj = get(-1);

	tsize count = (m_info->end - m_info->prev->end);
	while (count--) setAt(--(m_info->end), MNObject::Null());

	CallInfo* info = m_info;
	m_info = info->prev;
	delete info;

	if (needRet) push(obj);

	return m_info;
}

UpLink*  MNFiber::openLink(tint32 index)
{
	tlist<UpLink*>::iterator itor = m_openLinks.begin();
	for (; itor != m_openLinks.end(); ++itor)
	{
		UpLink* ul = *itor;
		Opened* ov = (Opened*)ul->link;
		if (ov->index == (index + m_info->begin)) return ul;
	}

	//! add new up link
	{
		UpLink* ul = new UpLink;
		ul->link = new Opened(this, index + m_info->begin);
		ul->inc();
		m_openLinks.push_back(ul);
		return ul;
	}
	return NULL;
}

void  MNFiber::closeLinks(tint32 level)
{
	tsize index = (level + m_info->begin);
	tlist<UpLink*>::iterator itor = m_openLinks.begin();
	while (itor != m_openLinks.end())
	{
		UpLink* ul = *itor;
		Opened* ov = (Opened*)ul->link;
		if (ov->index >= index)
		{
			ul->close();
			ul->dec();
			itor = m_openLinks.erase(itor);
			continue;
		}
		++itor;
	}
}

tsize MNFiber::stackSize() const
{
	return m_info->end - m_info->begin;
}

void MNFiber::travelMark()
{
	for (tsize i = 0; i < m_stack.size(); ++i)
	{
		MNObject& val = m_stack[i];
		if (MNCollectable* collectable = val.toCollectable())
		{
			collectable->mark();
		}
	}
}

class CodeIterator
{
public:

	MNFiber::CallInfo* info;

	CodeIterator(MNFiber::CallInfo* info) : info(info) {};

	template<typename T>
	CodeIterator& operator >>(T& t)
	{
		t = *((T*)info->pc);
		info->pc += sizeof(T);
		return *this;
	}
	void jump(int pc)
	{
		info->pc += pc;
	}
};

void MNFiber::call(tsize nargs, bool ret)
{
	CallInfo* info = enterCall(nargs, ret);

	//! script
	while (info != NULL)
	{
		MNClosure* closure = info->closure;
		if (!closure) break;
		if (closure->getFunc().isNull()) break;

		if (closure->isNative())
		{
			TCFunction func = closure->getFunc().toCFunction();
			info = returnCall(func(this));
			if (info->closure && info->closure->isNative()) return;
			else continue;
		}

		CodeIterator code(info);
		while (code.info == info)
		{
			tbyte cmd;
			code >> cmd;
			switch (cmd)
			{
			case cmd_push_null : push_null(); break;
			case cmd_push_false: push_bool(false); break;
			case cmd_push_true: push_bool(true); break;
			case cmd_push_float:
			{
				float _float;
				code >> _float;
				push_float(_float);
			}
			break;
			case cmd_push_int:
			{
				tint32 _int;
				code >> _int;
				push_int(_int);
			}
			break;
			case cmd_push_table: push_table(); break;
			case cmd_push_array: push_array(); break;
			case cmd_push_closure:
			{
				tuint16 funcIndex, upLinkSize;
				code >> funcIndex >> upLinkSize;
				MNClosure* cls = new MNClosure();
				cls->setFunc(getConst(funcIndex));
				cls->link(global());
				while (upLinkSize--)
				{
					tbyte loc;
					tuint16 index;
					code >> loc >> index;
					UpLink* ul = (loc == cmd_load_stack) ? openLink(index) : closure->getLink(index);
					cls->addLink(ul);
				}
				push(MNObject(TObjectType::Closure, cls->getReferrer()));
			}
			break;

			case cmd_pop1: pop(1); break;
			case cmd_pop2: pop(2); break;
			case cmd_popn:
			{
				tbyte count;
				code >> count;
				pop(count);
			}
			break;
			case cmd_load_upval:
			{
				tuint16 index;
				code >> index;
				push(closure->getUpval(index));
			}
			break;
			case cmd_store_upval:
			{
				tuint16 index;
				code >> index;
				closure->setUpval(index, get(-1));
				pop(1);
			}
			break;
			case cmd_load_const:
			{
				tuint16 index;
				code >> index;
				push_const(index);
			}
			break;
			case cmd_load_stack:
			{
				tuint16 index;
				code >> index;
				load_stack(index);
			}
			break;
			case cmd_store_stack:
			{
				tuint16 index;
				code >> index;
				store_stack(index);
			}
			break;
			case cmd_load_field  : load_field(); break;
			case cmd_store_field : store_field(); break;
			case cmd_load_global:
			{
				tuint16 index;
				code >> index;

				push(getAt(0));
				push_const(index);
				load_raw_field();
			}
			break;
			case cmd_store_global:
			{
				tuint16 index;
				code >> index;

				MNObject val = get(-1);
				pop(1);

				push(getAt(0));
				push_const(index);
				push(val);
				store_global();
			}
			break;
			case cmd_set_meta    : set_meta(); break;
			case cmd_get_meta    : get_meta(); break;
			case cmd_up1:    up(1, 0); break;
			case cmd_up1_x2: up(1, 2); break;
			case cmd_up2:    up(2, 0); break;
			case cmd_up:
			{
				tbyte n, x;
				code >> n >> x;
				up(n, x);
			}
			break;
			case cmd_swap    : swap(); break;
			case cmd_eq  : equals(); break;
			case cmd_neq :
				{
					equals();
					MNObject ret = get(-1);
					if (ret.isBool())
					{
						pop(1);
						push(MNObject::Bool(!ret.toBool()));
					}
				}
				break;
			case cmd_lt : less_than(); break;
			case cmd_gt : swap(); less_than(); break;
			case cmd_leq:
			{
				up(2, 0);    //! [left right left right]
				less_than(); //! [left right bool]

				if (!get(-1).toBool())
				{
					pop(1);   //! [left right]
					equals(); //! [bool]
				}
				else
				{
					up(1, 2); //! [bool left right bool]
					pop(3);   //! [bool]
				}
			}
			break;
			case cmd_geq:
			{
				swap();
				up(2, 0);    //! [right left left right]
				less_than(); //! [right left bool]

				if (!get(-1).toBool())
				{
					pop(1);   //! [right left]
					equals(); //! [bool]
				}
				else
				{
					up(1, 2); //! [bool right left bool]
					pop(3);   //! [bool]
				}
			}
			break;
			case cmd_tostring: tostring(); break;
			case cmd_add     : add(); break;
			case cmd_sub     : sub(); break;
			case cmd_mul     : mul(); break;
			case cmd_div     : div(); break;
			case cmd_mod     : mod(); break;

			case cmd_jmp:
			{
				tint16 len;
				code >> len;
				code.jump(len);
			}
			break;
			case cmd_fjp:
			{
				tint16 len;
				code >> len;
				if (!get(-1).toBool(false)) code.jump(len);
				pop(1);
			}
			break;
			case cmd_call:
			case cmd_call_void:
			{
				tbyte nargs;
				code >> nargs;
				if (CallInfo* newCall = enterCall(nargs, cmd_call == cmd)) info = newCall;
			}
			break;
			case cmd_return:
			case cmd_return_void:
			{
				info = returnCall(cmd_return == cmd);
				if (info->closure && info->closure->isNative()) return;
			}
			break;
			case cmd_close_links:
			{
				tuint level = 0;
				code >> level;
				closeLinks(level);
			}
			break;
			}
		}
	}
}