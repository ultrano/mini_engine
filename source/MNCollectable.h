#ifndef _H_MNCollectable
#define _H_MNCollectable

#include "MNCountable.h"
#include "MNObject.h"

class MNGlobal;
class MNCollectable : public MNCountable
{
	MN_RTTI(MNCollectable, MNCountable);

public:

	MNCollectable();
	~MNCollectable();

	MNGlobal* global() const;

	bool isMarked() const;
	void mark();
	void unmark();

	void setMeta(const MNObject& meta);
	const MNObject& getMeta() const;

	MNCollectable* link(MNGlobal* g);
	virtual void finalize();
	virtual void travelMark() = 0;

public:

	MNGlobal* m_global;
	MNCollectable* m_next;
	MNCollectable* m_prev;
	tbyte m_mark;
	MNObject m_meta;
};

#endif