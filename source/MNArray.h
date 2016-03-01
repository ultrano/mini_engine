#ifndef _H_MNArray
#define _H_MNArray

#include "MNCollectable.h"

class MNObject;
class MNArray : public MNCollectable
{
	MN_RTTI(MNArray, MNCollectable);
public:
	
	MNArray(tsize size = 0);
	~MNArray();

	tboolean add(const MNObject& val);
	tboolean remove(const MNObject& val);
	void     clear();
	tboolean tryGet(const MNObject& key, MNObject& val) const;
	tboolean trySet(const MNObject& key, const MNObject& val);
	tsize    count() const;
	tboolean iterate(tsize& itor, MNObject& val) const;
protected:
	virtual void travelMark();
private:
	tarray<MNObject> m_arr;
};

#endif