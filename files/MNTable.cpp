#include "MNTable.h"
#include "MNObject.h"
#include "MNString.h"

struct MNTable::Node
{
	MNObject key;
	MNObject val;
	Node* next;
};

tboolean isEqulas(const MNObject& left, const MNObject& right)
{
	return (left.getType() == right.getType() && left.getHash() == right.getHash());
}

MNTable::MNTable(tsize size)
	: m_nodes(NULL)
	, m_frees(NULL)
	, m_size(0)
	, m_used(0)
{
	allocNodes(size);
}

MNTable::~MNTable()
{
	clear();
}

tboolean MNTable::insert(const MNObject& key, const MNObject& val)
{
	if (key.isNull()) return false;

	//! don't insert if it already is
	{
		Node* node = findNode(key);
		if (node)
		{
			node->val = val;
			return true;
		}
	}

	Node* freeNode = getFreeNode();
	if (!freeNode)
	{
		expandNodes();
		return insert(key, val);
	}

	tsize   index;
	m_used += 1;

	thash32 hash = key.getHash();
	index = hash % m_size;
	Node* head1 = &(m_nodes[index]);
	if (head1->key.isNull())
	{
		head1->key = key;
		head1->val = val;
		head1->next = NULL;
		return true;
	}

	index = head1->key.getHash() % m_size;
	Node* head2 = &(m_nodes[index]);
	if (head1 == head2)
	{
		freeNode->key = key;
		freeNode->val = val;
		freeNode->next = head1->next;
		head1->next = freeNode;
	}
	else
	{
		while (head2->next != head1) head2 = head2->next;
		freeNode->key = head1->key;
		freeNode->val = head1->val;
		freeNode->next = head1->next;
		head2->next = freeNode;

		head1->key = key;
		head1->val = val;
		head1->next = NULL;
	}

	return true;
}

tboolean MNTable::trySet(const MNObject& key, const MNObject& val)
{
	if (key.isNull()) return false;

	Node* node = findNode(key);
	if (node) node->val = val;

	return (node != NULL);
}

tboolean MNTable::tryGet(const MNObject& key, MNObject& val) const
{
	if (key.isNull()) return false;

	Node* node = findNode(key);
	if (node) val = node->val;

	return (node != NULL);
}

tboolean MNTable::hasKey(const MNObject& key)
{
	if (key.isNull()) return false;

	Node* node = findNode(key);
	return (node != NULL);
}

void  MNTable::clear()
{
	while (m_size--)
	{
		Node& node = m_nodes[m_size];
		node.~Node();
	}
	if (m_nodes) MNMemory::free((void*)m_nodes);
	m_nodes = NULL;
	m_size  = 0;
}

void  MNTable::allocNodes(tsize size)
{
	if (size == 0) return;
	Node* nodes = (Node*)MNMemory::malloc(sizeof(Node)*size);
	for (tsize i = 0; i < size; ++i)
	{
		Node* node = new((void*)&nodes[i]) Node();
		if ((i + 1) < size) node->next = &nodes[i + 1];
	}
	nodes[size - 1].next = NULL;
	m_frees = &nodes[size - 1];
	m_nodes = nodes;
	m_size  = size;
	m_used  = 0; 
}

void  MNTable::expandNodes()
{
	const tsize addSize = 1 << 3;
	Node* oldNodes = m_nodes;
	tsize oldSize = m_size;
	allocNodes(m_size + addSize);
	while (oldSize--)
	{
		Node& node = oldNodes[oldSize];
		insert(node.key, node.val);
		node.~Node();
	}
	if (oldNodes) MNMemory::free((void*)oldNodes);
}

MNTable::Node* MNTable::getFreeNode()
{
	if (!m_frees) return NULL;

	while (true)
	{
		Node* node = m_frees;
		if (node->key.isNull()) return node;

		if (m_frees != m_nodes) --m_frees;
		else break;
		
	}
	return NULL;
}

MNTable::Node* MNTable::findNode(const MNObject& key) const
{
	if (m_size == 0) return NULL;

	thash32 hash = key.getHash();
	tsize index = hash % m_size;
	
	Node* node = &(m_nodes[index]);
	while (node)
	{
		if (isEqulas(node->key, key)) break;
		node = node->next;
	}

	return node;
}

void MNTable::travelMark()
{
	for (tsize i = 0; i < m_size; ++i)
	{
		Node& node = m_nodes[i];
		MNCollectable* collectable = NULL;

		collectable = node.key.toCollectable();
		if (collectable) collectable->mark();

		collectable = node.val.toCollectable();
		if (collectable) collectable->mark();
	}
}

tsize MNTable::count() const
{
	return m_used;
}

tsize MNTable::total() const
{
	return m_size;
}

tboolean MNTable::iterate(tsize index, MNObject& key, MNObject& val) const
{
	if (index >= m_size) return false;
	Node& node = m_nodes[index];
	key = node.key;
	val = node.val;
	return true;
}