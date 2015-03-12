#include "MNMemory.h"

#include <stdio.h>

//#ifdef PLATFORM_WIN32
//#pragma pack(push,1)
//#endif

#define MemBlockSize (sizeof(MemBlock))
struct MemBlock
{
	MemBlock* next;
	bool isUsing;

	inline bool isTail()
	{
		return (next == NULL);
	};

	inline unsigned int capacity()
	{
		if (isTail()) return 0;
		unsigned int p = (unsigned int)((char*)next - ((char*)this + MemBlockSize));
		return p;
	};

	inline void* memory()
	{
		return (void*)((char*)this + MemBlockSize);
	}

	inline void merge()
	{
		while (next != NULL)
		{
			if (next->isTail()) break;
			if (next->isUsing) break;
			next = next->next;
		}
	}

	inline void divide(unsigned int size)
	{
		if (capacity() <= (size + MemBlockSize)) return;

		MemBlock* sep = (MemBlock*)((char*)(memory()) + size);
		sep->isUsing = false;
		sep->next = next;
		next = sep;
	}
};

struct MemRod
{
	MemRod* next;
	unsigned int maximumCapacity;
	MemBlock*    cursor;
	MemBlock     original;

	static MemRod* createRod(unsigned int size)
	{
		if (size == 0) return NULL;
		unsigned int baseSize = (sizeof(MemRod) + MemBlockSize);
		unsigned int totalSize = baseSize + size;
		void* chunk = malloc(totalSize);

		MemRod*   rod = (MemRod*)chunk;
		MemBlock* head = (MemBlock*)&rod->original;
		MemBlock* tail = (MemBlock*)((char*)(chunk)+totalSize - MemBlockSize);

		rod->next = NULL;
		rod->cursor = head;

		head->next = tail;
		tail->next = NULL;

		head->isUsing = false;
		tail->isUsing = true;

		rod->maximumCapacity = head->capacity();

		return rod;
	}

	static void deleteRod(MemRod* rod)
	{
		::free((void*)rod);
	}

	MemBlock* request(unsigned int size)
	{
		if (size > maximumCapacity) return NULL;
		MemBlock* anchor = cursor;
		while (cursor != NULL)
		{
			if (false == cursor->isUsing)
			{
				cursor->merge();
				if (size <= cursor->capacity()) break;
			}
			cursor = cursor->next;
			if (cursor->isTail()) cursor = &original;
			if (cursor <= anchor && anchor < cursor->next) return NULL;
		}

		cursor->divide(size);

		return cursor;
	}

	void defragment()
	{
		MemBlock* block = &original;
		while (block != NULL)
		{
			if (block->isTail()) break;
			block->merge();
			block = block->next;
		}
		cursor = &original;
	}
};

//#ifdef PLATFORM_WIN32
//#pragma pack(pop)
//#endif

int& getMemCount()
{
	static int count = 0;
	return count;
}

int& getUsedBytes()
{
	static int count = 0;
	return count;
}

int& getMaxUsedBytes()
{
	static int count = 0;
	return count;
}

class MemCore
{
	enum { DEFAULT_MAX_CAPACITY = 512 };
	MemRod* m_fuelRod;
	MemRod* m_cursor;

	MemCore()
		: m_fuelRod(MemRod::createRod(DEFAULT_MAX_CAPACITY))
		, m_cursor(m_fuelRod)
	{
	};
	~MemCore()
	{
		printf("\n");
		printf("remain requested mem count: %d\n", getMemCount());
		printf("maximum used bytes: %dbytes\n", getMaxUsedBytes());
		MemRod* rod = m_fuelRod;
		while (rod)
		{
			MemRod* next = rod->next;
			MemRod::deleteRod(rod);
			rod = next;
		}
		m_fuelRod = NULL;
	}

public:

	static MemCore& instance()
	{
		static MemCore core;
		return core;
	}

	inline MemRod* addMemRod(unsigned int size)
	{
		unsigned int allocSize = (size > DEFAULT_MAX_CAPACITY) ? size : DEFAULT_MAX_CAPACITY;
		MemRod* newRod = MemRod::createRod(allocSize);
		newRod->next = m_fuelRod;
		return (m_fuelRod = newRod);
	}

	inline void* alloc(unsigned int size)
	{
		getUsedBytes() += size;
		if (getUsedBytes() > getMaxUsedBytes()) getMaxUsedBytes() = getUsedBytes();
		MemBlock* block = NULL;
		do
		{
			block = m_cursor->request(size);
			if (block != NULL) break;
			if (m_cursor->next == NULL)
			{
				m_cursor = addMemRod(size);
			}
			else
			{
				m_cursor = m_cursor->next;
			}
		} while (true);

		block->isUsing = true;

		return block->memory();
	}

	inline void dealloc(void* mem)
	{
		MemBlock* block = (MemBlock*)((char*)mem - MemBlockSize);
		block->isUsing = false;
		getUsedBytes() -= block->capacity();
	}

	void defragment()
	{
		m_cursor = m_fuelRod;
		while (m_cursor != NULL)
		{
			m_cursor->defragment();
			m_cursor = m_cursor->next;
		}
		m_cursor = m_fuelRod;
	}
};

void* MNMemory::operator new(size_t size)
{
	return MNMemory::malloc(size);
}

void MNMemory::operator delete(void* memory)
{
	MNMemory::free(memory);
}

void* MNMemory::malloc(size_t size)
{
	++getMemCount();
	//return ::malloc( size );
	return MemCore::instance().alloc(size);
}

void* MNMemory::realloc(void* mem, size_t size)
{
	MNMemory::free(mem);
	return MNMemory::malloc(size);
}

void MNMemory::free(void* mem)
{
	--getMemCount();
	//::free( mem );
	MemCore::instance().dealloc(mem);
}
