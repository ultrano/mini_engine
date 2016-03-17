#include "MNObject.h"
#include "MNReferrer.h"
#include "MNString.h"
#include "MNCollectable.h"
#include "MNClosure.h"
#include "MNTable.h"
#include "MNArray.h"
#include "MNFunction.h"
#include "MNFiber.h"
#include "MNUserData.h"

#include <stdarg.h>

MNObject MNObject::String(const tstring& str)
{
	MNString* strval = new MNString(str);
	return MNObject(TObjectType::TString, strval->getReferrer());
}

MNObject MNObject::String(const thashstring& str)
{
	MNString* strval = new MNString(str);
	return MNObject(TObjectType::TString, strval->getReferrer());
}

MNObject MNObject::Format(const tchar* format, ...)
{
	static const tuint bufSize = 1024;
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
	TObjectType type = TObjectType::TNull;
	if (rtti == MNArray::getRtti())         type = TObjectType::TArray;
	else if (rtti == MNTable::getRtti())    type = TObjectType::TTable;
	else if (rtti == MNClosure::getRtti())  type = TObjectType::TClosure;
	else if (rtti == MNFunction::getRtti()) type = TObjectType::TFunction;
	else if (rtti == MNString::getRtti())   type = TObjectType::TString;
	else if (rtti == MNFiber::getRtti())    type = TObjectType::TFiber;
	else if (rtti == MNUserData::getRtti()) type = TObjectType::TUserData;

	return MNObject(type, ref);
}
MNObject::MNObject()
	: valType(TObjectType::TNull)
{

}

MNObject::MNObject(const MNObject& copy)
	: valType(TObjectType::TNull)
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
	if ((val._ref = ref))
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
	if (((valType & TObjectType::Referrer) == 1) && val._ref->getHard() == 0) return TObjectType::TNull;
	return valType;
}

thashtype MNObject::getHash() const
{
	switch (getType())
	{
	case TObjectType::TNull : return 0;
	case TObjectType::TString: return toString()->ss().hash();
	default: return (thashtype)toRaw();
	}
	return 0;
}

bool MNObject::isNull() const
{
	return (getType() == TObjectType::TNull);
}

bool MNObject::isPointer() const
{
	return (getType() == TObjectType::TPointer);
}

bool MNObject::isInt() const
{
	return (getType() == TObjectType::TInt);
}

bool MNObject::isReal() const
{
	return (getType() == TObjectType::TReal);
}

bool MNObject::isBool() const
{
	return (getType() == TObjectType::TBoolean);
}

bool MNObject::isCFunction() const
{
	return (getType() == TObjectType::TCFunction);
}

bool MNObject::isReferrer() const
{
	return ((valType & TObjectType::Referrer) == 1 && val._ref->getHard() != 0);
}

bool MNObject::isString() const
{
	return isReferrer() && (getType() == TObjectType::TString);
}

bool MNObject::isClosure() const
{
	return isReferrer() && (getType() == TObjectType::TClosure);
}

bool MNObject::isTable() const
{
	return isReferrer() && (getType() == TObjectType::TTable);
}

bool MNObject::isArray() const
{
	return isReferrer() && (getType() == TObjectType::TArray);
}

bool MNObject::isFunction() const
{
	return isReferrer() && (getType() == TObjectType::TFunction);
}

bool MNObject::isFiber() const
{
	return isReferrer() && (getType() == TObjectType::TFiber);
}

bool MNObject::isUserData() const
{
	return isReferrer() && (getType() == TObjectType::TUserData);
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

tinteger      MNObject::toInt(tinteger def) const
{
	if (isInt()) return val._int;
	if (isReal()) return (tinteger)val._float;
	return def;
}

treal      MNObject::toReal(treal def) const
{
	if (isReal()) return val._float;
	if (isInt()) return (treal)val._int;
	return def;
}

tboolean    MNObject::toBool(bool def) const
{
	if (isBool()) return val._bool;
	return def;
}

NativeFunc   MNObject::toCFunction() const
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

MNFiber* MNObject::toFiber() const
{
	if (isReferrer()) return mnrtti_cast<MNFiber>(toCountable());
	return NULL;
}

MNUserData* MNObject::toUserData() const
{
	if (isReferrer()) return mnrtti_cast<MNUserData>(toCountable());
	return NULL;
}
