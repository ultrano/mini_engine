#ifndef _H_MNInstance
#define _H_MNInstance

#include "MNCollectable.h"
#include "MNObject.h"

class MNClass;
class MNInstance : public MNCollectable
{
	MN_RTTI(MNInstance, MNCollectable);

public:

	MNInstance(const MNObject& _class);
	~MNInstance();

	tboolean trySet(const MNObject& key, const MNObject& val);
	tboolean tryGet(const MNObject& key, MNObject& val) const;

protected:

	virtual void travelMark();

private:

	tarray<MNObject> m_fields;
	MNClass* m_class;
};

#endif