#include "MNFunction.h"
#include "MNObject.h"

MNFunction::MNFunction()
	: m_codes(NULL)
	, m_ncode(0)
	, m_nvars(0)
{

}

MNFunction::~MNFunction()
{
	if (m_codes != NULL) MNMemory::free(m_codes);
}

tsize    MNFunction::getVarCount() const
{
	return m_nvars;
}

tbyte*   MNFunction::getCode() const
{
	return &m_codes[0];
}

const MNObject& MNFunction::getConst(tsize idx)
{
	if (idx < m_consts.size()) return m_consts[idx];
	return MNObject::Null();
}