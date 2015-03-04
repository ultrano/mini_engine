#ifndef _H_MNMemory
#define _H_MNMemory

#include <new>
#include <stdlib.h>

class MNMemory
{
public:

	static void* malloc(size_t size);
	static void* realloc(void* mem, size_t size);
	static void  free(void* mem);

	static	void*	operator new(size_t size);
	static	void	operator delete(void* memory);

};

#endif