#include "MNUserData.h"

MNUserData::MNUserData(tsize size)
	: m_data(NULL)
	, m_size(size)
{
	if (m_size > 0) m_data = MNMemory::malloc(m_size);
}

MNUserData::~MNUserData()
{
	if (m_data) MNMemory::free(m_data);
}

void* MNUserData::getData() const
{
	return m_data;
}

tsize MNUserData::getSize() const
{
	return m_size;
}
void MNUserData::travelMark()
{
}