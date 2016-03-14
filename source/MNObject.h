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
class MNUserData;
class MNObject : public MNMemory
{
public:
	static const MNObject& Null() { static MNObject obj(TObjectType::TNull); return obj; }
	static MNObject Int(tinteger i) { MNObject obj(TObjectType::TInt); obj.val._int = i; return obj; }
	static MNObject Real(treal f) { MNObject obj(TObjectType::TReal); obj.val._float = f; return obj; }
	static MNObject Bool(bool b) { MNObject obj(TObjectType::TBoolean); obj.val._bool = b; return obj; }
	static MNObject Pointer(void* p) { MNObject obj(TObjectType::TPointer); obj.val._pointer = p; return obj; }
	static MNObject CFunction(NativeFunc cfunc)  { MNObject obj(TObjectType::TCFunction); obj.val._func = cfunc; return obj;}
	static MNObject String(const tstring& str);
	static MNObject String(const thashstring& str);
	static MNObject Format(const tchar* format, ...);
	static MNObject Referrer(MNReferrer* ref);
public:

	MNObject();
	MNObject(const MNObject& copy);
	MNObject(tuint8 type);
	MNObject(tuint8 type, MNReferrer* ref);
	~MNObject();

	MNObject& assign(const MNObject& right);
	MNObject& operator = (const MNObject& right) { return assign(right); }

	tint getType() const;
	thashtype getHash() const;

	bool isNull() const;
	bool isPointer() const;
	bool isInt() const;
	bool isReal() const;
	bool isBool() const;
	bool isCFunction() const;
	bool isReferrer() const;
	bool isString() const;
	bool isClosure() const;
	bool isTable() const;
	bool isArray() const;
	bool isFunction() const;
	bool isFiber() const;
	bool isUserData() const;

	void*       toRaw() const;
	void*       toPointer() const;
	tinteger    toInt(tinteger def = 0) const;
	treal       toReal(treal def = 0.0f) const;
	tboolean    toBool(bool def = false) const;
	NativeFunc  toCFunction() const;
	MNString*   toString() const;
	MNClosure*  toClosure() const;
	MNCountable* toCountable() const;
	MNCollectable* toCollectable() const;
	MNTable*    toTable() const;
	MNArray*    toArray() const;
	MNFunction* toFunction() const;
	MNFiber*    toFiber() const;
	MNUserData* toUserData() const;

private:

	union
	{
		tinteger _int;
		treal _float;
		tboolean _bool;
		void* _pointer;
		NativeFunc _func;
		MNReferrer* _ref;
	} val;

	tuint8 valType;
};


#endif