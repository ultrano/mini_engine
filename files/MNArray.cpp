#include "MNArray.h"

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

tsize MNArray::count() const
{
	return m_arr.size();
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