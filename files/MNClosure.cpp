#include "MNClosure.h"
#include "MNFiber.h"

//////////////////////////////////////////////////////////////////////////

Opened::Opened()
	: fiber(NULL)
	, index(0)
{

}

Opened::Opened(MNFiber* f, tuint i)
	: fiber(f)
	, index(i)
{

}

Opened::~Opened()
{

}

bool Opened::isOpened()
{
	return true;
}

const MNObject& Opened::get()
{
	return fiber->getAt(index);
}

void Opened::set(const MNObject& v)
{
	fiber->setAt(index, v);
}


//////////////////////////////////////////////////////////////////////////

UpLink::UpLink() : link(NULL)
{

}

UpLink::UpLink(const MNObject& v)
	: link(new Closed(v))
{

}

UpLink::UpLink(MNFiber* fiber, tuint index)
	: link(new Opened(fiber, index))
{

}

UpLink::~UpLink()
{
	if (link) delete link;
}

const MNObject& UpLink::get()
{
	if (link) return link->get();
	static MNObject null;
	return null;
}

void UpLink::set(const MNObject& val)
{
	if (link) link->set(val);
}

void UpLink::close()
{
	if (link && link->isOpened())
	{
		Closed* cl = new Closed;
		Opened* ol = (Opened*)link;
		cl->val = ol->get();
		delete ol;
		link = cl;
	}
}

void UpLink::inc()
{
	nref += 1;
}

void UpLink::dec()
{
	if (--nref == 0) delete this;
}

////////////////////////////////////////////////////////////////////////////////////

Closed::Closed()
{
}

Closed::Closed(const MNObject& v)
	: val(v)
{

}

Closed::~Closed()
{
	val = MNObject();
}

bool Closed::isOpened()
{
	return false;
}

const MNObject& Closed::get()
{
	return val;
}

void Closed::set(const MNObject& v)
{
	val = v;
}
///////////////////////////////////////////////////////////////////////////////////

MNClosure::MNClosure()
{

}

MNClosure::MNClosure(MNObject func)
	: m_func(func)
{

}

MNClosure::~MNClosure()
{
	//printf("~MNClosure\n");
	for (tsize i = 0; i < m_uplinks.size(); ++i)
	{
		if (UpLink* link = m_uplinks[i])
		{
			link->dec();
		}
	}
}

UpLink*	MNClosure::getLink(tuint idx)
{
	return (idx < m_uplinks.size()) ? m_uplinks[idx] : NULL;
}

void	MNClosure::addLink(UpLink* link)
{
	if (!link) return;
	link->inc();
	m_uplinks.push_back(link);
}

const MNObject&	MNClosure::getUpval(tuint idx)
{
	UpLink* link = getLink(idx);
	if (link) return link->get();
	static MNObject null;
	return null;
}

void	MNClosure::setUpval(tuint idx, const MNObject& v)
{
	UpLink* link = getLink(idx);
	if (link) link->set(v);
}

bool MNClosure::isNative() const
{
	return m_func.getType() == TObjectType::CFunction;
}

void MNClosure::bindThis(const MNObject& _this)
{
	m_this = _this;
}

const MNObject& MNClosure::getThis()
{
	return m_this;
}

const MNObject& MNClosure::getFunc() const
{
	return m_func;
}

void MNClosure::setFunc(const MNObject& func)
{
	m_func = func;
}

void MNClosure::clone(MNObject& cls) const
{
	MNClosure* closure = new MNClosure(m_func);
	closure->link(global());
	closure->setMeta(getMeta());
	tsize sz = m_uplinks.size();
	for (tsize i = 0; i < sz; ++i) closure->addLink(m_uplinks[i]);
	cls = MNObject(TObjectType::Closure, closure->getReferrer());
}

void MNClosure::travelMark()
{
	for (tsize i = 0; i < m_uplinks.size(); ++i)
	{
		UpLink* link = m_uplinks[i];
		if (MNCollectable* collectable = link->get().toCollectable()) collectable->mark();
	}
	if (MNCollectable* collectable = m_func.toCollectable()) collectable->mark();
	if (MNCollectable* collectable = m_this.toCollectable()) collectable->mark();
}