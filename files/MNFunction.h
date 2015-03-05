#ifndef _H_MNFunction
#define _H_MNFunction

#include "MNCountable.h"

class MNObject;
class MNFunction : public MNCountable
{
	MN_RTTI(MNFunction, MNCountable);
	friend class MNFuncBuilder;
public:

	MNFunction();
	~MNFunction();

	tbyte*   getCode() const;
	const MNObject& getConst(tsize idx);

//private:
	tbyte* m_codes;
	tsize  m_ncode;
	tsize  m_nargs;
	tsize  m_nvars;
	tarray<MNObject> m_consts;
};

#endif