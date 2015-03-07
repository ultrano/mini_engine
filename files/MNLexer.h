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
		tstring str;
		tuint col, row;
	};
public:
	Scanner* m_scan;
	tchar m_buf[512];
	tchar m_char;
	tuint m_index;
	tuint m_col, m_row;

	MNLexer();
	~MNLexer();
	void openFile(const tstring& filePath);
	void nextChar();
	void scan(Token& tok);
};


enum tok_type
{
	tok_none = 127,
	tok_string = 128,
	tok_number = 129,
	tok_identify = 130,

	tok_if       = 131,
	tok_else     = 132,
	tok_for      = 133,
	tok_while    = 134,
	tok_switch   = 135,
	tok_case     = 136,
	tok_break    = 137,
	tok_continue = 138,
	tok_func     = 139,
	tok_var      = 140,
	tok_this     = 141,
	tok_null     = 142,
	tok_true     = 143,
	tok_false    = 144,
	tok_return   = 145,

	tok_addeq  = 146,
	tok_subeq  = 147,
	tok_diveq  = 148,
	tok_muleq  = 149,
	tok_eq     = 150,
	tok_neq    = 151,
	tok_geq    = 152,
	tok_leq    = 153,
	tok_inc    = 154,
	tok_dec    = 155,
	tok_push   = 156,
	tok_pull   = 157,
	tok_and    = 158,
	tok_or     = 159,

	tok_error  = 254,
	tok_eos    = 255,
	max_tok    = tok_eos,
};

inline void reservedWords(MNLexer::Token& tok)
{
	if (tok.type != tok_identify) return;
	if (tok.str.size() == 0) return;

	if (tok.str == "var")           tok.type = tok_var;
	else if (tok.str == "func")     tok.type = tok_func;
	else if (tok.str == "return")   tok.type = tok_return;
	else if (tok.str == "this")     tok.type = tok_this;
	else if (tok.str == "null")     tok.type = tok_null;
	else if (tok.str == "if")       tok.type = tok_if;
	else if (tok.str == "else")     tok.type = tok_else;
	else if (tok.str == "for")      tok.type = tok_for;
	else if (tok.str == "while")    tok.type = tok_while;
	else if (tok.str == "true")     tok.type = tok_true;
	else if (tok.str == "false")    tok.type = tok_false;
	else if (tok.str == "switch")   tok.type = tok_switch;
	else if (tok.str == "case")     tok.type = tok_case;
	else if (tok.str == "break")    tok.type = tok_break;
	else if (tok.str == "continue") tok.type = tok_continue;
}

#endif