// ======================================================================
//
// MemoryManager.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_MemoryManager_H
#define INCLUDED_MemoryManager_H

// ======================================================================

#include "sharedDebug/DebugHelp.h"

// ======================================================================

#include <new>
#include "sharedMemoryManager/OsNewDel.h"

// ======================================================================
// Memory manager class.
//
// This class API is multi-thread safe.
//
// This class provides extensive debugging features for applications, including
// overwrite guard bands, initialize pattern fills, free pattern fills, and 
// memory tracking.

class MemoryManager
{
public:

	MemoryManager();
	~MemoryManager();

	static void            setLimit(int megabytes, bool hardLimit, bool preallocate);
	static int             getLimit();
	static bool            isHardLimit();

	static void            registerDebugFlags();
	static void            debugReport();
	static void            debugReportMap();
	static bool            reportToFile(const char * fileName, bool leak);

	static int             getCurrentNumberOfAllocations();
	static unsigned long   getCurrentNumberOfBytesAllocated(const int processId = 0);
	static unsigned long   getCurrentNumberOfBytesAllocatedNoLeakTest();
	static int             getMaximumNumberOfAllocations();
	static unsigned long   getMaximumNumberOfBytesAllocated();
	static int             getSystemMemoryAllocatedMegabytes();

#ifndef _WIN32
	static int             getProcessVmSizeKBytes(const int processId = 0);
#endif

	static DLLEXPORT void *allocate(size_t size, uint32 owner, bool array, bool leakTest);
	static DLLEXPORT void  free(void *pointer, bool array);
	static DLLEXPORT void  own(void *pointer);
	static void *          reallocate(void *userPointer, size_t newSize);

	static void            verify(bool guardPatterns, bool freePatterns);
	static void            setReportAllocations(bool reportAllocations);
	static void            report();

private:

	// disabled
	MemoryManager(MemoryManager const &);
	MemoryManager &operator =(MemoryManager const &);
};

// ======================================================================

#ifdef _DEBUG
	#define MEM_OWN(a) MemoryManager::own(a)
#else
	#define MEM_OWN(a) UNREF(a)
#endif

// ======================================================================

#endif
