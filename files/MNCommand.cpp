//#include "MNCommand.h"
//#include "MNFiber.h"
//#include "MNGlobal.h"
//#include "MNObject.h"
//#include "MNString.h"
//#include "MNClosure.h"
//#include "MNTable.h"
//#include "MNArray.h"
//
//void binomalOp(MNFiber* fiber, const tstring& opStr, const MNObject& left, const MNObject& right, MNObject& ret)
//{
//	MNCollectable* collectable = left.toCollectable();
//	MNObject meta = collectable->getMeta();
//	MNObject op = MNObject::String(opStr);
//	ret = MNObject::Null();
//	while (true)
//	{
//		if (meta.isTable())
//		{
//			meta.toTable()->tryGet(op, ret);
//			if (ret.isClosure()) meta = ret;
//		}
//
//		if (meta.isClosure())
//		{
//			fiber->push(meta);
//			fiber->push(left);
//			fiber->push(right);
//			MNCommand(fiber).call(2, true);
//			ret = fiber->get(-1);
//			fiber->pop(1);
//			break;
//		}
//
//		collectable = meta.toCollectable();
//		if (collectable) meta = collectable->getMeta();
//		else break;
//	}
//}
//
//void MNCommand::pop(tsize count)
//{
//	m_fiber->pop(count);
//}
//
//void MNCommand::push_null()
//{
//	m_fiber->push(MNObject::Null());
//}
//
//void MNCommand::push_int(tint32 val)
//{
//	m_fiber->push(MNObject::Int(val));
//}
//
//void MNCommand::push_float(tfloat val)
//{
//	m_fiber->push(MNObject::Float(val));
//}
//
//void MNCommand::push_string(const tstring& val)
//{
//	const tsize shortStrLen = 4;
//	if (val.length() > shortStrLen)
//	{
//		MNObject ret;
//		m_fiber->global()->getString(ret, val);
//		m_fiber->push(ret);
//	}
//	else
//	{
//		m_fiber->push(MNObject::String(val));
//	}
//}
//
//void MNCommand::push_pointer(void* val)
//{
//	m_fiber->push(MNObject::Pointer(val));
//}
//
//void MNCommand::push_bool(tboolean val)
//{
//	m_fiber->push(MNObject::Bool(val));
//}
//
//void MNCommand::push_closure(TFunction val)
//{
//	MNClosure* closure = new MNClosure(MNObject::CFunction(val));
//	MNReferrer* ref = closure->link(m_fiber->global())->getReferrer();
//	MNObject obj(TObjectType::Closure, ref);
//	m_fiber->push(obj);
//}
//
//void MNCommand::push_table()
//{
//	MNTable* table = new MNTable();
//	MNReferrer* ref = table->link(m_fiber->global())->getReferrer();
//	MNObject obj(TObjectType::Table, ref);
//	m_fiber->push(obj);
//}
//
//void MNCommand::push_array()
//{
//	push_string("array");
//	load_global();
//
//	MNArray* array = new MNArray();
//	array->setMeta(m_fiber->get(-1));
//	m_fiber->pop(1);
//
//	MNObject obj(TObjectType::Array, array->getReferrer());
//	m_fiber->push(obj);
//}
//
//tint32       MNCommand::to_int(tint32 idx, tint32 def)
//{
//	const MNObject& obj = m_fiber->get(idx);
//	return obj.toInt(def);
//}
//
//tfloat       MNCommand::to_float(tint32 idx, tfloat def)
//{
//	const MNObject& obj = m_fiber->get(idx);
//	return obj.toFloat(def);
//}
//
//const tchar* MNCommand::to_string(tint32 idx, const tchar* def)
//{
//	const MNObject& obj = m_fiber->get(idx);
//	MNString* str = obj.toString();
//	if (str) return str->ss().str().c_str();
//	return def;
//}
//
//void*        MNCommand::to_pointer(tint32 idx, void* def)
//{
//	const MNObject& obj = m_fiber->get(idx);
//	if (obj.isPointer()) return obj.toPointer();
//	return def;
//}
//
//tboolean     MNCommand::to_bool(tint32 idx, tboolean def)
//{
//	const MNObject& obj = m_fiber->get(idx);
//	return obj.toBool(def);
//}
//
//void  MNCommand::load_stack(tint32 idx)
//{
//	m_fiber->push(m_fiber->get(idx));
//}
//
//void  MNCommand::store_stack(tint32 idx)
//{
//	m_fiber->set(idx, m_fiber->get(-1));
//	m_fiber->pop(1);
//}
//
//void MNCommand::load_field()
//{
//	MNObject obj = m_fiber->get(-2);
//	MNObject key = m_fiber->get(-1);
//	m_fiber->pop(2);
//
//	MNObject ret;
//	tboolean succeed = false;
//	switch (obj.getType())
//	{
//	case TObjectType::Table:
//	{
//		MNTable* table = obj.toTable();
//		succeed = table->tryGet(key, ret);
//	}
//	break;
//	}
//
//	MNCollectable* collectable = obj.toCollectable();
//	if (!succeed)
//	{
//		MNObject meta = collectable->getMeta();
//		MNObject op   = MNObject::String("->");
//		while (true)
//		{
//			if (meta.isTable())
//			{
//				MNTable* metaTable = meta.toTable();
//				if (metaTable->tryGet(key, ret)) break;
//				metaTable->tryGet(op, ret);
//				if (ret.isClosure()) meta = ret;
//				ret = MNObject::Null();
//			}
//
//			if (meta.isClosure())
//			{
//				m_fiber->push(meta);
//				m_fiber->push(obj);
//				m_fiber->push(key);
//				call(2, true);
//				ret = m_fiber->get(-1);
//				m_fiber->pop(1);
//				break;
//			}
//
//			collectable = meta.toCollectable();
//			if (collectable) meta = collectable->getMeta();
//			else break;
//		}
//	}
//
//	m_fiber->push(ret);
//}
//
//void MNCommand::store_field()
//{
//	MNObject obj = m_fiber->get(-3);
//	MNObject key = m_fiber->get(-2);
//	MNObject val = m_fiber->get(-1);
//	m_fiber->pop(3);
//
//	MNObject ret;
//	tboolean succeed = false;
//	switch (obj.getType())
//	{
//	case TObjectType::Table:
//	{
//		MNTable* table = obj.toTable();
//		succeed = table->insert(key, val);
//	}
//	break;
//	}
//
//	MNCollectable* collectable = obj.toCollectable();
//	if (!succeed)
//	{
//		MNObject meta = collectable->getMeta();
//		MNObject op   = MNObject::String("-<");
//		while (true)
//		{
//			if (meta.isTable())
//			{
//				MNTable* metaTable = meta.toTable();
//				if (metaTable->trySet(key, ret)) break;
//				metaTable->tryGet(op, ret);
//				if (ret.isClosure()) meta = ret;
//				ret = MNObject::Null();
//			}
//
//			if (meta.isClosure())
//			{
//				m_fiber->push(meta);
//				m_fiber->push(obj);
//				m_fiber->push(key);
//				m_fiber->push(val);
//				call(3, false);
//				break;
//			}
//
//			collectable = meta.toCollectable();
//			if (collectable) meta = collectable->getMeta();
//			else break;
//		}
//	}
//}
//
//void MNCommand::load_global()
//{
//	m_fiber->push(m_fiber->getAt(0));
//	swap();
//	load_field();
//}
//
//void MNCommand::store_global()
//{
//	m_fiber->push(m_fiber->getAt(0));
//	up(1, 2);
//	store_field();
//}
//
//void MNCommand::set_meta()
//{
//	MNObject obj  = m_fiber->get(-2);
//	MNObject meta = m_fiber->get(-1);
//	m_fiber->pop(2);
//
//	MNCollectable* collectable = obj.toCollectable();
//	if (collectable) collectable->setMeta(meta);
//}
//
//void MNCommand::get_meta()
//{
//	MNObject obj = m_fiber->get(-1);
//	m_fiber->pop(1);
//
//	MNCollectable* collectable = obj.toCollectable();
//	if (collectable) m_fiber->push(collectable->getMeta());
//	else m_fiber->push(MNObject::Null());
//}
//
//void MNCommand::up(tsize n, tsize x)
//{
//	if (n == 0) return;
//
//	tsize nx = n + x;
//	for (tint i = 0; i < (tint32)n; ++i) m_fiber->push(MNObject::Null());
//	for (tint i = 1; i <= (tint32)nx; ++i) m_fiber->set(-i, m_fiber->get(-i - n));
//	for (tint i = 1; i <= (tint32)n; ++i) m_fiber->set(-i - nx, m_fiber->get(-i));
//}
//
//void MNCommand::swap()
//{
//	MNObject top = m_fiber->get(-1);
//	m_fiber->set(-1, m_fiber->get(-2));
//	m_fiber->set(-2, top);
//}
//
//void MNCommand::equals()
//{
//	MNObject left  = m_fiber->get(-1);
//	MNObject right = m_fiber->get(-2);
//	m_fiber->pop(2);
//
//	MNObject ret;
//	switch (left.getType())
//	{
//	case TObjectType::Null    : ret = MNObject::Bool(right.isNull()); break;
//	case TObjectType::Pointer : ret = MNObject::Bool(left.toPointer() == right.toPointer()); break;
//	case TObjectType::Boolean : ret = MNObject::Bool(left.toBool() == right.toBool()); break;
//	case TObjectType::Function: ret = MNObject::Bool(left.toFunction() == right.toFunction()); break;
//	case TObjectType::String: if (right.isString()) ret = left.toString()->ss() == right.toString()->ss(); break;
//	case TObjectType::Int: case TObjectType::Float:
//	{
//		if (right.isInt()) ret = MNObject::Bool(left.toInt() == right.toInt());
//		else if (right.isFloat()) ret = MNObject::Bool(left.toFloat() == right.toFloat());
//	}
//	break;
//	case TObjectType::Table:
//	{
//		binomalOp(m_fiber, "==", left, right, ret);
//	}
//	break;
//	}
//	m_fiber->push(ret);
//}
//
//void MNCommand::tostring()
//{
//	MNObject object = m_fiber->get(-1);
//	m_fiber->pop(1);
//
//	MNObject str = MNObject::String("[null:null]");
//	switch (object.getType())
//	{
//	case TObjectType::Pointer : str = MNObject::Format("[pointer: %p]", object.toPointer()); break;
//	case TObjectType::Boolean : str = MNObject::Format("[boolean: %s]", object.toBool()? "true":"false"); break;
//	case TObjectType::Function: str = MNObject::Format("[cfunction: %p]", object.toFunction()); break;
//	case TObjectType::String  : str = object; break;
//	case TObjectType::Int     : str = MNObject::Format("%d", object.toInt()); break;
//	case TObjectType::Float   : str = MNObject::Format("%f", object.toFloat()); break;
//	}
//
//	m_fiber->push(str);
//}
//
//void MNCommand::add()
//{
//	MNObject left  = m_fiber->get(-1);
//	MNObject right = m_fiber->get(-2);
//	m_fiber->pop(2);
//
//	MNObject ret;
//	switch (left.getType())
//	{
//	case TObjectType::Int  : if (right.isFloat() || right.isInt()) ret = MNObject::Int(left.toInt() + right.toInt()); break;
//	case TObjectType::Float: if (right.isFloat() || right.isInt()) ret = MNObject::Float(left.toFloat() + right.toFloat()); break;
//	case TObjectType::String:
//	{
//		m_fiber->push(right);
//		tostring();
//		MNObject strObj1 = m_fiber->get(-1);
//		m_fiber->pop(1);
//
//		MNString* str1 = left.toString();
//		MNString* str2 = strObj1.toString();
//		ret = MNObject::Format("%s%s", str1->ss().str(), str2->ss().str());
//	}
//	break;
//	case  TObjectType::Table:
//	{
//		binomalOp(m_fiber, "+", left, right, ret);
//	}
//	break;
//	}
//
//	m_fiber->push(ret);
//}
//
//void MNCommand::sub()
//{
//	MNObject left  = m_fiber->get(-1);
//	MNObject right = m_fiber->get(-2);
//	m_fiber->pop(2);
//
//	MNObject ret;
//	switch (left.getType())
//	{
//	case TObjectType::Int  : if (right.isFloat() || right.isInt()) ret = MNObject::Int(left.toInt() - right.toInt()); break;
//	case TObjectType::Float: if (right.isFloat() || right.isInt()) ret = MNObject::Float(left.toFloat() - right.toFloat()); break;
//	case TObjectType::Table:
//	{
//		binomalOp(m_fiber, "-", left, right, ret);
//	}
//	break;
//	}
//
//	m_fiber->push(ret);
//}
//
//void MNCommand::mul()
//{
//	MNObject left  = m_fiber->get(-1);
//	MNObject right = m_fiber->get(-2);
//	m_fiber->pop(2);
//
//	MNObject ret;
//	switch (left.getType())
//	{
//	case TObjectType::Int: if (right.isFloat() || right.isInt()) ret = MNObject::Int(left.toInt() * right.toInt()); break;
//	case TObjectType::Float: if (right.isFloat() || right.isInt()) ret = MNObject::Float(left.toFloat() * right.toFloat()); break;
//	case TObjectType::Table:
//	{
//		binomalOp(m_fiber, "*", left, right, ret);
//	}
//	break;
//	}
//
//	m_fiber->push(ret);
//}
//
//void MNCommand::div()
//{
//	MNObject left  = m_fiber->get(-1);
//	MNObject right = m_fiber->get(-2);
//	m_fiber->pop(2);
//
//	MNObject ret;
//	switch (left.getType())
//	{
//	case TObjectType::Int:
//	{
//		if (right.isFloat() || right.isInt())
//		{
//			if (right.toInt() != 0) ret = MNObject::Int(left.toInt() / right.toInt());
//		}
//	}
//	break;
//	case TObjectType::Float:
//	{
//		if (right.isFloat() || right.isInt())
//		{
//			if (right.toFloat() != 0.0f) ret = MNObject::Float(left.toFloat() / right.toFloat());
//		}
//	}
//	break;
//	case TObjectType::Table:
//	{
//		binomalOp(m_fiber, "/", left, right, ret);
//	}
//	break;
//	}
//
//	m_fiber->push(ret);
//}
//
//void MNCommand::mod()
//{
//	MNObject left  = m_fiber->get(-1);
//	MNObject right = m_fiber->get(-2);
//	m_fiber->pop(2);
//
//	MNObject ret;
//	switch (left.getType())
//	{
//	case TObjectType::Int:
//	{
//		if (right.isFloat() || right.isInt())
//		{
//			if (right.toInt() != 0) ret = MNObject::Int(left.toInt() % right.toInt());
//		}
//	}
//	break;
//	case TObjectType::Float:
//	{
//		if (right.isFloat() || right.isInt())
//		{
//			if (right.toFloat() != 0.0f) ret = MNObject::Float(fmod(left.toFloat(), right.toFloat()));
//		}
//	}
//	break;
//	case TObjectType::Table:
//	{
//		binomalOp(m_fiber, "%", left, right, ret);
//	}
//	break;
//	}
//
//	m_fiber->push(ret);
//}
//
//void MNCommand::call(tsize nargs, bool ret)
//{
//	MNFiber::CallInfo* info = m_fiber->enterClosure(nargs, ret);
//	if (!info) return;
//
//	MNClosure* closure = info->closure;
//	if (closure->isNative())
//	{
//		TFunction func = closure->getFunc().toFunction();
//		m_fiber->returnClosure(func(m_fiber));
//	}
//}