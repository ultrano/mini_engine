#ifndef _H_MNCompiler
#define _H_MNCompiler

#include "MNMemory.h"
#include "MNPrimaryType.h"
#include "MNLexer.h"

class MNFunction;
class MNObject;

class MNCodeMaker : public MNMemory
{
public:

	tsize   cursor;
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


struct MNExp
{
	enum
	{
		exp_none,		
		exp_loaded,
		exp_local,
		exp_upval,
		exp_global,
		exp_field,
	};
	tbyte type;
	tuint16 index;
	MNExp() : type(exp_none) {};
	tboolean operator ==(const MNExp& e) { return (e.type == type && e.index == index); }
};

class MNFuncBuilder
{
public:
	MNFuncBuilder* upFunc;
	MNFunction*    func;
	MNCodeMaker    codeMaker;

	tarray<thashstring>	locals;
	tarray<MNExp>	    links;
	tarray<tsize>		blocks;
	tarray<tsize>		breakouts;
	tarray<tsize>		playbacks;
	
	MNFuncBuilder(MNFuncBuilder* up);
	~MNFuncBuilder();
	tsize addConst(const MNObject& val);
	bool  addLocal( const thashstring& name);
	void  findLocal( const thashstring& name, MNExp& e );
};

class MNCompiler : public MNMemory
{
public:

	MNFuncBuilder* m_func;
	MNLexer m_lexer;
	MNLexer::Token m_current;
	MNLexer::Token m_peeking;

	MNCodeMaker& code();
	void     advance();
	tboolean check(tint type) const;
	tboolean peek(tint type) const;

	tboolean build(MNObject& func);

	void     _statements();
	tboolean _statement();
	void _var();
	void _block();
	void _if();
	void _while();
	void _break();
	void _continue();
	void _func();
	void _return();

	void _load(MNExp& e);
	void _assign(MNExp& e, tboolean leftVal);
	tboolean _exp(tboolean leftVal = true);
	void     _exp_or(MNExp& e);
	void     _exp_and(MNExp& e);
	void     _exp_logical(MNExp& e);
	void     _exp_add_sub(MNExp& e);
	void     _exp_mul_div(MNExp& e);
	void     _exp_term(MNExp& e);
	void     _exp_postfix(MNExp& e);
	void     _exp_primary(MNExp& e);
};

#endif