#include "MNObject.h"
#include "MNReferrer.h"
#include "MNString.h"
#include "MNCollectable.h"
#include "MNClosure.h"
#include "MNTable.h"
#include "MNArray.h"
#include "MNFunction.h"
#include "MNFiber.h"

#include <stdarg.h>

MNObject MNObject::String(const tstring& str)
{
	MNString* strval = new MNString(str);
	return MNObject(TObjectType::String, strval->getReferrer());
}

MNObject MNObject::String(const thashstring& str)
{
	MNString* strval = new MNString(str);
	return MNObject(TObjectType::String, strval->getReferrer());
}

MNObject MNObject::Format(const tchar* format, ...)
{
	static const tuint bufSize = 256;
	tchar buf[bufSize] = { 0 };
	va_list args;
	va_start(args, format);
	vsprintf(&buf[0], format, args);
	va_end(args);
	return String(&buf[0]);
}
MNObject MNObject::Referrer(MNReferrer* ref)
{
	if (!ref) return MNObject::Null();

	MNCountable* obj = ref->getObject();
	const MNRtti* rtti = obj->queryRtti();
	TObjectType type = TObjectType::Null;
	if (rtti == MNArray::getRtti()) type = TObjectType::Array;
	else if (rtti == MNTable::getRtti()) type = TObjectType::Table;
	else if (rtti == MNClosure::getRtti()) type = TObjectType::Closure;
	else if (rtti == MNFunction::getRtti()) type = TObjectType::Function;
	else if (rtti == MNString::getRtti()) type = TObjectType::String;
	else if (rtti == MNFiber::getRtti()) type = TObjectType::Fiber;

	return MNObject(type, ref);
}
MNObject::MNObject()
	: valType(TObjectType::Null)
{

}

MNObject::MNObject(const MNObject& copy)
	: valType(TObjectType::Null)
{
	assign(copy);
}

MNObject::MNObject(tuint8 type)
	: valType(type)
{

}

MNObject::MNObject(tuint8 type, MNReferrer* ref)
	: valType(type)
{
	if (val._ref = ref)
	{
		ref->incWeak();
		ref->incHard();
	}
}

MNObject::~MNObject()
{
	assign(Null());
}

MNObject& MNObject::assign(const MNObject& right)
{
	if ((right.valType & TObjectType::Referrer) == 1)
	{
		MNReferrer* ref = right.val._ref;
		ref->incWeak();
		ref->incHard();
	}

	if ((valType & TObjectType::Referrer) == 1)
	{
		MNReferrer* ref = val._ref;
		ref->decHard();
		ref->decWeak();
	}

	valType = right.valType;
	val = right.val;
	return *this;
}

tint MNObject::getType() const
{
	if (valType == TObjectType::Referrer && val._ref->getHard() == 0) return TObjectType::Null;
	return valType;
}

bool MNObject::isNull() const
{
	return (getType() == TObjectType::Null);
}

bool MNObject::isPointer() const
{
	return (getType() == TObjectType::Pointer);
}

bool MNObject::isInt() const
{
	return (getType() == TObjectType::Int);
}

bool MNObject::isFloat() const
{
	return (getType() == TObjectType::Float);
}

bool MNObject::isBool() const
{
	return (getType() == TObjectType::Boolean);
}

bool MNObject::isCFunction() const
{
	return (getType() == TObjectType::CFunction);
}

bool MNObject::isReferrer() const
{
	return ((valType & TObjectType::Referrer) == 1 && val._ref->getHard() != 0);
}

bool MNObject::isString() const
{
	return isReferrer() && (getType() == TObjectType::String);
}

bool MNObject::isClosure() const
{
	return isReferrer() && (getType() == TObjectType::Closure);
}

bool MNObject::isTable() const
{
	return isReferrer() && (getType() == TObjectType::Table);
}

bool MNObject::isArray() const
{
	return isReferrer() && (getType() == TObjectType::Array);
}

bool MNObject::isFunction() const
{
	return isReferrer() && (getType() == TObjectType::Function);
}

void*       MNObject::toRaw() const
{
	return val._pointer;
}

void*       MNObject::toPointer() const
{
	if (isPointer()) return val._pointer;
	return NULL;
}

tint32      MNObject::toInt(tint32 def) const
{
	if (isInt()) return val._int;
	if (isFloat()) return (tint32)val._float;
	return def;
}

tfloat      MNObject::toFloat(tfloat def) const
{
	if (isFloat()) return val._float;
	if (isInt()) return (tfloat)val._int;
	return def;
}

tboolean    MNObject::toBool(bool def) const
{
	if (isBool()) return val._bool;
	return def;
}

TCFunction   MNObject::toCFunction() const
{
	if (isCFunction()) return val._func;
	return NULL;
}

MNString*   MNObject::toString() const
{
	if (isString()) return mnrtti_cast<MNString>(toCountable());
	return NULL;
}

MNClosure*  MNObject::toClosure() const
{
	if (isClosure()) return mnrtti_cast<MNClosure>(toCountable());
	return NULL;
}

MNCountable* MNObject::toCountable() const
{
	if (isReferrer()) return val._ref->getObject();
	return NULL;
}

MNCollectable* MNObject::toCollectable() const
{
	if (isReferrer()) return mnrtti_cast<MNCollectable>(toCountable());
	return NULL;
}

MNTable*    MNObject::toTable() const
{
	if (isReferrer()) return mnrtti_cast<MNTable>(toCountable());
	return NULL;
}

MNArray*    MNObject::toArray() const
{
	if (isReferrer()) return mnrtti_cast<MNArray>(toCountable());
	return NULL;
}

MNFunction* MNObject::toFunction() const
{
	if (isReferrer()) return mnrtti_cast<MNFunction>(toCountable());
	return NULL;
}