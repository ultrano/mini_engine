#include "MNGlobal.h"
#include "MNTable.h"
#include "MNFiber.h"
#include "MNBasicLib.h"

MNGlobal::MNGlobal(MNFiber* root)
	: m_heap(NULL)
	, m_root(root)
	, m_stringTable(new MNTable())
	, m_shared(NULL)
{
	root->m_global = this;
	m_stringTable->link(this);

	//! global table
	m_shared = new MNTable();
	m_shared->link(this);
	root->push(MNObject(TObjectType::Table, m_shared->getReferrer()));

	//! common function
	MNBasicLib(root);

	//! cache table
	{
		root->up(1, 0);
		root->push_string("cache");
		root->push_table();
		root->store_field();
	}
}

MNGlobal::~MNGlobal()
{
	m_stringTable->finalize();
	m_stringTable = NULL;
}

void MNGlobal::getString(MNObject& ret, const tstring& str)
{
	MNObject key = MNObject::String(str);
	if (!m_stringTable->tryGet(key, ret))
	{
		m_stringTable->insert(key, key);
		ret = key;
	}
}

void MNGlobal::finalize()
{
	while (m_heap != NULL)
	{
		if (m_heap == m_root) m_heap = m_heap->m_next;
		else m_heap->finalize();
	}
	m_root->finalize();
	delete this;
}

tsize MNGlobal::GC()
{
	//! clear marks in heap
	{
		for (MNCollectable* gct = m_heap; gct != NULL; gct = gct->m_next) gct->unmark();
	}

	//! make mark travel from root
	{
		m_root->mark();
		m_stringTable->mark();
	}

	tsize count = 0;
	//! delete unmarked objects and fold link
	{
		while (m_heap != NULL && !m_heap->isMarked())
		{
			count += 1;
			m_heap->finalize();
		}

		MNCollectable* marked = m_heap;
		while (marked != NULL)
		{
			MNCollectable* unknown = marked->m_next;
			if (unknown != NULL && !unknown->isMarked())
			{
				count += 1;
				unknown->finalize();
				continue;
			}
			marked = unknown;
		}
	}
	return count;
}