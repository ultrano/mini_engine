#include "MNLexer.h"

class MNLexer::Scanner
{
public:
	/* return : 읽은 바이트 수. 읽을게 없다면 -1 */
	virtual tint read(tbyte* b, unsigned len) = 0;
};


MNLexer::MNLexer()
	: m_scan(NULL)
	, m_index(sizeof(m_buf))
	, m_char(0)
	, m_col(1)
	, m_row(1)
{

}

MNLexer::~MNLexer()
{

}

tint MNLexer::readChar()
{
	if (!m_scan) return -1;
	if (m_index == sizeof(m_buf))
	{
		int ret = m_scan->read((tbyte*)&m_buf[0], sizeof(m_buf));
		if (ret == 0 || ret == -1) return -1;
		else
		{
			if (ret < sizeof(m_buf)) m_buf[ret] = -1;
			m_index = 0;
		}
	}
	return m_buf[m_index++];
}

void MNLexer::scan(Token& tok)
{
	int _char = 0;
	while (isspace(_char = readChar()));

	tok.col = m_col;
	tok.row = m_row;
	if (_char == -1)
	{
		tok.type = tok_eos;
	}
	else if (isdigit(_char))
	{
		tok.type = tok_number;
		tchar buf[256] = { 0 };
		tchar* d = &buf[0];
		while (isdigit(_char)) { *d++ = _char; _char = readChar(); }
		tok.str = &buf[0];
	}
	else if (m_char == '"')
	{
		_char = readChar();
		tok.type = tok_string;
		char buf[1024] = { 0 };
		char* d = &buf[0];
		while (_char != '"')
		{
			if (m_char == -1 || m_char == '\n') tok.type = tok_eos;
			*d++ = _char;
			_char = readChar();
		}
		_char = readChar();
		tok.str = &buf[0];
	}
	else if (isalpha(_char) || m_char == '_')
	{
		static char buf[256] = { 0 };
		memset(&buf[0], 0, sizeof(buf));
		char* d = &buf[0];
		while ((isalnum(_char) || m_char == '_')) { *d++ = _char; _char = readChar(); }
		tok.str = &buf[0];
		tok.type = tok_identify;
	}
	else
	{
		tok.type = _char;
		_char = readChar();
		if (m_char == '=')
		{
			switch (tok.type)
			{
			case '=': tok.text = keyword(kw_eq); getChar(); break;
			case '!': tok.text = keyword(kw_neq); getChar(); break;
			case '>': tok.text = keyword(kw_geq); getChar(); break;
			case '<': tok.text = keyword(kw_leq); getChar(); break;
			case '+': tok.text = keyword(kw_addeq); getChar(); break;
			case '-': tok.text = keyword(kw_subeq); getChar(); break;
			case '/': tok.text = keyword(kw_diveq); getChar(); break;
			case '*': tok.text = keyword(kw_muleq); getChar(); break;
			}
			tok.type = tok_identify;
		}
		else if ((m_char == '-' || m_char == '+') && m_char == tok.type)
		{
			tok.text = (m_char == '+') ? keyword(kw_inc) : keyword(kw_dec);
			tok.type = tok_identify;
			getChar();
		}
		else if ((m_char == '<' || m_char == '>') && m_char == tok.type)
		{
			tok.text = (m_char == '<') ? keyword(kw_push) : keyword(kw_pull);
			tok.type = tok_identify;
			getChar();
		}
		else if ((m_char == '&' || m_char == '|') && m_char == tok.type)
		{
			tok.text = (m_char == '&') ? keyword(kw_and) : keyword(kw_or);
			tok.type = tok_identify;
			getChar();
		}
	}
}
