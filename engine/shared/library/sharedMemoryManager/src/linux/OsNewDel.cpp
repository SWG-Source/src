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

