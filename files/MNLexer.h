#ifndef _H_MNLexer
#define _H_MNLexer

#include "MNPrimaryType.h"

enum tok_type
{
	tok_none = 127,
	tok_string = 128,
	tok_number = 129,
	tok_identify = 130,
	tok_if = 131,
	tok_else = 132,
	tok_for = 133,
	tok_while = 134,
	tok_switch = 135,
	tok_case = 136,
	tok_break = 137,
	tok_continue = 138,
	tok_function = 139,
	tok_var = 140,
	tok_this   = 141,
	tok_null   = 142,
	tok_true   = 143,
	tok_false  = 144,
	tok_return = 145,
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

	tok_eos = 255,
	max_tok = tok_eos,
};


class MNLexer
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
	tuint m_index;
	tuint m_col, m_row;

	MNLexer();
	~MNLexer();
	tint readChar();
	void scan(Token& tok);
};

#endif