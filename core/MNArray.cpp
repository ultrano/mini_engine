#include "MNArray.h"

struct Comparer
{
	const MNObject& val;
	Comparer(const MNObject& v):val(v){}
	bool operator ()(const MNObject& v)
	{
		return (v.getType() == val.getType() && v.getHash() == val.getHash());
	}
};

MNArray::MNArray(tsize size)
{
	m_arr.resize(size);
}

MNArray::~MNArray()
{

}

tboolean MNArray::add(const MNObject& val)
{
	m_arr.push_back(val);
	return true;
}

tboolean MNArray::tryGet(const MNObject& key, MNObject& val) const
{
	if (!key.isInt() && !key.isFloat()) return false;
	tsize idx = key.toInt();
	tboolean ret = (idx < m_arr.size());
	if (ret) val = m_arr[idx];
	return ret;
}

tboolean MNArray::trySet(const MNObject& key, const MNObject& val)
{
	if (!key.isInt() && !key.isFloat()) return false;
	tsize idx = key.toInt();
	tboolean ret = (idx < m_arr.size());
	if (ret) m_arr[idx] = val;
	return ret;
}

tboolean MNArray::remove(const MNObject& val)
{
	Comparer comparer(val);
	tarray<MNObject>::iterator itor = std::remove_if(m_arr.begin(), m_arr.end(), comparer);
	bool ret = (itor != m_arr.end());
	m_arr.erase(itor, m_arr.end());
	return ret;
}

tsize MNArray::count() const
{
	return m_arr.size();
}

tboolean MNArray::iterate(tsize index, MNObject& val) const
{
	if (index >= m_arr.size()) return false;
	val = m_arr[index];
	return true;
}

void MNArray::travelMark()
{
	tsize count = m_arr.size();
	while (count--)
	{
		const MNObject& val = m_arr[count];
		MNCollectable* collectable = val.toCollectable();
		if (collectable) collectable->mark();
	}
}