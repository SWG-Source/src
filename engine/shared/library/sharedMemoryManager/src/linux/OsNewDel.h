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

enum MemoryManagerNotALeak
{
	MM_notALeak
};

void *operator new(size_t size, MemoryManagerNotALeak) throw(std::bad_alloc);
void *operator new(size_t size) throw(std::bad_alloc);
void *operator new[](size_t size) throw(std::bad_alloc);
void *operator new(size_t size, char const *file, int line) throw(std::bad_alloc);
void *operator new[](size_t size, char const *file, int line) throw(std::bad_alloc);

void operator delete(void *pointer) throw();
void operator delete[](void *pointer) throw();
void operator delete(void *pointer, char const *file, int line) throw();
void operator delete[](void *pointer, char const *file, int line) throw();

// ======================================================================

#endif // INCLUDED_OsNewDel_H

