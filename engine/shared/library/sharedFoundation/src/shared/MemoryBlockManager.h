// ======================================================================
//
// MemoryBlockManager.h
//
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2000-2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MemoryBlockManager_H
#define INCLUDED_MemoryBlockManager_H

// ======================================================================

class MemoryBlockManager
{
public:

	static void  install(bool debugDumpOnRemove);

public:

	DLLEXPORT  MemoryBlockManager(char const * name, bool shared, int elementSize, int elementsPerBlock, int minimumNumberOfBlocks, int maximumNumberOfBlocks);
	DLLEXPORT ~MemoryBlockManager();

	const char *     getName() const;

	bool             isFull() const;
	DLLEXPORT int    getElementSize() const;
	int              getElementCount() const;

	DLLEXPORT void * allocate(bool returnNullOnFailure=false);
	DLLEXPORT void   free(void * pointer);

public:

#ifdef _DEBUG
	void  debugDump() const;
#endif

protected:

	// disable these routines
	MemoryBlockManager();
	MemoryBlockManager(const MemoryBlockManager &);
	MemoryBlockManager &operator =(const MemoryBlockManager &);

public:

	class Allocator;

private:

	char const * m_name;
	bool         m_shared;
	int          m_currentNumberOfElements;
	Allocator *  m_allocator;
};

// ======================================================================

inline char const * MemoryBlockManager::getName() const
{
	return m_name;
}

// ======================================================================

#endif
