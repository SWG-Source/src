// ======================================================================
//
// OsMemory.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_OsMemory_H
#define INCLUDED_OsMemory_H

// ======================================================================

class OsMemory
{
public:
	static void install();
	static void remove();

	static void * reserve(size_t bytes);
	static void * commit(void *addr, size_t bytes);
	static bool   free(void *addr, size_t bytes);
	static bool   protect(void *addr, size_t bytes, bool allowAccess);
};

// ======================================================================

#endif // INCLUDED_OsMemory_H

