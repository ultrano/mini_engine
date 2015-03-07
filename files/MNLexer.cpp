#include "MNLexer.h"

class MNLexer::Scanner : public MNMemory
{
public:
	virtual ~Scanner() {};
	/* return : 읽은 바이트 수. 읽을게 없다면 -1 */
	virtual tint read(tbyte* b, unsigned len) = 0;
};

class FileScanner : public MNLexer::Scanner
{
public:

	FILE* m_file;
	FileScanner(const tstring& filePath)
		: m_file(NULL)
	{
		fopen_s(&m_file, filePath.c_str(), "rb");
	}
	~FileScanner()
	{
		if (m_file)
		{
			fclose(m_file);
			m_file = NULL;
		}
	}
	virtual tint read(tbyte* b, unsigned len)
	{
		if ( m_file )
		{
			fpos_t pos0 = 0, pos1 = 0;
			fgetpos( m_file, &pos0 );
			fread( b, len, 1, m_file );
			fgetpos( m_file, &pos1 );
			return (tint)((pos1-pos0)? (pos1-pos0):-1);
		}
		return -1;
	}
};

MNLexer::MNLexer()
	: m_scan(NULL)
	, m_index(sizeof(m_buf))
	, m_col(1)
	, m_row(1)
{

}

MNLexer::~MNLexer()
{
	if (m_scan) delete m_scan;
}

void MNLexer::openFile(const tstring& filePath)
{
	m_scan = new FileScanner(filePath);
	nextChar();
}

void MNLexer::nextChar()
{
	if (!m_scan)
	{
		m_char = -1;
		return;
	}
	if (m_index == sizeof(m_buf))
	{
		int ret = m_scan->read((tbyte*)&m_buf[0], sizeof(m_buf));
		if (ret == 0 || ret == -1)
		{
			m_char = -1;
			return;
		}
		else
		{
			if (ret < sizeof(m_buf)) m_buf[ret] = -1;
			m_index = 0;
		}
	}
	m_char = m_buf[m_index++];
}

void MNLexer::scan(Token& tok)
{
	while (isspace(m_char)) nextChar();

	tok.col = m_col;
	tok.row = m_row;
	if (m_char == -1)
	{
		tok.type = tok_eos;
	}
	else if (isdigit(m_char))
	{
		tok.type = tok_integer;
		const tsize bufSize = 256;
		tchar buf[bufSize] = { 0 };
		tsize index = 0;
		while (true)
		{
			if (m_char == '.') tok.type = tok_float;
			else if (!isdigit(m_char)) break;

			if (index == bufSize)
			{
				tok.type = tok_error;
				return;
			}
			buf[index++] = m_char;
			nextChar();
		}
		tok.str = &buf[0];
	}
	else if (m_char == '"')
	{
		nextChar();
		tok.type = tok_string;
		char buf[1024] = { 0 };
		char* d = &buf[0];
		while (m_char != '"')
		{
			if (m_char == -1 || m_char == '\n') tok.type = tok_eos;
			*d++ = m_char;
			nextChar();
		}
		nextChar();
		tok.str = &buf[0];
	}
	else if (isalpha(m_char) || m_char == '_')
	{
		const tsize bufSize = 256;
		char buf[bufSize] = { 0 };
		tsize index = 0;
		while ((isalnum(m_char) || m_char == '_'))
		{
			if (index == bufSize)
			{
				tok.type = tok_error;
				return;
			}
			buf[index++] = m_char;
			nextChar();
		}
		tok.str = &buf[0];
		tok.type = tok_identify;
		reservedWords(tok);
	}
	else
	{
		tok.type = m_char;
		nextChar();
		if (m_char == '=')
		{
			switch (tok.type)
			{
			case '=': tok.type = tok_eq;    break;
			case '!': tok.type = tok_neq;   break;
			case '>': tok.type = tok_geq;   break;
			case '<': tok.type = tok_leq;   break;
			case '+': tok.type = tok_addeq; break;
			case '-': tok.type = tok_subeq; break;
			case '/': tok.type = tok_diveq; break;
			case '*': tok.type = tok_muleq; break;
			default:  tok.type = tok_error; return;
			}
			nextChar();
		}
		else if ((m_char == '-' || m_char == '+') && m_char == tok.type)
		{
			tok.type = (m_char == '+') ? tok_inc : tok_dec;
			nextChar();
		}
		else if ((m_char == '<' || m_char == '>') && m_char == tok.type)
		{
			tok.type = (m_char == '<') ? tok_push : tok_pull;
			nextChar();
		}
		else if ((m_char == '&' || m_char == '|') && m_char == tok.type)
		{
			tok.type = (m_char == '&') ? tok_and : tok_or;
			nextChar();
		}
	}
}
