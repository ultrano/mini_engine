#include "MNCountable.h"
#include "MNReferrer.h"

MNCountable::MNCountable()
{
	m_ref = new MNReferrer(this);
	m_ref->incWeak();
}

MNCountable::~MNCountable()
{
	if (m_ref) finalize();
}

void MNCountable::finalize()
{
	if (!m_ref) return;

	//printf("%s is filnalized\n", queryRtti()->name);

	MNReferrer* ref = m_ref;
	m_ref = NULL;
	delete this;
	ref->m_obj = NULL;
	ref->m_hard = 0;
	ref->decWeak();
}