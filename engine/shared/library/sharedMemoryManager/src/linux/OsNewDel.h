// ======================================================================
//
// OsNewDel.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_OsNewDel_H
#define INCLUDED_OsNewDel_H

// ======================================================================
#include <cstddef>

enum MemoryManagerNotALeak
{
	MM_notALeak
};

void *operator new(size_t size, MemoryManagerNotALeak);
void *operator new(size_t size);
void *operator new[](std::size_t size);
void *operator new(size_t size, char const *file, int line);
void *operator new[](size_t size, char const *file, int line);

void operator delete(void *pointer) throw();
void operator delete[](void *pointer) throw();
void operator delete(void *pointer, char const *file, int line) throw();
void operator delete[](void *pointer, char const *file, int line) throw();

// ======================================================================

#endif // INCLUDED_OsNewDel_H

