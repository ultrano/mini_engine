#include "MNCompiler.h"
#include "MNFunction.h"
#include "MNObject.h"
#include "MNString.h"

#include <stdarg.h>

class MNCompileException : public MNMemory
{
public:
	MNCompileException(const tstring& str) : msg(str) {}
	tstring msg;
};

#define compile_error(format, ...) compile_error_print(m_current.row, m_current.col, (format), __VA_ARGS__)
void compile_error_print(tsize row, tsize col, const tchar* format, ...)
{
	static const tuint bufSize = 512;
	tchar buf1[bufSize] = { 0 };
	tchar buf2[bufSize] = { 0 };
	va_list args;
	va_start(args, format);
	vsprintf(&buf1[0], format, args);
	va_end(args);
	sprintf(&buf2[0], "compile error(%d, %d): %s", row, col, &buf1[0]);
	throw new MNCompileException(&buf2[0]);
}

#define compile_warning(format, ...) //compile_warning_print(m_current.row, m_current.col, (format), __VA_ARGS__)
void compile_warning_print(tsize row, tsize col, const tchar* format, ...)
{
	static const tuint bufSize = 256;
	tchar buf[bufSize] = { 0 };
	va_list args;
	va_start(args, format);
	vsprintf(&buf[0], format, args);
	va_end(args);
	printf("compile warning(%d, %d): %s\n", row, col, &buf[0]);
}

MNFuncBuilder::MNFuncBuilder(MNFuncBuilder* up)
	: upFunc(up)
	, func(new MNFunction())
	, codeMaker(func->m_codes, func->m_ncode)
{
	addLocal(thashstring("this"));
}

MNFuncBuilder::~MNFuncBuilder()
{
}

tsize MNFuncBuilder::addConst(const MNObject& val)
{
	tsize index = func->m_consts.size();
	while (index--)
	{
		const MNObject& obj = func->m_consts[index];
		if (obj.isString() && val.isString())
		{
			if (obj.toString()->ss() == val.toString()->ss())
			{
				return index;
			}
		}
	}
	func->m_consts.push_back(val);
	return func->m_consts.size() - 1;
}

bool MNFuncBuilder::addLocal(const thashstring& name)
{
	tsize index = locals.size();
	while(index--) if (locals[index] == name) return false;

	locals.push_back(name);
	if (locals.size() > func->m_nvars) func->m_nvars = locals.size();
	return true;
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

tboolean MNCompiler::build(MNObject& func)
{
	try
	{
		advance();
		advance();
		m_func = new MNFuncBuilder(NULL);
		_statements();
		code() << cmd_return_void;

		func = MNObject(TObjectType::Function, m_func->func->getReferrer());
		return true;
	}
	catch (MNCompileException* e)
	{
		printf("compile error: %s\n", e->msg.c_str());
	}
	while (m_func)
	{
		MNFuncBuilder* upFunc = m_func->upFunc;
		delete m_func->func;
		delete m_func;
		m_func = upFunc;
	}
	func = MNObject::Null();
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
	else if (ret = check(tok_if)) _if();
	else if (ret = check(tok_for));
	else if (ret = check(tok_switch));
	else if (ret = check(tok_while)) _while();
	else if (ret = check(tok_func)) _func();
	else if (ret = check('{')) _block();
	else if (ret = check(';')) while (check(';')) advance();
	else if (ret = check(tok_return))
	{
		_return();
		if (check(';')) while (check(';')) advance();
		else compile_error("expected ';' is gone after 'return'");
	}
	else if (ret = check(tok_break))
	{
		_break();
		if (check(';')) while (check(';')) advance();
		else compile_error("expected ';' is gone after 'break'");
	}
	else if (ret = check(tok_continue))
	{
		_continue();
		if (check(';')) while (check(';')) advance();
		else compile_error("expected ';' is gone after 'continue'");
	}
	else if (ret = _exp(false))
	{
		if (check(';')) while (check(';')) advance();
		else compile_error("expected ';' is gone after expression");
	}

	return ret;
}

void MNCompiler::_var()
{
	while (true)
	{
		advance(); //! skip 'var' or ','

		if (!check(tok_identify)) compile_error("worong variable");
		if (!m_func->addLocal(m_current.str)) compile_error("overalpped variable declaration '%s'", m_current.str.c_str());

		if (peek('=')) _exp(false);
		if (check(',')) continue; else break;
	}
}

void MNCompiler::_block()
{
	if (!check('{')) return;

	tsize nlocals = m_func->locals.size();

	advance();
	_statements();
	if (!check('}')) compile_error("there is no block ending");
	advance();

	if (nlocals < m_func->locals.size())
	{
		m_func->locals.resize(nlocals);
		code() << cmd_close_links << tuint16(nlocals);
	}
}

void MNCompiler::_if()
{
	if (!check(tok_if)) return;
	advance();

	if (!check('(')) compile_error("'if' statement needs conditional expression");
	advance();
	_exp();
	if (!check(')')) compile_error("it's wrong conditional expression for 'if' statement");
	advance();

	tsize fjp = (code() << cmd_fjp).cursor;
	tsize begin = (code() << tint16(0)).cursor;
	_statement();
	tsize end = code().cursor;
	if (check(tok_else))
	{
		advance();
		tsize jmp = (code() << cmd_jmp).cursor;
		end = (code() << tint16(0)).cursor;
		_statement();
		code().modify(jmp, tint16(code().cursor - end));
	}
	code().modify(fjp, tint16(end - begin));
}

void MNCompiler::_while()
{
	if (!check(tok_while)) return;
	advance();

	code() << cmd_jmp << tint16(sizeof(cmd_jmp) + sizeof(tint16));
	m_func->breakouts.push_back(code().cursor);

	tsize out   = (code() << cmd_jmp).cursor;
	tsize out_a = (code() << tint16(0)).cursor;

	m_func->playbacks.push_back(out_a);

	if (!check('(')) compile_error("it's wrong condition for 'while'");
	advance();
	_exp();
	if (!check(')')) compile_error("there is no terminal for condition for 'while'");
	advance();

	tsize fjp   = (code() << cmd_fjp).cursor;
	tsize fjp_a = (code() << tint16(0)).cursor;
	
	_statement();
	
	tsize jmp   = (code() << cmd_jmp).cursor;
	tsize jmp_a = (code() << tint16(0)).cursor;

	m_func->breakouts.pop_back();
	m_func->playbacks.pop_back();
	code().modify(out, tint16(jmp_a - out_a));
	code().modify(fjp, tint16(jmp_a - fjp_a));
	code().modify(jmp, tint16(out_a - jmp_a));
}

void MNCompiler::_break()
{
	if (!m_func->breakouts.size()) compile_error("wrong 'break' using");
	if (!check(tok_break)) return;
	advance();

	tsize jmp = (code() << cmd_jmp).cursor;
	code() << tint16(0);
	tint16 step = m_func->breakouts.back() - code().cursor;
	code().modify(jmp, step);
}

void MNCompiler::_continue()
{
	if (!m_func->playbacks.size()) compile_error("wrong 'continue' using");
	if (!check(tok_continue)) return;
	advance();

	tsize jmp = (code() << cmd_jmp).cursor;
	code() << tint16(0);
	tint16 step = m_func->playbacks.back() - code().cursor;
	code().modify(jmp, step);
}

void MNCompiler::_func()
{
	if (!check(tok_func)) return;
	advance();

	MNFuncBuilder* func = new MNFuncBuilder(m_func);
	m_func = func;

	//! function parameter
	{
		if (!check('(')) compile_error("function needs arguments statement");
		advance();

		if (!check(')')) while (true)
		{
			if (check(tok_identify))
			{
				if (!m_func->addLocal(m_current.str)) compile_error("overalpped parameter declaration '%s'", m_current.str.c_str());
				advance();
			}
			else if (check(',')) advance();
			else if (check(')')) break;
			else compile_error("it's wrong function parameter");
		}
		advance();
	}

	//! function body
	_block();
	code() << cmd_return_void;

	m_func = func->upFunc;
	tuint16 funcIndex = m_func->addConst(MNObject(TObjectType::Function, func->func->getReferrer()));
	tuint16 linkIndex = tuint16(func->links.size());
	code() << cmd_push_closure << funcIndex << linkIndex;
	for (tsize i = 0; i < linkIndex; ++i)
	{
		const MNExp& e = func->links[i];
		tbyte cmd = (e.type == MNExp::exp_local) ? cmd_load_stack : cmd_load_upval;
		code() << cmd << e.index;
	}
	delete func;
}

void MNCompiler::_return()
{
	if (!check(tok_return)) return;
	advance();

	bool hasRet = true;
	if (hasRet = !check(';')) _exp(); 
	code() << (hasRet ? cmd_return : cmd_return_void);
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
	case MNExp::exp_global: code() << cmd_up1; break;
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
	_exp_or(e);
	tbyte cmd = check(tok_add_assign)? cmd_add:
		        check(tok_sub_assign)? cmd_sub:
		        check(tok_mul_assign)? cmd_mul:
		        check(tok_div_assign)? cmd_div: cmd_none;
	if (cmd != cmd_none)
	{
		advance();
		MNExp e1 = e;
		if (e1.type == MNExp::exp_field) code() << cmd_up2;
		_load(e1);
		_exp();
		code() << cmd;
		_assign(e, leftVal);
	}
	else if (check('='))
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

void MNCompiler::_exp_or(MNExp& e)
{
	_exp_and(e);
	while (e.type != MNExp::exp_none)
	{
		tbyte cmd = check(tok_or) ? cmd_or : cmd_none;
		if (cmd == cmd_none) break;
		advance();
		_load(e);
		_exp_and(e);
		_load(e);
		code() << cmd;
	}
}

void MNCompiler::_exp_and(MNExp& e)
{
	_exp_logical(e);
	while (e.type != MNExp::exp_none)
	{
		tbyte cmd = check(tok_and) ? cmd_and : cmd_none;
		if (cmd == cmd_none) break;
		advance();
		_load(e);
		_exp_logical(e);
		_load(e);
		code() << cmd;
	}
}

void MNCompiler::_exp_logical(MNExp& e)
{
	_exp_add_sub(e);
	while (e.type != MNExp::exp_none)
	{
		tbyte cmd = check(tok_eq)? cmd_eq:
			        check(tok_neq)? cmd_neq:
			        check(tok_leq)? cmd_leq:
			        check(tok_geq)? cmd_geq:
			        check('<')? cmd_lt:
			        check('>')? cmd_gt:cmd_none;
		if (cmd == cmd_none) break;
		advance();
		_load(e);
		_exp_add_sub(e);
		_load(e);
		code() << cmd;
	}
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
	_exp_term(e);
	while (e.type != MNExp::exp_none)
	{
		tbyte cmd = check('*')? cmd_mul:check('/')?cmd_div:cmd_none;
		if (cmd == cmd_none) break;
		advance();
		_load(e);
		_exp_term(e);
		_load(e);
		code() << cmd;
	}
}

void MNCompiler::_exp_term(MNExp& e)
{
	_exp_postfix(e);
}

void MNCompiler::_exp_postfix(MNExp& e)
{
	_exp_primary(e);
	while (e.type != MNExp::exp_none)
	{
		if (check('('))
		{
			advance();
			if (e.type == MNExp::exp_field)
			{
				code() << cmd_load_method; //! [object key] -> [closure object]
			}
			else if (e.type != MNExp::exp_loaded)
			{
				_load(e); //! [closure]
				code() << cmd_load_this;  //! [closure object]
			}
			else if (e.type == MNExp::exp_none) compile_error("postfix compile is failed");

			tbyte nargs = 1;
			if (!check(')')) while (true)
			{
				_exp();
				nargs += 1;
				if (check(')')) break;
				else if (check(',')) advance();
				else compile_error("function call arguments error");
			}
			advance();

			code() << cmd_call << nargs;
			e.type = MNExp::exp_loaded;
		}
		else if (check('['))
		{
			advance();
			_load(e);
			_exp();
			e.type = MNExp::exp_field;
			if (!check(']')) compile_error("accessing field needs terminal token -> ']'");
			advance();
		}
		else if (check('.'))
		{
			advance();
			_load(e);
			if (!check(tok_identify)) compile_error("accessing field needs field name");
			e.index = m_func->addConst(MNObject::String(m_current.str));
			code() << cmd_load_const << e.index;
			e.type = MNExp::exp_field;
			advance();
		}
		else break;
	}
}

void MNCompiler::_exp_primary(MNExp& e)
{
	//*
	if (check(tok_identify) || check(tok_this))
	{
		m_func->findLocal(m_current.str, e);
		if (e.type == MNExp::exp_none)
		{
			compile_warning("no var named '%s', trying to find in field", m_current.str.c_str());
			e.index = m_func->addConst(MNObject::String(m_current.str));
			code() << cmd_load_this;
			code() << cmd_load_const << e.index;
			e.type = MNExp::exp_field;
		}
		advance();
	}
	else if (check(tok_global))
	{
		advance();
		if (!check(tok_identify)) compile_error("invalid global field");

		e.index = m_func->addConst(MNObject::String(m_current.str));
		e.type  = MNExp::exp_global;
		advance();
	}
	else if (check(tok_func))
	{
		_func();
		e.type = MNExp::exp_loaded;
	}
	else if (check(tok_yield))
	{
		advance();

		bool hasRet = true;
		if (hasRet = !check(';')) _exp();
		code() << (hasRet? cmd_yield : cmd_yield_void);
		e.type = MNExp::exp_loaded;
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

		MNFuncBuilder* func = new MNFuncBuilder(m_func);
		m_func = func;

		tuint16 nfield = 0;
		while (!check('}'))
		{
			if (!check(tok_string) && !check(tok_identify)) compile_error("wrong table field");
			tbyte cmd = peek(':') ? cmd_insert_field : peek('=') ? cmd_store_stack : cmd_none;

			if (cmd == cmd_insert_field)
			{
				code() << cmd_load_this;
				code() << cmd_load_const << (e.index = m_func->addConst(MNObject::String(m_current.str)));
			}
			else if (cmd == cmd_store_stack)
			{
				if (!m_func->addLocal(m_current.str)) compile_warning("overapped field name");
				m_func->findLocal(m_current.str, e);
			}
			else compile_error("delimiter ':' or '=' is missing ");

			advance(); //! field name
			advance(); //! ':' or '='
			_exp();

			if (cmd == cmd_insert_field) code() << cmd;
			else if (cmd == cmd_store_stack) code() << cmd << e.index;

			nfield += 1;
			if (check(',')) advance();
		}
		advance();
		code() << cmd_load_this << cmd_return;

		m_func = func->upFunc;
		tuint16 funcIndex = m_func->addConst(MNObject(TObjectType::Function, func->func->getReferrer()));
		tuint16 linkIndex = tuint16(func->links.size());
		code() << cmd_push_closure << funcIndex << linkIndex;
		for (tsize i = 0; i < linkIndex; ++i)
		{
			const MNExp& ul = func->links[i];
			tbyte cmd = (ul.type == MNExp::exp_local) ? cmd_load_stack : cmd_load_upval;
			code() << cmd << ul.index;
		}
		code() << cmd_push_table << nfield;
		code() << cmd_call << tbyte(1);
		delete func;

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
			code() << cmd_up1;
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
		e.type = MNExp::exp_loaded;
	}
	//*/
}