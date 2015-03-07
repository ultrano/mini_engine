#ifndef _H_MNString
#define _H_MNString

#include "MNCountable.h"

class MNString : public MNCountable
{
	MN_RTTI(MNString, MNCountable);
public:

	MNString(const tstring& str);
	~MNString();

	const thashstring& ss() const { return m_ss; }

private:
	const thashstring m_ss;
};

#endif