#include "MNCollectable.h"
#include "MNGlobal.h"

enum { Unmarked, Marked, Finalized };

MNCollectable::MNCollectable()
	: m_global(NULL)
	, m_prev(NULL)
	, m_next(NULL)
	, m_mark(Unmarked)
{

}

MNCollectable::~MNCollectable()
{
	if (m_next) m_next->m_prev = m_prev;
	if (m_prev) m_prev->m_next = m_next;

	if (m_global->m_heap == this) m_global->m_heap = m_next;
}

MNGlobal* MNCollectable::global() const
{
	return m_global;
}

bool MNCollectable::isMarked() const
{
	return m_mark == Marked;
}

void MNCollectable::mark()
{
	if (m_mark != Marked)
	{
		m_mark = Marked;
		MNCollectable* collectable = m_meta.toCollectable();
		if (collectable) collectable->mark();
		travelMark();
	}
}
void MNCollectable::unmark()
{
	m_mark = Unmarked;
}

void MNCollectable::setMeta(const MNObject& meta)
{
	m_meta = meta;
}

const MNObject& MNCollectable::getMeta() const
{
	return m_meta;
}

MNCollectable* MNCollectable::link(MNGlobal* g)
{
	m_global = g;
	m_next = g->m_heap;
	m_prev = NULL;

	if (g->m_heap) g->m_heap->m_prev = this;
	g->m_heap = this;

	return this;
}

void MNCollectable::finalize()
{
	if (m_mark == Finalized) return;
	m_mark = Finalized;
/*
	if (getMeta().isObject())
	{
		MNValue __finalizer = getMeta().get((MNGCObject*)m_state->newString("__finalizer"));
		if (__finalizer.to<MNClosure>()) __finalizer.call(this);
	}*/

	__super::finalize();
}
