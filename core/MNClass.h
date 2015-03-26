#ifndef _H_MNClass
#define _H_MNClass

#include "MNCollectable.h"

class MNClass : public MNCollectable
{
	MN_RTTI(MNClass, MNCollectable);

public:

	enum Prop
	{
		Static   = 1<<0,
		Variable = 1<<1,
		Function = 1<<2,
	};
	struct Member
	{
		union
		{
			tint32 _int;
			struct { tflag8 prop; tuint16 index; };
		};
	};

	MNClass(tsize nmembers, MNClass* super);
	~MNClass();

	tboolean insert(const MNObject& key, const MNObject& mem);
	tboolean tryGet(const MNObject& key, MNObject& mem) const;
	tboolean hasKey(const MNObject& key);
	tboolean iterate(tsize& itor, MNObject& key, MNObject& mem) const;

private:

	virtual void travelMark();

private:

	MNClass* m_super;
	MNTable* m_members;

};

#endif