#include "MNUserData.h"

MNUserData::MNUserData(tsize size, UserFinalizer uf)
	: m_data(NULL)
	, m_size(size)
    , m_finalizer(uf)
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

void MNUserData::finalize()
{
    if (m_finalizer != NULL)
        m_finalizer(m_data, m_size);
    
    __super::finalize();
}

void MNUserData::travelMark()
{
}