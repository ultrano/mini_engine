#ifndef _H_MNUserData
#define _H_MNUserData

#include "MNCollectable.h"

class MNUserData : public MNCollectable
{
	MN_RTTI(MNUserData, MNCollectable);
public:

	MNUserData(tsize size, UserFinalizer uf);
	~MNUserData();

	void* getData() const;
	tsize getSize() const;

private:
    
	virtual void finalize();
	virtual void travelMark();

private:
	void* m_data;
	tsize m_size;
    UserFinalizer m_finalizer;
};

#endif