// ======================================================================
//
// OsNewDel.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedMemoryManager/FirstSharedMemoryManager.h"
#include "sharedMemoryManager/MemoryManager.h"
#include "sharedMemoryManager/OsNewDel.h"

#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>

static MemoryManager memoryManager __attribute__ ((init_priority (101)));

#define USE_LIBC_MALLOC_HOOKS 0

#if USE_LIBC_MALLOC_HOOKS
extern "C" 
{
	void memoryManagerFreeHook(__malloc_ptr_t __ptr, __const __malloc_ptr_t)
	{
		delete[] (char *)__ptr;
	}
	
	__malloc_ptr_t memoryManagerMallocHook(size_t __size, const __malloc_ptr_t)
	{
		return new char[__size];
	}
	
	__malloc_ptr_t memoryManagerReallocHook(__malloc_ptr_t __ptr, size_t size, __const __malloc_ptr_t)
	{
		if(! __ptr)
			return new char[size];
		
		return MemoryManager::reallocate(__ptr, size);
	}

	__malloc_ptr_t memoryManagerMemAlignHook(size_t alignment, size_t size, __const __malloc_ptr_t)
	{
		DEBUG_FATAL(true, ("memalign not implemented!"));
		return new char[size];		
	}

	static void memoryManagerMallocInitializeHook(void)
	{
		__free_hook = memoryManagerFreeHook;
		__malloc_hook = memoryManagerMallocHook;
		__realloc_hook = memoryManagerReallocHook;
		__memalign_hook = memoryManagerMemAlignHook;
	}
	void (*__malloc_initialize_hook) (void) = memoryManagerMallocInitializeHook;
}

#endif//USE_LIBC_MALLOC_HOOKS

// ======================================================================

void *operator new(size_t size, MemoryManagerNotALeak)
{
	return MemoryManager::allocate(size, reinterpret_cast<uint32>(__builtin_return_address(0)), false, false);
}

// ----------------------------------------------------------------------

void *operator new(std::size_t size)
{
	return MemoryManager::allocate(size, reinterpret_cast<uint32>(__builtin_return_address(0)), false, true);
}

// ----------------------------------------------------------------------

void *operator new[](size_t size)
{
	return MemoryManager::allocate(size, reinterpret_cast<uint32>(__builtin_return_address(0)), true, true);
}

// ----------------------------------------------------------------------

void *operator new(size_t size, const char *file, int line)
{
	return MemoryManager::allocate(size, reinterpret_cast<uint32>(__builtin_return_address(0)), false, true);
}

// ----------------------------------------------------------------------

void *operator new[](size_t size, const char *file, int line)
{
	return MemoryManager::allocate(size, reinterpret_cast<uint32>(__builtin_return_address(0)), true, true);
}

// ----------------------------------------------------------------------

void operator delete(void *pointer) throw()
{
	if (pointer)
		MemoryManager::free(pointer, false);
}

// ----------------------------------------------------------------------

void operator delete[](void *pointer) throw()
{
	if (pointer)
		MemoryManager::free(pointer, true);
}

// ----------------------------------------------------------------------

void operator delete(void *pointer, const char *file, int line) throw()
{
	UNREF(file);
	UNREF(line);

	if (pointer)
		MemoryManager::free(pointer, false);
}

// ----------------------------------------------------------------------

void operator delete[](void *pointer, const char *file, int line) throw()
{
	UNREF(file);
	UNREF(line);

	if (pointer)
		MemoryManager::free(pointer, true);
}

// ======================================================================

