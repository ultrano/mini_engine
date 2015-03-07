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
	addLocal(thashstring("this"));
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
	if (locals.size() > func->m_nvars) func->m_nvars = locals.size();
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

tboolean MNCompiler::build()
{
	try
	{
		advance();
		advance();
		m_func = new MNFuncBuilder(NULL);
		_statements();
		code() << cmd_return_void;
		return true;
	}
	catch (MNCompileException* e)
	{
		printf(e->msg.c_str());
	}
	return false;
}

void     MNCompiler::_statements()
{
	while (_statement());
}

tboolean MNCompiler::_statement()
{
	tboolean ret = false;
	if (ret = check(tok_var)) _var();
	else if (ret = check(tok_if));
	else if (ret = check(tok_else));
	else if (ret = check(tok_for));
	else if (ret = check(tok_while));
	else if (ret = check(tok_switch));
	else if (ret = check(tok_func));
	else if (ret = check(tok_return));
	else if (ret = check(';')) advance();
	else ret = _exp(false);

	return ret;
}

void MNCompiler::_var()
{
	while (true)
	{
		advance(); //! skip 'var' or ','

		if (!check(tok_identify)) compile_error("worong variable");
		m_func->addLocal(m_current.str);

		if (peek('=')) _exp(false);
		if (check(',')) continue; else break;
	}
}

void MNCompiler::_load(MNExp& e)
{
	switch ( e.type )
	{
	case MNExp::exp_local : code() << cmd_load_stack  << e.index; break;
	case MNExp::exp_upval : code() << cmd_load_upval  << e.index; break;
	case MNExp::exp_global: code() << cmd_load_global << e.index; break;
	case MNExp::exp_field : code() << cmd_load_field; break;
	case MNExp::exp_loaded: break;
	default: compile_error("unknown expression can't be loaded ");
	}
	e.type = MNExp::exp_loaded;
}

void MNCompiler::_assign(MNExp& e, tboolean leftVal)
{
	if ( leftVal ) switch ( e.type )
	{
	case MNExp::exp_local :
	case MNExp::exp_upval :
	case MNExp::exp_global: code() << cmd_up1_x1; break;
	case MNExp::exp_field : code() << cmd_up1_x2; break;
	default: compile_error("can't assign to unknown l-expression"); break;
	}

	switch ( e.type )
	{
	case MNExp::exp_local : code() << cmd_store_stack  << e.index; break;
	case MNExp::exp_upval : code() << cmd_store_upval  << e.index; break;
	case MNExp::exp_global: code() << cmd_store_global << e.index; break;
	case MNExp::exp_field : code() << cmd_store_field; break;
	default: compile_error("unknown r-expression"); break;
	}
	e.type = leftVal? MNExp::exp_loaded : MNExp::exp_none;
}

tboolean MNCompiler::_exp(tboolean leftVal)
{
	MNExp e;
	_exp_add_sub(e);
	if (check('='))
	{
		advance();
		_exp();
		_assign(e, leftVal);
	}
	else if (leftVal) _load(e);
	else if (e.type == MNExp::exp_loaded) code() << cmd_pop1;
	else if (e.type == MNExp::exp_field)  code() << cmd_pop2;
	else if (e.type == MNExp::exp_none) return false;
	else compile_error("expression wasn't completed");

	return true;
}

void MNCompiler::_exp_add_sub(MNExp& e)
{
	_exp_mul_div(e);
	while (e.type != MNExp::exp_none)
	{
		tbyte cmd = check('+')? cmd_add:check('-')?cmd_sub:cmd_none;
		if (cmd == cmd_none) break;
		advance();
		_load(e);
		_exp_mul_div(e);
		_load(e);
		code() << cmd;
	}
}

void MNCompiler::_exp_mul_div(MNExp& e)
{
	_primary(e);
	while (e.type != MNExp::exp_none)
	{
		tbyte cmd = check('*')? cmd_mul:check('/')?cmd_div:cmd_none;
		if (cmd == cmd_none) break;
		advance();
		_load(e);
		_primary(e);
		_load(e);
		code() << cmd;
	}
}

void MNCompiler::_primary(MNExp& e)
{
	//*
	if (check(tok_identify) || check(tok_this))
	{
		m_func->findLocal(m_current.str, e);
		if (e.type == MNExp::exp_none)
		{
			e.index = m_func->addConst(MNObject::String(m_current.str));
			code() << cmd_load_stack << tuint16(0);
			code() << cmd_load_const << e.index;
			e.type = MNExp::exp_field;
		}
		advance();
	}
	else if (check(tok_func))
	{
		//_func(true);
		//e.type = expr_ontop;
	}
	else if (check(tok_null))
	{
		code() << cmd_push_null;
		e.type = MNExp::exp_loaded;
		advance();
	}
	else if (check(tok_true) || check(tok_false))
	{
		tbyte cmd = check(tok_true)? cmd_push_true:cmd_push_false;
		code() << cmd;
		e.type = MNExp::exp_loaded;
		advance();
	}
	else if (check(tok_string))
	{
		e.index = m_func->addConst(MNObject::String(m_current.str));
		code() << cmd_load_const << e.index;
		e.type = MNExp::exp_loaded;
		advance();
	}
	else if (check(tok_integer))
	{
		tint num = 0;
		sscanf_s( m_current.str.str().c_str(), "%d", &num );
		code() << cmd_push_int << num;
		e.type = MNExp::exp_loaded;
		advance();
	}
	else if (check(tok_float))
	{
		float num = 0;
		sscanf_s( m_current.str.str().c_str(), "%f", &num );
		code() << cmd_push_float << num;
		e.type = MNExp::exp_loaded;
		advance();
	}
	else if (check('{'))
	{
		advance();
		code() << cmd_push_table;
		while ( !check('}') )
		{
			/*
			if ( peek(0).type != tok_identify ) error_compile("테이블 초기화 에러");
			JSValue name = JSValueString(peek(0).text);
			advance();
			if ( peek(0).type == ':' )
			{
				advance();

				e.ui8  = funcinfo->func()->addConst( name );

				emit() << cmd_up;
				emit() << cmd_load_const << e.ui8;
				syntax_expr();
				emit() << cmd_store_indexed;
			}

			if ( peek(0).type == ',' ) advance();
			//*/
		}

		advance();
		e.type = MNExp::exp_loaded;
	}
	else if (check('['))
	{
		advance();
		tsize pos = (code() << cmd_push_array).cursor;
		code() << tuint16(0);

		tint index = 0;
		while (!check(']'))
		{
			code() << cmd_up1_x1;
			code() << cmd_push_int << index++;
			_exp();
			code() << cmd_store_field;
			if (check(',')) advance();
		}
		advance();
		code().modify(pos, tuint16(index));
		e.type = MNExp::exp_loaded;
	}
	else if (check('('))
	{
		advance();
		_exp();
		if (!check(')')) compile_error("primary term error");
		advance();
	}
	//*/
}