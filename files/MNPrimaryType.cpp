#include "MNPrimaryType.h"

THashString::THashString()
	: m_hash(0)
{
}

THashString::THashString(const tstring& str)
	: m_str(str)
{
	m_hash = makeHash(m_str);
}

thash32 THashString::makeHash(const tstring& str)
{
	return makeHash(&str[0], str.length());
}

thash32 THashString::makeHash(const char* str, tsize len)
{
	thash32 b = 378551;
	thash32 a = 63689;
	thash32 hash = 0;

	for (std::size_t i = 0; i < len; i++)
	{
		hash = hash * a + str[i];
		a = a * b;
	}

	return (hash & 0x7FFFFFFF);
}