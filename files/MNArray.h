#ifndef _H_MNArray
#define _H_MNArray

#include "MNCollectable.h"

class MNObject;
class MNArray : public MNCollectable
{
	MN_RTTI(MNArray, MNCollectable);
public:
	
	MNArray();
	~MNArray();

	tboolean add(const MNObject& val);
	tboolean tryGet(const MNObject& key, MNObject& val) const;
	tboolean trySet(const MNObject& key, const MNObject& val);
	tsize    count() const;
protected:
	virtual void travelMark();
private:
	tarray<MNObject> m_arr;
};

#endif