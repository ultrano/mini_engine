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
	for (tsize i = 0; i < m_uplinks.size(); ++i)
	{
		if (UpLink* link = m_uplinks[i])
		{
			link->nref -= 1;
			if (link->nref == 0) delete link;
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
	link->nref += 1;
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

void MNClosure::bindThis(MNObject _this)
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