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

void * __cdecl operator new(size_t size, MemoryManagerNotALeak);
void * __cdecl operator new(size_t size);
void * __cdecl operator new[](size_t size);
void * __cdecl operator new(size_t size, char const *file, int line);
void * __cdecl operator new[](size_t size, char const *file, int line);
void * __cdecl operator new(size_t size, void *placement);

void operator delete(void *pointer);
void operator delete[](void *pointer);
void operator delete(void *pointer, char const *file, int line);
void operator delete[](void *pointer, char const *file, int line);
void operator delete(void *pointer, void *placement);

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE

inline void *operator new(size_t size, void *placement)
{
	static_cast<void>(size);
	return placement;
}

inline void operator delete(void *pointer, void *placement)
{
	static_cast<void>(pointer);
	static_cast<void>(placement);
}

#endif // __PLACEMENT_NEW_INLINE

// ======================================================================

#endif INCLUDED_OsNewDel_H

