#ifndef _H_MNCountable
#define _H_MNCountable

#include "MNPrimaryType.h"
#include "MNMemory.h"
#include "MNRtti.h"

class MNReferrer;

class MNCountable : public MNMemory
{
	MN_RTTI_ROOT(MNCountable);
	friend class MNReferrer;
public:

	MNCountable();
	virtual ~MNCountable();

	MNReferrer* getReferrer() const { return m_ref; };

	virtual void finalize();

private:
	MNReferrer* m_ref;
};

#endif