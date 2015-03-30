#ifndef _H_MNClass
#define _H_MNClass

#include "MNCollectable.h"

class MNClass : public MNCollectable
{
	MN_RTTI(MNClass, MNCollectable);

public:

	enum Prop
	{
		Static = 1<<0,
		Field  = 1<<1,
		Method = 1<<2,
	};
	struct Member
	{
		union
		{
			tint32 _int;
			struct { tflag8 prop; tuint16 index; };
		};
		Member() :_int(0) {}
	};

	MNClass(tsize nmembers, const MNObject& super);
	~MNClass();

	void     newInstance(MNObject& ret);
	tboolean addField(const MNObject& key, const MNObject& initVal);
	tboolean addMethod(const MNObject& key, const MNObject& methodVal);
	tboolean queryMember(const MNObject& key, Member& mem) const;
	tboolean tryGet(const MNObject& key, MNObject& val);
private:

	virtual void finalize();
	virtual void travelMark();

private:
	friend class MNInstance;
	MNClass* m_super;
	MNTable* m_members;
	tarray<MNObject> m_initVals;
	tarray<MNObject> m_methods;
};

#endif