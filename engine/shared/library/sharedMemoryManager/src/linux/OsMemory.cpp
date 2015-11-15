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

void *OsMemory::commit(void *, size_t bytes)
{
	return ::malloc(bytes);
}

// ----------------------------------------------------------------------

bool OsMemory::free(void *addr, size_t)
{
	::free(addr);

	return true;
}

// ======================================================================
