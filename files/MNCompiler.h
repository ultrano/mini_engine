#ifndef _H_MNCompiler
#define _H_MNCompiler

#include "MNMemory.h"
#include "MNPrimaryType.h"

class MNFunction;
class MNObject;

class MNCodeMaker : public MNMemory
{
public:

	tsize  cursor;
	tsize&  size;
	tbyte*& bytes;

	MNCodeMaker(tbyte*& b, tsize& s): cursor(0), size(s), bytes(b)
	{
		size = 1;
		bytes = (tbyte*)MNMemory::malloc(size);
	};

	template<typename T>
	MNCodeMaker& operator <<(const T& t)
	{
		tsize newtPos = cursor + sizeof(T);
		if (newtPos > size)
		{
			tsize  newSize = (newtPos + 32);
			tbyte* newBytes = (tbyte*)MNMemory::malloc(newSize);
			memcpy(newBytes, bytes, size);
			MNMemory::free(bytes);
			bytes = newBytes;
			size = newSize;
		}
		memcpy(&bytes[cursor], &t, sizeof(T));
		cursor = newtPos;
		return *this;
	}

	template<typename T>
	void modify(tsize pos, const T& t)
	{
		if (pos + sizeof(T) < size) memcpy(&bytes[pos], &t, sizeof(T));
	}
};

class MNFuncBuilder
{
public:
	MNFuncBuilder* upFunc;
	MNFunction* func;
	MNCodeMaker codeMaker;
	
	MNFuncBuilder(MNFuncBuilder* up);
	tsize addConst(const MNObject& val);
};

#endif