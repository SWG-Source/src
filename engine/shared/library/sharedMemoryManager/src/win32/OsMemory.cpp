// ======================================================================
//
// OsMemory.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedMemoryManager/FirstSharedMemoryManager.h"
#include "sharedMemoryManager/OsMemory.h"

// ======================================================================

void OsMemory::install()
{
}

// ----------------------------------------------------------------------

void OsMemory::remove()
{
}

// ----------------------------------------------------------------------

void *OsMemory::reserve(size_t bytes)
{
	return VirtualAlloc(0, bytes, MEM_RESERVE, PAGE_READWRITE);
}

// ----------------------------------------------------------------------

void *OsMemory::commit(void *addr, size_t bytes)
{
	return VirtualAlloc(addr, bytes, MEM_COMMIT, PAGE_READWRITE);
}

// ----------------------------------------------------------------------

bool OsMemory::free(void *addr, size_t bytes)
{
	UNREF(bytes);
	return VirtualFree(addr, 0, MEM_RELEASE) ? true : false;
}

// ----------------------------------------------------------------------

bool OsMemory::protect(void *addr, size_t bytes, bool allowAccess)
{
	DWORD old;
	BOOL result = VirtualProtect(addr, bytes, allowAccess ? PAGE_READWRITE : PAGE_NOACCESS, &old);
	return result ? true : false;
}

// ======================================================================

