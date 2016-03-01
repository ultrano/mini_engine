#ifndef _H_MNReferrer
#define _H_MNReferrer

#include "MNMemory.h"
#include "MNPrimaryType.h"

class MNCountable;
class MNReferrer : public MNMemory
{
	friend class MNCountable;
public:

	MNReferrer(MNCountable* obj);
	~MNReferrer();

	inline void incWeak() { ++m_weak; }
	inline void decWeak() { if (--m_weak == 0) delete this; }
	inline MNCountable* getObject() const { return m_obj; }
	inline tuint16 getHard() const { return m_hard; }
	inline tuint16 getWeak() const { return m_weak; }
	inline void incHard() { if (!m_obj) return; ++m_hard; }
	void decHard();

	bool isUndead() const { return (getHard() == 0 && getObject()); }
private:
	MNCountable* m_obj;
	tuint16 m_hard;
	tuint16 m_weak;
};

#endif