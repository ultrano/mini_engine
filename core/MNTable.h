#ifndef _H_MNTable
#define _H_MNTable

#include "MNCollectable.h"

class MNObject;
class MNTable : public MNCollectable
{
	MN_RTTI(MNTable, MNCollectable);
public:
	
	MNTable(tsize size = 0);
	~MNTable();

	tboolean insert(const MNObject& key, const MNObject& val);
	tboolean trySet(const MNObject& key, const MNObject& val);
	tboolean tryGet(const MNObject& key, MNObject& val) const;
	tboolean hasKey(const MNObject& key);
	void     clear();
	tsize    count() const;
	tsize    total() const;
	tboolean iterate(tsize& itor, MNObject& key, MNObject& val) const;
private:
	struct Node;

	void  allocNodes(tsize size);
	void  expandNodes();
	Node* getFreeNode();
	Node* findNode(const MNObject& key) const;;

	virtual void travelMark();

private:
	Node* m_frees;
	Node* m_nodes;
	tsize m_size;
	tsize m_used;
};

#endif