#include "MNCompiler.h"
#include "MNFunction.h"
#include "MNObject.h"

class MNCompileException : public MNMemory
{
public:
	MNCompileException(const tstring& str) : msg(str) {}
	tstring msg;
};
#define compile_error(msg) throw new MNCompileException((msg))

MNFuncBuilder::MNFuncBuilder(MNFuncBuilder* up)
	: upFunc(up)
	, func(new MNFunction())
	, codeMaker(func->m_codes, func->m_ncode)
{
}

tsize MNFuncBuilder::addConst(const MNObject& val)
{
	func->m_consts.push_back(val);
	return func->m_consts.size() - 1;
}

void MNFuncBuilder::addLocal( const thashstring& name)
{
	tsize index = locals.size();
	while(index--) if (locals[index] == name) compile_error("overalpped variable declaration");

	locals.push_back(name);
}

void MNFuncBuilder::findLocal( const thashstring& name, MNExp& e )
{
	e.type = MNExp::exp_none;

	//! search local
	{
		tsize index = locals.size();
		while (index--) if (locals[index] == name)
		{
			e.type  = MNExp::exp_local;
			e.index = index;
			return;
		}
	}

	//! search up scope
	if (upFunc)
	{
		upFunc->findLocal(name, e);
		if (e.type == MNExp::exp_none) return;

		tsize index = 0;
		for (; index < links.size(); ++index) if (links[index] == e) break;
		if (index == links.size()) links.push_back(e);
		e.type  = MNExp::exp_upval;
		e.index = index;
	}
}

MNCodeMaker& MNCompiler::code()
{
	return m_func->codeMaker;
}

void     MNCompiler::advance()
{
	m_current = m_peeking;
	m_lexer.scan(m_peeking);
}

tboolean MNCompiler::check(tint type) const
{
	return (m_current.type == type);
}

tboolean MNCompiler::peek(tint type) const
{
	return (m_peeking.type == type);
}

tboolean MNCompiler::_statement()
{
	if (check(tok_var)) _var();
	else if (check(tok_if));
	else if (check(tok_else));
	else if (check(tok_for));
	else if (check(tok_while));
	else if (check(tok_switch));
	else if (check(tok_func));
	else if (check(tok_return));

	return false;
}

void MNCompiler::_var()
{
	while (true)
	{
		advance(); //! skip 'var' or ','

		if (check(tok_identify)) compile_error("worong variable");
		m_func->addLocal(m_current.str);

		advance();

		if (peek('=')) _exp();
		if (check(',')) continue; else break;
	}
}

void MNCompiler::_exp(tboolean leftVal)
{
}

void MNCompiler::_load(MNExp& e)
{
	switch ( e.type )
	{
	case MNExp::exp_local : code() << cmd_load_stack << e.index; break;
	case MNExp::exp_upval : code() << cmd_load_upval << e.index; break;
	case MNExp::exp_global: code() << cmd_load_global << e.index; break;
	case MNExp::exp_field : code() << cmd_load_field; break;
	case MNExp::exp_loaded: break;
	default: compile_error("this type of value can't be loaded ");
	}
	e.type = MNExp::exp_loaded;
}