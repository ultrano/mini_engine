#include "MNReferrer.h"
#include "MNCountable.h"

MNReferrer::MNReferrer(MNCountable* obj)
	: m_obj(obj)
	, m_hard(0)
	, m_weak(0)
{
}

MNReferrer::~MNReferrer()
{
}

void MNReferrer::decHard()
{
	if (!m_obj) return;
	if (m_hard == 0) return;
	--m_hard;
	if (m_hard == 0)
	{
		m_obj->finalize();
		m_obj = NULL;
	}
}
