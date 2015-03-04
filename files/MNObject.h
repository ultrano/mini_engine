#ifndef _H_MNValue
#define _H_MNValue

#include "MNMemory.h"
#include "MNPrimaryType.h"

class MNFiber;
class MNReferrer;
class MNCountable;
class MNCollectable;
class MNString;
class MNClosure;
class MNTable;
class MNArray;
class MNFunction;
class MNObject : public MNMemory
{
public:
	static const MNObject& Null() { static MNObject obj(TObjectType::Null); return obj; }
	static MNObject Int(tint32 i) { MNObject obj(TObjectType::Int); obj.val._int = i; return obj; }
	static MNObject Float(tfloat f) { MNObject obj(TObjectType::Float); obj.val._float = f; return obj; }
	static MNObject Bool(bool b) { MNObject obj(TObjectType::Boolean); obj.val._bool = b; return obj; }
	static MNObject Pointer(void* p) { MNObject obj(TObjectType::Pointer); obj.val._pointer = p; return obj; }
	static MNObject CFunction(TCFunction cfunc)  { MNObject obj(TObjectType::CFunction); obj.val._func = cfunc; return obj; }
	static MNObject String(const tstring& str);
	static MNObject Format(const tstring& format, ...);
public:

	MNObject();
	MNObject(const MNObject& copy);
	MNObject(tuint8 type);
	MNObject(tuint8 type, MNReferrer* ref);
	~MNObject();

	MNObject& assign(const MNObject& right);
	MNObject& operator = (const MNObject& right) { return assign(right); }

	tint getType() const;

	bool isNull() const;
	bool isPointer() const;
	bool isInt() const;
	bool isFloat() const;
	bool isBool() const;
	bool isCFunction() const;
	bool isReferrer() const;
	bool isString() const;
	bool isClosure() const;
	bool isTable() const;
	bool isArray() const;
	bool isFunction() const;

	void*       toRaw() const;
	void*       toPointer() const;
	tint32      toInt(tint32 def = 0) const;
	tfloat      toFloat(tfloat def = 0.0f) const;
	tboolean    toBool(bool def = false) const;
	TCFunction  toCFunction() const;
	MNString*   toString() const;
	MNClosure*  toClosure() const;
	MNCountable* toCountable() const;
	MNCollectable* toCollectable() const;
	MNTable*    toTable() const;
	MNArray*    toArray() const;
	MNFunction* toFunction() const;

private:

	union
	{
		tint32 _int;
		tfloat _float;
		tboolean _bool;
		void* _pointer;
		TCFunction _func;
		MNReferrer* _ref;
	} val;

	tuint8 valType;
};


#endif