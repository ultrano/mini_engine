#ifndef _H_MNLexer
#define _H_MNLexer

#include "MNPrimaryType.h"
#include "MNMemory.h"

class MNLexer : public MNMemory
{
public:
	class Scanner;
	class Token
	{
	public:
		tbyte type;
		thashstring str;
		tuint col, row;
	};
public:
	Scanner* m_scan;
	tchar m_buf[512];
	tchar m_char;
	tchar m_next;
	tuint m_index;
	tuint m_col, m_row;

	MNLexer();
	~MNLexer();
	void  openFile(const tstring& filePath);
    void  openText(const tstring& text);
	tchar read();
	void  advance();
	void  scan(Token& tok);
};


enum tok_type
{
	tok_none       = 127,
	tok_string     = 128,
	tok_integer    = 129,
	tok_float      = 130,
	tok_identify   = 131,
	tok_global     = 132,
	tok_if         = 133,
	tok_else       = 134,
	tok_for        = 135,
	tok_while      = 136,
	//tok_switch     = 137, //! not yet supported
	tok_case       = 138,
	tok_break      = 139,
	tok_continue   = 140,
	tok_func       = 141,
	tok_var        = 142,
	tok_this       = 143,
	tok_null       = 144,
	tok_true       = 145,
	tok_false      = 146,
	tok_return     = 147,
	tok_yield      = 148,
	//tok_class      = 149,
	//tok_new        = 150,
	//tok_base       = 151,
	tok_add_assign = 152,
	tok_sub_assign = 153,
	tok_div_assign = 154,
	tok_mul_assign = 155,
	tok_eq         = 156,
	tok_neq        = 157,
	tok_geq        = 158,
	tok_leq        = 159,
	tok_inc        = 160,
	tok_dec        = 161,
	tok_push       = 162,
	tok_pull       = 163,
	tok_and        = 164,
	tok_or         = 165,
	//tok_native     = 167,
	//tok_constructor = 166,

	tok_error  = 254,
	tok_eos    = 255,
	max_tok    = tok_eos,
};

inline void reservedWords(MNLexer::Token& tok)
{
	if (tok.type != tok_identify) return;
	if (tok.str.size() == 0) return;

	static const thashstring _if("if");
	static const thashstring _else("else");
	static const thashstring _for("for");
	static const thashstring _while("while");
	static const thashstring _switch("switch");
	static const thashstring _case("case");
	static const thashstring _break("break");
	static const thashstring _continue("continue");
	static const thashstring _func("func");
	static const thashstring _var("var");
	static const thashstring _this("this");
	static const thashstring _null("null");
	static const thashstring _true("true");
	static const thashstring _false("false");
	static const thashstring _return("return");
	static const thashstring _yield("yield");

	if (tok.str == _var)           tok.type = tok_var;
	else if (tok.str == _func)     tok.type = tok_func;
	else if (tok.str == _return)   tok.type = tok_return;
	else if (tok.str == _this)     tok.type = tok_this;
	else if (tok.str == _null)     tok.type = tok_null;
	else if (tok.str == _if)       tok.type = tok_if;
	else if (tok.str == _else)     tok.type = tok_else;
	else if (tok.str == _for)      tok.type = tok_for;
	else if (tok.str == _while)    tok.type = tok_while;
	else if (tok.str == _true)     tok.type = tok_true;
	else if (tok.str == _false)    tok.type = tok_false;
	//else if (tok.str == _switch)   tok.type = tok_switch; //! not yet supported
	else if (tok.str == _case)     tok.type = tok_case;
	else if (tok.str == _break)    tok.type = tok_break;
	else if (tok.str == _continue) tok.type = tok_continue;
	else if (tok.str == _yield)    tok.type = tok_yield;
}

#endif