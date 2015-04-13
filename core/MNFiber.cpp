#include "MNFiber.h"
#include "MNGlobal.h"
#include "MNObject.h"
#include "MNString.h"
#include "MNClosure.h"
#include "MNTable.h"
#include "MNArray.h"
#include "MNFunction.h"
#include "MNCompiler.h"
#include "MNUserData.h"
#include "MNClass.h"
#include "MNInstance.h"

#include <math.h>

void unaryOp(MNFiber* fiber, const tstring& opStr, const MNObject& val, MNObject& ret)
{
	MNObject meta  = val.toCollectable()->getMeta();
	MNObject field = MNObject::String(opStr);
	while (meta.toCollectable())
	{
		fiber->push(meta);
		fiber->push(field);
		bool success = fiber->load_raw_field();
		MNObject op = fiber->get(-1); fiber->pop(1);
		if (success && op.isClosure())
		{
			//! prepare dangling reference by expanding stack in "push"
			MNObject v = val;
			fiber->push(op);
			fiber->push(v);
			fiber->call(1, true);
			ret = fiber->get(-1);
			fiber->pop(1);
			break;
		}
		meta = meta.toCollectable()->getMeta();
	}
}

void binaryOp(MNFiber* fiber, const tstring& opStr, const MNObject& left, const MNObject& right, MNObject& ret)
{
	MNCollectable* collectable = left.toCollectable();
	MNObject meta  = collectable->getMeta();
	MNObject field = MNObject::String(opStr);
	while (meta.toCollectable())
	{
		fiber->push(meta);
		fiber->push(field);
		bool success = fiber->load_raw_field();
		MNObject op = fiber->get(-1); fiber->pop(1);
		if (success && op.isClosure())
		{
			//! prepare dangling reference by expanding stack in "push"
			MNObject l = left;
			MNObject r = right;
			fiber->push(op);
			fiber->push(l);
			fiber->push(r);
			fiber->call(2, true);
			ret = fiber->get(-1);
			fiber->pop(1);
			break;
		}
		meta = meta.toCollectable()->getMeta();
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

	m_status = MNFiber::Resume;

	if (global == NULL) global = new MNGlobal(this);

	link(m_global = global);
	if (stackSize() == 0) push_null();
	setAt(0, global->m_root->getAt(0));
}

MNFiber::~MNFiber()
{
	tlist<UpLink*>::iterator itor = m_openLinks.begin();
	for (; itor != m_openLinks.end(); ++itor) (*itor)->dec();

	m_stack.clear();
	while (m_info)
	{
		CallInfo* info = m_info->prev;
		m_info->closure = NULL;
		delete m_info;
		m_info = info;
	}
}

MNGlobal* MNFiber::global() const
{
	return m_global;
}

bool MNFiber::compileFile(MNObject& func, const tstring& path)
{
	push_string(path); //! [path]
	push_string("rootPath"); //! [path "rootPath"]
	load_global();           //! [path rootPath]
	if (get(-1).isString()) //! [path rootPath]
	{
		swap();//! [rootPath path]
		add(); //! [fullPath]
	}
	else pop(1); //! [path]

	MNObject fullPath = get(-1);
	if (!fullPath.isString()) return false;

	push_string("cache"); //! [fullPath "cache"]
	load_global();        //! [fullPath cache]
	MNObject cache = get(-1);

	swap();               //! [cache fullPath]
	load_field();         //! [func]

	func = get(-1);
	pop(1); //! []

	if (!func.isFunction()) //! there is no cached func
	{
		MNCompiler compiler;
		compiler.m_lexer.openFile(fullPath.toString()->ss().str());
		if (!compiler.build(func)) return false;

		push(cache);
		push(fullPath);
		push(func);
		store_field();
	}
	return true;
}

bool MNFiber::dofile(const tstring& path)
{
	MNObject func;
	if (!compileFile(func, path)) return false;

	push_closure(NULL); //! [closure]
	MNClosure* closure = get(-1).toClosure();
	closure->setFunc(func);

	load_stack(0);  //! [closure global]
	call(1, false);
	return true;
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

void MNFiber::push_integer(tinteger val)
{
	push(MNObject::Int(val));
}

void MNFiber::push_real(treal val)
{
	push(MNObject::Real(val));
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
	push_string("closure");
	load_global();

	MNClosure* closure = new MNClosure(MNObject::CFunction(val));
	closure->setMeta(get(-1));
	pop(1);

	MNObject obj(TObjectType::Closure, closure->link(global())->getReferrer());
	push(obj);
}

void MNFiber::push_table(tsize size)
{
	push_string("table");
	load_global();

	MNTable* table = new MNTable(size);
	table->setMeta(get(-1));
	pop(1);

	MNObject obj(TObjectType::Table, table->link(global())->getReferrer());
	push(obj);
}

void MNFiber::push_array(tsize size)
{
	push_string("array");
	load_global();

	MNArray* array = new MNArray(size);
	array->setMeta(get(-1));
	pop(1);

	MNObject obj(TObjectType::Array, array->link(global())->getReferrer());
	push(obj);
}

void MNFiber::push_const(tsize idx)
{
	push(getConst(idx));
}

void* MNFiber::push_userdata(tsize size)
{
	if (size > 0)
	{
		MNUserData* userData = new MNUserData(size);
		MNObject ud(TObjectType::UserData, userData->link(global())->getReferrer());
		push(ud);
		return userData->getData();
	}
	push_null();
	return NULL;
}

void MNFiber::push(const MNObject& val)
{
	const tsize expand = 32;
	if (m_info->end >= (tint32)m_stack.size())
	{
		MNObject val2 = val;
		m_stack.resize(m_info->end + expand);
		setAt(m_info->end++, val2);
	}
	else
	{
		setAt(m_info->end++, val);
	}
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

bool MNFiber::load_raw_field()
{
	const MNObject& obj = get(-2);
	const MNObject& key = get(-1);

	bool ret = false;
	MNObject val;
	switch (obj.getType())
	{
	case TObjectType::Table: ret = obj.toTable()->tryGet(key, val); break;
	case TObjectType::Array: ret = obj.toArray()->tryGet(key, val); break;
	case TObjectType::Class: ret = obj.toClass()->tryGet(key, val); break;
	case TObjectType::Instance: ret = obj.toInstance()->tryGet(key, val); break;
	}
	pop(2);
	push(val);
	return ret;
}

tboolean MNFiber::store_raw_field(tboolean insert)
{
	const MNObject& obj = get(-3);
	const MNObject& key = get(-2);
	const MNObject& val = get(-1);

	bool ret = false;
	switch (obj.getType())
	{
	case TObjectType::Table: 
		if (insert) ret = obj.toTable()->insert(key, val);
		else ret = obj.toTable()->trySet(key, val); 
		break;
	case TObjectType::Array: ret = obj.toArray()->trySet(key, val); break;
	case TObjectType::Instance: ret = obj.toInstance()->trySet(key, val); break;
	}
	pop(3);
	return ret;
}

void MNFiber::load_field()
{
	const MNObject& obj = get(-2);
	const MNObject& key = get(-1);

	bool ret = false;
	MNObject val;
	switch (obj.getType())
	{
	case TObjectType::Table: ret = obj.toTable()->tryGet(key, val); break;
	case TObjectType::Array: ret = obj.toArray()->tryGet(key, val); break;
	case TObjectType::Class: ret = true; obj.toClass()->tryGet(key, val); break;
	case TObjectType::Instance: ret = true; obj.toInstance()->tryGet(key, val); break;
	}

	MNCollectable* collectable = NULL;
	if (!ret && (collectable = obj.toCollectable()))
	{
		MNObject meta  = collectable->getMeta();
		MNObject field = MNObject::String("->");
		while (meta.toCollectable())
		{
			push(meta);
			push(field);
			load_raw_field();
			MNObject op = get(-1); pop(1);
			if (op.isClosure())
			{
				push(op);
				push(obj);
				push(key);
				call(2, true);
				val = get(-1);
				pop(1);
				break;
			}
			else
			{
				push(op);
				push(key);
				bool success = load_raw_field();
				val = get(-1); pop(1);
				if (success) break;
			}
			meta = meta.toCollectable()->getMeta();
		}
	}

	pop(2);
	push(val);
}

void MNFiber::store_field(tboolean insert)
{
	const MNObject& obj = get(-3);
	const MNObject& key = get(-2);
	const MNObject& val = get(-1);

	bool ret = false;
	switch (obj.getType())
	{
	case TObjectType::Table: 
		if (insert) ret = obj.toTable()->insert(key, val); 
		else ret = obj.toTable()->trySet(key, val); 
		break;
	case TObjectType::Array: ret = obj.toArray()->trySet(key, val); break;
	case TObjectType::Instance: ret = true; obj.toInstance()->trySet(key, val); break;
	}

	MNCollectable* collectable = NULL;
	if (!ret && (collectable = obj.toCollectable()))
	{
		MNObject meta  = collectable->getMeta();
		MNObject field = MNObject::String("-<");
		while (meta.toCollectable())
		{
			push(meta);
			push(field);
			load_raw_field();
			MNObject op = get(-1); pop(1);
			if (op.isClosure())
			{
				push(op);
				push(obj);
				push(key);
				push(val);
				call(3, false);
				break;
			}
			else
			{
				push(op);
				push(key);
				push(val);
				if (store_raw_field(false)) break;
			}
			meta = meta.toCollectable()->getMeta();
		}
	}
	pop(3);
}

void MNFiber::load_global()
{
	push(getAt(0));
	swap();
	load_field();
}

void MNFiber::store_global()
{
	push(getAt(0));
	up(1, 2);
	pop(1);
	store_field();
}

void MNFiber::set_meta()
{
	const MNObject& obj  = get(-2);
	const MNObject& meta = get(-1);

	MNCollectable* collectable = obj.toCollectable();
	if (collectable) collectable->setMeta(meta);
	
	pop(2);
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

	bool ret = false;
	switch (left.getType())
	{
	case TObjectType::Null     : ret = (right.isNull()); break;
	case TObjectType::Pointer  : ret = (left.toPointer() == right.toPointer()); break;
	case TObjectType::Boolean  : ret = (left.toBool() == right.toBool()); break;
	case TObjectType::CFunction: ret = (left.toCFunction() == right.toCFunction()); break;
	case TObjectType::String   : if (right.isString()) ret = left.toString()->ss() == right.toString()->ss(); break;
	case TObjectType::Int:
		{
			if (right.isInt()) ret = (left.toInt() == right.toInt());
			else if (right.isReal()) ret = (left.toReal() == right.toReal());
		}
		break;
	case TObjectType::Real:
		{
			ret = (left.toReal() == right.toReal());
		}
		break;
	case TObjectType::UserData:
	case TObjectType::Table:
		{
			MNObject val;
			binaryOp(this, "==", left, right, val);
			ret = val.toBool();
		}
		break;
	}
	push(MNObject::Bool(ret));
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
	case TObjectType::Real:
		{
			if (right.isInt()) ret = MNObject::Bool(left.toInt() < right.toInt());
			else if (right.isReal()) ret = MNObject::Bool(left.toReal() < right.toReal());
		}
		break;
	case TObjectType::UserData:
	case  TObjectType::Table:
		{
			binaryOp(this, "<", left, right, ret);
		}
		break;
	}
	push(ret);
}

void MNFiber::and_with()
{
	MNObject left = get(-2);
	MNObject right = get(-1);
	pop(2);

	push(MNObject::Bool((left.toBool() && right.toBool())));
}

void MNFiber::or_with()
{
	MNObject left = get(-2);
	MNObject right = get(-1);
	pop(2);

	push(MNObject::Bool(left.toBool() || right.toBool()));
}

void MNFiber::tostring()
{
	const MNObject& object = get(-1);

	MNObject str = MNObject::String("[null:null]");
	switch (object.getType())
	{
	case TObjectType::Class    : str = MNObject::Format("[class: %p]", object.toRaw()); break;
	case TObjectType::Instance : str = MNObject::Format("[instance: %p]", object.toRaw()); break;
	case TObjectType::Array    : str = MNObject::Format("[array: %p]", object.toArray()); break;
	case TObjectType::Table    : str = MNObject::Format("[table: %p]", object.toTable()); break;
	case TObjectType::Pointer  : str = MNObject::Format("[pointer: %p]", object.toPointer()); break;
	case TObjectType::CFunction: str = MNObject::Format("[cfunction: %p]", object.toCFunction()); break;
	case TObjectType::Function : str = MNObject::Format("[function: %p]", object.toFunction()); break;
	case TObjectType::Closure  : str = MNObject::Format("[closure: %p]", object.toClosure()); break;
	case TObjectType::String   : str = object; break;
	case TObjectType::Int      : str = MNObject::Format("%d", object.toInt()); break;
	case TObjectType::Real     : str = MNObject::Format("%f", object.toReal()); break;
	case TObjectType::Boolean  : str = MNObject::Format("%s", object.toBool() ? "true" : "false"); break;
	case TObjectType::UserData : 
		{
			MNUserData* ud = object.toUserData();
			str = MNObject::Format("[userdata: %p, size: %d]", ud, ud->getSize());
		}
		break;
	}

	pop(1);
	push(str);
}

void MNFiber::inc()
{

}

void MNFiber::dec()
{
}

void MNFiber::neg()
{
	const MNObject& val = get(-1);

	MNObject ret;
	switch (val.getType())
	{
	case TObjectType::Int   : ret = MNObject::Int(-val.toInt()); break;
	case TObjectType::Real  : ret = MNObject::Real(-val.toReal());  break;
	case TObjectType::UserData :
	case  TObjectType::Table: unaryOp(this, "-", val, ret); break;
	}
	pop(1);
	push(ret);
}

void MNFiber::add()
{
	const MNObject& left  = get(-2);
	const MNObject& right = get(-1);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int:
	{
		if (right.isInt())   ret = MNObject::Int(left.toInt() + right.toInt());
		if (right.isReal()) ret = MNObject::Real(left.toReal() + right.toReal());
	}
	break;
	case TObjectType::Real: if (right.isReal() || right.isInt()) ret = MNObject::Real(left.toReal() + right.toReal()); break;
	case TObjectType::String:
	{
		tostring();
		MNObject strObj1 = get(-1);

		MNString* str1 = left.toString();
		MNString* str2 = strObj1.toString();
		ret = MNObject::Format("%s%s", str1->ss().str().c_str(), str2->ss().str().c_str());
	}
	break;
	case TObjectType::UserData :
	case  TObjectType::Table:
	{
		binaryOp(this, "+", left, right, ret);
	}
	break;
	}

	pop(2);
	push(ret);
}

void MNFiber::sub()
{
	const MNObject& left  = get(-2);
	const MNObject& right = get(-1);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int:
	{
		if (right.isInt())   ret = MNObject::Int(left.toInt() - right.toInt());
		if (right.isReal()) ret = MNObject::Real(left.toReal() - right.toReal());
	}
	break;
	case TObjectType::Real: if (right.isReal() || right.isInt()) ret = MNObject::Real(left.toReal() - right.toReal()); break;
	case TObjectType::UserData :
	case TObjectType::Table:
	{
		binaryOp(this, "-", left, right, ret);
	}
	break;
	}

	pop(2);
	push(ret);
}

void MNFiber::mul()
{
	const MNObject& left  = get(-2);
	const MNObject& right = get(-1);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int:
	{
		if (right.isInt())   ret = MNObject::Int(left.toInt() * right.toInt());
		if (right.isReal()) ret = MNObject::Real(left.toReal() * right.toReal());
	}
	break;
	case TObjectType::Real: if (right.isReal() || right.isInt()) ret = MNObject::Real(left.toReal() * right.toReal()); break;
	case TObjectType::UserData :
	case TObjectType::Table:
	{
		binaryOp(this, "*", left, right, ret);
	}
	break;
	}

	pop(2);
	push(ret);
}

void MNFiber::div()
{
	const MNObject& left  = get(-2);
	const MNObject& right = get(-1);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int:
	{
		if (right.isInt()   && right.toInt() != 0)   ret = MNObject::Int(left.toInt() / right.toInt());
		if (right.isReal() && right.toReal() != 0) ret = MNObject::Real(left.toReal() / right.toReal());
	}
	break;
	case TObjectType::Real:
	{
		if (right.isReal() || right.isInt())
		{
			if (right.toReal() != 0.0f) ret = MNObject::Real(left.toReal() / right.toReal());
		}
	}
	break;
	case TObjectType::UserData :
	case TObjectType::Table:
	{
		binaryOp(this, "/", left, right, ret);
	}
	break;
	}

	pop(2);
	push(ret);
}

void MNFiber::mod()
{
	const MNObject& left  = get(-2);
	const MNObject& right = get(-1);

	MNObject ret;
	switch (left.getType())
	{
	case TObjectType::Int:
	{
		if (right.isInt() && right.toInt() != 0)   ret = MNObject::Int(left.toInt() % right.toInt());
		if (right.isReal() && right.toReal() != 0) ret = MNObject::Real(fmod(left.toReal(), right.toReal()));
	}
	break;
	case TObjectType::Real:
	{
		if (right.isReal() || right.isInt())
		{
			if (right.toReal() != 0.0f) ret = MNObject::Real(fmod(left.toReal(), right.toReal()));
		}
	}
	break;
	case TObjectType::UserData :
	case TObjectType::Table:
	{
		binaryOp(this, "%", left, right, ret);
	}
	break;
	}

	pop(2);
	push(ret);
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
			tsize i = ov->index;
			ul->close();
			ul->dec();
			itor = m_openLinks.erase(itor);
			continue;
		}
		++itor;
	}
	while (index < m_info->end) setAt(index++, MNObject::Null());
}

tsize MNFiber::localSize() const
{
	return m_info->end - m_info->begin;
}

tsize MNFiber::stackSize() const
{
	return m_info->end;
}

void  MNFiber::setStatus(tbyte status)
{
	m_status = status;
}

tbyte MNFiber::getStatus() const
{
	return m_status;
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
	if (enterCall(nargs, ret)) excuteCall();
}

MNFiber::CallInfo* MNFiber::enterCall(tuint nargs, bool ret)
{
	tuint clsIndex = m_info->end - (nargs + 1);
	MNObject clsVal = getAt(clsIndex);
	MNClosure* cls = clsVal.toClosure();
	if (!cls)
	{
		pop(nargs + 1);
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

	if (!cls->getThis().isNull()) setAt(m_info->begin, cls->getThis());

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
	closeLinks(0);

	tsize count = (m_info->end - m_info->prev->end);
	tsize end = m_info->end;
	for (tsize i=0; i < count; ++i)
	{
		setAt(end-(i+1), MNObject::Null());
	}

	CallInfo* info = m_info;
	m_info = info->prev;
	delete info;

	if (needRet) push(obj);

	return m_info;
}

tint32 MNFiber::excuteCall()
{
	CallInfo* info     = m_info; 
	CallInfo* prevInfo = m_info->prev;
	//! script
	while (info != prevInfo)
	{
		MNClosure* closure = info->closure;
		if (!closure) break;
		if (closure->getFunc().isNull())
		{
			info = returnCall(false);
			break;
		}

		if (closure->isNative())
		{
			TCFunction func = closure->getFunc().toCFunction();
			bool ret = (func) ? func(this) : false;
			info = returnCall(ret);
			continue;;
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
					push_real(_float);
				}
				break;
			case cmd_push_int:
				{
					tint32 _int;
					code >> _int;
					push_integer(_int);
				}
				break;
			case cmd_push_table:
				{
					tuint16 size = 0;
					code >> size;
					push_table(size); 
				}
				break;
			case cmd_push_array:
				{
					tuint16 size = 0;
					code >> size;
					push_array(size);
				}
				break;
			case cmd_push_closure:
				{
					tuint16 funcIndex, upLinkSize;
					code >> funcIndex >> upLinkSize;

					push_closure(NULL);
					MNClosure* cls = get(-1).toClosure();
					cls->setFunc(getConst(funcIndex));

					while (upLinkSize--)
					{
						tbyte loc;
						tuint16 index;
						code >> loc >> index;
						UpLink* ul = (loc == cmd_load_stack) ? openLink(index) : closure->getLink(index);
						cls->addLink(ul);
					}
				}
				break;
			case cmd_new_class :
				{
					tuint16 nfield = 0;
					code >> nfield;

					MNObject _super = get(-1); pop(1);
					MNClass* _class = new MNClass(nfield, _super);
					MNObject classObj = MNObject::Referrer(_class->link(global())->getReferrer());
					_class->addStatic(MNObject::String("->"), classObj); //! default setting
					push(classObj);
				}
				break;
			case cmd_add_class_static:
			case cmd_add_class_field:
				{
					MNObject classObj = get(-3);
					MNObject name     = get(-2);
					MNObject val      = get(-1);
					MNClass* _class   = classObj.toClass();
					if (cmd == cmd_add_class_field) _class->addField(name, val);
					else _class->addStatic(name, val);
					pop(3);
				}
				break;
			case cmd_new_inst:
				{
					tuint16 nargs = 0;
					code >> nargs;
					MNObject classObj = get(-(nargs+1));
					if (MNClass* _class = classObj.toClass())
					{
						MNInstance* inst = new MNInstance(classObj);
						inst->link(global());
						MNObject instObj = MNObject::Referrer(inst->getReferrer());
						push(classObj);
						push_string("constructor");
						load_field();
						MNObject constructor = get(-1); pop(1);
						set(-(nargs+1), constructor);
						set(-nargs, instObj);
						call(nargs, false);
						push(instObj);
					}
					else
					{
						pop(nargs+1);
						push_null();
					}
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
			case cmd_load_method:
				{
					//! [object field]
					MNObject object = get(-2);
					load_field(); //! [closure]
					push(object); //! [closure object]
				}
				break;
			case cmd_load_this : load_stack(0); break;
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
			case cmd_store_field : store_field(false); break;
			case cmd_insert_field: store_field(true); break;
			case cmd_load_global:
				{
					tuint16 index;
					code >> index;

					push_const(index);
					load_global();
				}
				break;
			case cmd_store_global:
				{
					tuint16 index;
					code >> index;

					MNObject val = get(-1);
					pop(1);

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
					pop(1);
					push(MNObject::Bool(!ret.toBool()));
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
			case cmd_and: and_with(); break;
			case cmd_or : or_with();  break;
			case cmd_tostring: tostring(); break;
			case cmd_neg     : neg(); break;
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
					tint32 selfIdx = -tint32(nargs);
					const MNObject& self = get(selfIdx);
					if (self.isClass()) set(selfIdx, get(0));
					if (CallInfo* newCall = enterCall(nargs, cmd_call == cmd)) info = newCall;
				}
				break;
			case cmd_return:
			case cmd_return_void:
				{
					info = returnCall(cmd_return == cmd);
				}
				break;
			case cmd_close_links:
				{
					tuint16 level = 0;
					code >> level;
					closeLinks(level);
				}
				break;
			case cmd_yield_void:
				push_null();
			case cmd_yield:
				return MNFiber::Suspend;
			case cmd_break_point:
				{
					int a = 0;
				}
				break;
			}
		}
	}
	return MNFiber::Stop;
}