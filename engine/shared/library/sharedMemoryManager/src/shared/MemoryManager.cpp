// ======================================================================
//
// MemoryManager.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedMemoryManager/FirstSharedMemoryManager.h"
#include "sharedMemoryManager/MemoryManager.h"
#include "sharedMemoryManager/OsMemory.h"

#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/Production.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/DebugMonitor.h"
#include "sharedDebug/PixCounter.h"
#include "sharedDebug/Profiler.h"
#include "sharedSynchronization/RecursiveMutex.h"
#include "sharedDebug/RemoteDebug.h"

#include <cstdio>

#ifdef _WIN32
#include <io.h>
#include <crtdbg.h>
#else
#include <sys/mman.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <new>

// the below seems to bare minimum cause the DB process to segfault
// too bad, if we weren't using an omnibus mem manager we could potentially maximize usage
#define DISABLE_MEMORY_MANAGER 1

//lint -e826 // Suspicious pointer-to-pointer conversion (area too small)

#if !DISABLE_MEMORY_MANAGER

// ======================================================================

// this flag is different than above, as it doesn't fully disable the manager, only a few pieces
// including the destructor for some reason
#define DISABLED                1

// removed all the debug cases for these as these seem to cause problems
// recent modifications force the mem manager to always behave in production mode
// other areas will remain unaffected
#define DO_TRACK               0
#define DO_SCALAR              0
#define DO_GUARDS              0
#define DO_INITIALIZE_FILLS    0
#define DO_FREE_FILLS          0

// ======================================================================

namespace MemoryManagerNamespace
{
	class Block;

	class SystemAllocation
	{
	public:

		SystemAllocation(int size);
		~SystemAllocation();

		int getSize();

		SystemAllocation * getNext();
		void setNext(SystemAllocation *);

		Block * getFirstMemoryBlock();
		Block * getLastMemoryBlock();

	private:

		int m_size;
		SystemAllocation * m_next;
		int m_pad1;
		int m_pad2;
	};


	class Block
	{
	public:

		enum { cms_requestedSizeBits = 30 };

	public:

		Block * getPrevious();
		Block const * getPrevious() const;
		void    setPrevious(Block *previous);

		Block * getNext();
		Block const * getNext() const;
		void    setNext(Block *previous);

		bool    isFree() const;
		void    setFree(bool free);

		int     getSize() const;

	private:

		Block *        m_previous;
		Block *        m_next;
		bool           m_free:1;

	protected:

		// these are only in allocated blocks, but we can't expect a compiler to merge bitfields across multiple classes
#if DO_SCALAR
		bool           m_array:1;
#endif
#if DO_TRACK
		bool           m_leakTest:0;
#endif
#if DO_TRACK || DO_GUARDS
		unsigned int   m_requestedSize:cms_requestedSizeBits;
#endif
	};

	class FreeBlock;

	void       addToFreeList(Block * block);
	void       removeFromFreeList(FreeBlock * block);
	FreeBlock *searchFreeList(int blockSize);

	class FreeBlock : public Block
	{
	private:

		friend void       addToFreeList(Block *block);
		friend void       removeFromFreeList(FreeBlock *block);
		friend FreeBlock *searchFreeList(int blockSize);

	private:
		FreeBlock *    m_smallerFreeBlock;
		FreeBlock *    m_sameFreeBlock;
		FreeBlock *    m_largerFreeBlock;
		FreeBlock *    m_parentFreeBlock;
	};

	class AllocatedBlock : public Block
	{
	public:

#if DO_SCALAR
		bool  isAllocatedAsArray() const;
		void  setAllocatedAsArray(bool array);
#endif

#if DO_TRACK
		bool  checkForLeaks() const;
		void  setCheckForLeaks(bool checkForLeaks);

		uint32 getOwner(int index) const;
		void   setOwner(int index, uint32 owner);
		void   fillOwnerWithFreePattern();

#endif

#if DO_TRACK || DO_GUARDS
		int    getRequestedSize() const;
		void   setRequestedSize(int requestedSize);
#endif

	private:

#if DO_TRACK
		uint32         m_owner[DO_TRACK];
#endif
	};

#ifdef _DEBUG
	void logAllocationsNextFrame();
#endif

	int convertBytesToMegabytesForSystemAllocation(int bytes);
	void allocateSystemMemory(int megabytes);

	int          min(int a, int b);
	int          max(int a, int b);
	byte const * min(byte const * a, byte const * b);
	byte const * max(byte const * a, byte const * b);

	int    report(bool leak);
	void   report(AllocatedBlock const * block, bool leak);

	int    quickGuessIsValidPointerFromHeap(void const *);
	int    authoritativeIsValidPointerFromHeap(void const *);

	void    emitCharacters(char * & buffer, int blockSize, int & carryOverFree, int & carryOverUsed, int newFree, int newUsed, char const * const bufferOverrunAddress);

	bool   reportToFile(char const *fileName, bool leak);
	void   outputDebugStringWrapper(char const * message);
	void   logMessageToFd(char const * message);

	int const             cms_blockSize              = (sizeof(Block) + 15) & (~15);
	int const             cms_freeBlockSize          = (sizeof(FreeBlock) + 15) & (~15);
	extern int const      cms_allocatedBlockSize     = (sizeof(AllocatedBlock) + 15) & (~15);
	byte const            cms_guardFillPattern       = 0xAB;
	byte const            cms_initializeFillPattern  = 0xCD;
	byte const            cms_freeFillPattern        = 0xEF;
	int const             cms_guardBandSize          = DO_GUARDS ? 16 : 0;

	int const cms_systemAllocationRoundSize = 4 * 1024 * 1024;
	int const cms_systemAllocationMinimumSize = 4 * 1024 * 1024;

	bool                  ms_installed;
	bool                  ms_limitSet;
	bool                  ms_hardLimit;
	int                   ms_limitMegabytes = 3071;
	SystemAllocation *    ms_firstSystemAllocation;
	int                   ms_numberOfSystemAllocations;
	int                   ms_systemMemoryAllocatedMegabytes;
	bool                  ms_reportAllocations;

#ifdef _DEBUG
        PixCounter::ResetInteger ms_allocationsPerFrame;
        PixCounter::ResetInteger ms_bytesAllocatedPerFrame;
        PixCounter::ResetInteger ms_freesPerFrame;
        PixCounter::ResetInteger ms_bytesFreedPerFrame;

	bool                  ms_debugReportFlag;
	bool                  ms_debugReportMapFlag;
	bool                  ms_debugReportAllocations;
	bool                  ms_debugReportLogMemoryAllocFreePointers;
	bool                  ms_debugLogAllocations;
	bool                  ms_debugLogAllocationsNextFrame;
	bool                  ms_debugVerifyGuardPatterns;
	bool                  ms_debugVerifyFreePatterns;
	bool                  ms_debugProfileAllocate;
	bool                  ms_debugLogAllocationsNextFrameStarted;
#endif

	RecursiveMutex *      ms_criticalSection;

	char                  ms_memoryManagerBuffer[sizeof(MemoryManager)];

	FreeBlock *           ms_firstFreeBlock;

	int                   ms_allocateCalls;
	unsigned long         ms_allocateBytesTotal;
#ifndef _WIN32
	int                   ms_processVmSizeKBytes;
#endif
	int                   ms_freeCalls;

	int                   ms_allocations;
	int                   ms_maxAllocations;

	int                   ms_freeBlocks;
	int                   ms_maxFreeBlocks;
#ifdef _DEBUG
	int                   ms_maxTreeHeight;
#endif

#if DO_TRACK || DO_GUARDS
	unsigned long         ms_currentBytesRequested;
#endif
	unsigned long         ms_currentBytesAllocated;
#if DO_TRACK
	unsigned long         ms_currentBytesAllocatedNoLeakTest;
#endif
	unsigned long         ms_maxBytesAllocated;

	bool                  ms_allowNameLookup = true;
	int                   ms_logMessageFd = -1;
	void                  (*LogMessage)(char const * message) = &outputDebugStringWrapper;

	PROFILER_BLOCK_DEFINE(ms_allocateProfilerBlock, "MemoryManager::allocate");
}
using namespace MemoryManagerNamespace;

// ======================================================================

SystemAllocation::SystemAllocation(int size)
:
	m_size(size),
	m_next(nullptr),
	m_pad1(0),
	m_pad2(0)
{
	DEBUG_FATAL(sizeof(*this) != cms_blockSize, ("SystemAllocation size is not %d bytes", cms_blockSize));

#if DO_FREE_FILLS
	// fill the newly allocated user memory with the free pattern
	imemset(reinterpret_cast<byte*>(this) + cms_blockSize, cms_freeFillPattern, m_size - cms_blockSize);
#endif

	Block * firstMemoryBlock = getFirstMemoryBlock();
	Block * firstFreeBlock = 	reinterpret_cast<Block *>(reinterpret_cast<byte *>(firstMemoryBlock) + cms_blockSize);
	Block * lastMemoryBlock = getLastMemoryBlock();

	// set up the prefix sentinel block
	firstMemoryBlock->setPrevious(nullptr);
	firstMemoryBlock->setNext(firstFreeBlock);
	firstMemoryBlock->setFree(false);

	firstFreeBlock->setPrevious(firstMemoryBlock);
	firstFreeBlock->setNext(lastMemoryBlock);
	firstFreeBlock->setFree(true);

	// set up the suffix sentinel block
	lastMemoryBlock->setPrevious(firstFreeBlock);
	lastMemoryBlock->setNext(nullptr);
	lastMemoryBlock->setFree(false);

	// put the first block on the free list
	addToFreeList(firstFreeBlock);
}

// ----------------------------------------------------------------------

SystemAllocation::~SystemAllocation()
{
	OsMemory::free(this, m_size);
}

// ----------------------------------------------------------------------

int SystemAllocation::getSize()
{
	return m_size;
}

// ----------------------------------------------------------------------

SystemAllocation * SystemAllocation::getNext()
{
	return m_next;
}

// ----------------------------------------------------------------------

void SystemAllocation::setNext(SystemAllocation * next)
{
	m_next = next;
}

// ----------------------------------------------------------------------

Block * SystemAllocation ::getFirstMemoryBlock()
{
	return reinterpret_cast<Block *>(reinterpret_cast<byte *>(this) + cms_blockSize);
}

// ----------------------------------------------------------------------

Block * SystemAllocation ::getLastMemoryBlock()
{
	return reinterpret_cast<Block *>(reinterpret_cast<byte *>(this) + m_size - cms_blockSize);
}

// ======================================================================

inline Block * Block::getPrevious()
{
	return m_previous;
}

// ----------------------------------------------------------------------

inline Block const * Block::getPrevious() const
{
	return m_previous;
}

// ----------------------------------------------------------------------

inline void Block::setPrevious(Block *previous)
{
	m_previous = previous;
}

// ----------------------------------------------------------------------

inline Block * Block::getNext()
{
	return m_next;
}

// ----------------------------------------------------------------------

inline Block const * Block::getNext() const
{
	return m_next;
}

// ----------------------------------------------------------------------

inline void Block::setNext(Block *next)
{
	DEBUG_FATAL(next && reinterpret_cast<int>(next) - reinterpret_cast<int>(this) < cms_blockSize, ("too small"));
	m_next = next;
}

// ----------------------------------------------------------------------

inline bool Block::isFree() const
{
	return m_free;
}

// ----------------------------------------------------------------------

inline void Block::setFree(bool free)
{
	m_free = free;
}

// ----------------------------------------------------------------------

inline int Block::getSize() const
{
	return reinterpret_cast<byte const *>(m_next) - reinterpret_cast<byte const *>(this);
}

// ======================================================================

#if DO_SCALAR

inline bool AllocatedBlock::isAllocatedAsArray() const
{
	return m_array;
}

// ----------------------------------------------------------------------

inline void AllocatedBlock::setAllocatedAsArray(bool array)
{
	m_array = array;
}

#endif

// ----------------------------------------------------------------------

#if DO_TRACK

inline bool AllocatedBlock::checkForLeaks() const
{
	return m_leakTest;
}

// ----------------------------------------------------------------------

inline void AllocatedBlock::setCheckForLeaks(bool checkForLeaks)
{
	m_leakTest = checkForLeaks;
}

// ----------------------------------------------------------------------

inline uint32 AllocatedBlock::getOwner(int index) const
{
	return m_owner[index];
}

// ----------------------------------------------------------------------

inline void AllocatedBlock::setOwner(int index, uint32 owner)
{
	m_owner[index] = owner;
}

// ----------------------------------------------------------------------

inline void AllocatedBlock::fillOwnerWithFreePattern()
{
	memset(m_owner, cms_freeFillPattern, sizeof(m_owner));
}

#endif

// ----------------------------------------------------------------------

#if DO_TRACK || DO_GUARDS

inline int AllocatedBlock::getRequestedSize() const
{
	return m_requestedSize;
}

// ----------------------------------------------------------------------

inline void AllocatedBlock::setRequestedSize(int requestedSize)
{
	m_requestedSize = requestedSize;
}

#endif

// ======================================================================

inline int MemoryManagerNamespace::min(int a, int b)
{
	return (a < b) ? a : b;
}

// ----------------------------------------------------------------------

inline int MemoryManagerNamespace::max(int a, int b)
{
	return (a > b) ? a : b;
}

// ----------------------------------------------------------------------

inline byte const * MemoryManagerNamespace::min(byte const * a, byte const * b)
{
	return (a < b) ? a : b;
}

// ----------------------------------------------------------------------

inline byte const * MemoryManagerNamespace::max(byte const * a, byte const * b)
{
	return (a > b) ? a : b;
}

// ----------------------------------------------------------------------
// this is here to get around linkage problems on win32

void MemoryManagerNamespace::outputDebugStringWrapper(char const * message)
{
	OutputDebugString(message);
}

// ----------------------------------------------------------------------

void  MemoryManager::setLimit(int megabytes, bool hardLimit, bool preallocate)
{
	DEBUG_FATAL(ms_limitSet, ("MemoryManager::setLimit may only be called once"));
	ms_limitSet = true;
	ms_limitMegabytes = megabytes;
	ms_hardLimit = hardLimit;
	DEBUG_FATAL(ms_hardLimit && ms_systemMemoryAllocatedMegabytes > ms_limitMegabytes, ("hard limit broken before being set %d/%d", ms_systemMemoryAllocatedMegabytes, ms_limitMegabytes));
	if (preallocate && ms_systemMemoryAllocatedMegabytes < ms_limitMegabytes)
		allocateSystemMemory(ms_limitMegabytes - ms_systemMemoryAllocatedMegabytes);
}

// ----------------------------------------------------------------------

int MemoryManager::getLimit()
{
	return ms_limitMegabytes;
}

// ----------------------------------------------------------------------

bool MemoryManager::isHardLimit()
{
	return ms_hardLimit;
}

// ----------------------------------------------------------------------
/**
 * Install the MemoryManager.
 * @param newMaxBytes  Maximum number of bytes in the memory pool
 * @param newMaxBlocks  Number of blocks (free or allocated)
 * @param commitAll     Commit all the specified ram to the application
 */

MemoryManager::MemoryManager()
{
	// any allocation will install the memory manager if it has yet to be, so we want to bail safely here
	if (ms_installed)
		return;

#if defined(PLATFORM_WIN32) && defined(_DEBUG)
	/*
   _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
   int crtHeapFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
   crtHeapFlags &= ~_CRTDBG_ALLOC_MEM_DF;
   crtHeapFlags &= ~_CRTDBG_CHECK_ALWAYS_DF;
   crtHeapFlags &= ~_CRTDBG_CHECK_CRT_DF;
   crtHeapFlags &= ~_CRTDBG_DELAY_FREE_MEM_DF;
   crtHeapFlags &= ~_CRTDBG_LEAK_CHECK_DF;
   _CrtSetDbgFlag(crtHeapFlags);
	*/
#endif

	OsMemory::install();
	DebugHelp::install();

#if !DISABLED

	// this buffer must remain local to this function, otherwise it will get clobbered.
	// the problem is this function gets called before the library gets properly initialized,
	// and then when the library gets initialized, it will clear the global memory, thus
	// wiping out our mutex.
	static char s_criticalSectionBuffer[sizeof(RecursiveMutex)];
	ms_criticalSection = new(s_criticalSectionBuffer) RecursiveMutex;

	ms_reportAllocations = true;
	ms_installed = true;
//	ms_logEachAlloc = false;

#endif
}

// ----------------------------------------------------------------------
/**
 * Remove the MemoryManager.
 */

MemoryManager::~MemoryManager()
{
#if DISABLED
	return;
#else

	DEBUG_FATAL(!ms_installed, ("not installed"));

	ms_criticalSection->enter();

		DEBUG_REPORT_LOG_PRINT(true, ("MM::remove %lu/%lu=bytes %d/%d=allocs\n", getCurrentNumberOfBytesAllocated(), getMaximumNumberOfBytesAllocated(), getCurrentNumberOfAllocations(), getMaximumNumberOfAllocations()));
		DEBUG_OUTPUT_CHANNEL("Foundation\\MemoryManager", ("MM::remove %lu/%lu=bytes %d/%d=allocs\n", getCurrentNumberOfBytesAllocated(), getMaximumNumberOfBytesAllocated(), getCurrentNumberOfAllocations(), getMaximumNumberOfAllocations()));

#if DO_TRACK
		if (!ConfigSharedFoundation::getDemoMode() && ms_allocations && ms_reportAllocations)
		{
			ms_criticalSection->leave();
			int const leaks = MemoryManagerNamespace::report(true);
			ms_criticalSection->enter();
			if (leaks)
				REPORT(true, Report::RF_log | Report::RF_dialog, ("%d memory leak%s", leaks, leaks > 1 ? "s" : ""));
		}
#endif

		// free all of the allocations from the OS
		while (ms_firstSystemAllocation)
		{
			SystemAllocation * systemAllocation = ms_firstSystemAllocation;
			ms_firstSystemAllocation = ms_firstSystemAllocation->getNext();
			systemAllocation->~SystemAllocation();
		}

	ms_criticalSection->leave();
	ms_installed = false;

#ifdef _DEBUG
	DebugMonitor::remove();
#endif

	ms_criticalSection->~RecursiveMutex();

#endif

	DebugHelp::remove();
	OsMemory::remove();
}

// ----------------------------------------------------------------------

int MemoryManagerNamespace::convertBytesToMegabytesForSystemAllocation(int systemAllocationSize)
{
	if (systemAllocationSize < cms_systemAllocationMinimumSize)
		systemAllocationSize = cms_systemAllocationMinimumSize;

	systemAllocationSize = (systemAllocationSize + (cms_systemAllocationRoundSize - 1)) & (~(cms_systemAllocationRoundSize - 1));

	return systemAllocationSize / (1024 * 1024);
}

// ----------------------------------------------------------------------

void MemoryManagerNamespace::allocateSystemMemory(int megabytes)
{
	if (ms_hardLimit && ms_systemMemoryAllocatedMegabytes + megabytes > ms_limitMegabytes)
	{
		megabytes = ms_limitMegabytes - ms_systemMemoryAllocatedMegabytes;
		if (megabytes <= 0)
			return;
	}

	// allocate the system memory
	size_t systemAllocationSize = static_cast<size_t>(megabytes) * 1024 * 1024;
	void * data = OsMemory::commit(0, systemAllocationSize);

	// failed to allocate virtual memory.  there is insufficient virtual memory or address space to satisfy systemAllocationSize.
	if (!data)
		return;

	// Construct our tracking information
	SystemAllocation * systemAllocation = new(data) SystemAllocation(systemAllocationSize);
	++ms_numberOfSystemAllocations;
	ms_systemMemoryAllocatedMegabytes += megabytes;

	// insert the memory into the sorted linked list of system allocations
	SystemAllocation * back = nullptr;
	SystemAllocation * front = ms_firstSystemAllocation;
	for ( ; front && front->getFirstMemoryBlock() < systemAllocation->getFirstMemoryBlock(); back = front, front = front->getNext())
		{}
	if (back)
		back->setNext(systemAllocation);
	else
		ms_firstSystemAllocation = systemAllocation;
	systemAllocation->setNext(front);
}

// ----------------------------------------------------------------------

unsigned long MemoryManager::getCurrentNumberOfBytesAllocated(const int processId)
{	
	UNREF(processId);
	return ms_currentBytesAllocated;
}

// ----------------------------------------------------------------------

unsigned long MemoryManager::getCurrentNumberOfBytesAllocatedNoLeakTest()
{
#if DO_TRACK
	return ms_currentBytesAllocatedNoLeakTest;
#else
	return 0;
#endif
}

// ----------------------------------------------------------------------

unsigned long MemoryManager::getMaximumNumberOfBytesAllocated()
{
	return ms_maxBytesAllocated;
}

// ----------------------------------------------------------------------

int MemoryManager::getSystemMemoryAllocatedMegabytes()
{
	return ms_systemMemoryAllocatedMegabytes;
}

// ----------------------------------------------------------------------

int MemoryManager::getCurrentNumberOfAllocations()
{
	return ms_allocations;
}

// ----------------------------------------------------------------------

int MemoryManager::getMaximumNumberOfAllocations()
{
	return ms_maxAllocations;
}

// ----------------------------------------------------------------------

void MemoryManager::setReportAllocations(bool reportAllocations)
{
	ms_reportAllocations = reportAllocations;
}

// ----------------------------------------------------------------------

void MemoryManager::registerDebugFlags()
{
#ifdef _DEBUG
	DebugFlags::registerFlag(ms_debugReportFlag,                       "SharedMemoryManager", "reportMemory",                     debugReport);
	DebugFlags::registerFlag(ms_debugReportMapFlag,                    "SharedMemoryManager", "reportMemoryMap",                  debugReportMap);
	DebugFlags::registerFlag(ms_debugReportAllocations,                "SharedMemoryManager", "reportMemoryAllocations");
	DebugFlags::registerFlag(ms_debugReportLogMemoryAllocFreePointers, "SharedMemoryManager", "reportLogMemoryAllocFreePointers");
	DebugFlags::registerFlag(ms_debugLogAllocations,                   "SharedMemoryManager", "reportLogAllocations");
	DebugFlags::registerFlag(ms_debugLogAllocationsNextFrame,          "SharedMemoryManager", "reportLogAllocationsNextFrame",    logAllocationsNextFrame);
	DebugFlags::registerFlag(ms_debugVerifyGuardPatterns,              "SharedMemoryManager", "verifyGuardPatterns");
	DebugFlags::registerFlag(ms_debugVerifyFreePatterns,               "SharedMemoryManager", "verifyFreePatterns");
	DebugFlags::registerFlag(ms_debugProfileAllocate,                  "SharedMemoryManager", "profileAllocate");
#endif
}

// ----------------------------------------------------------------------
/**
 * Dump debugging information about the MemoryManager.
 */

void MemoryManager::debugReport()
{
#if !DISABLED
	DEBUG_FATAL(!ms_installed, ("not installed"));
	DEBUG_REPORT_PRINT(ms_limitSet, ("MM: %9dmb (%s limit)\n", ms_limitMegabytes, ms_hardLimit ? "hard" : "soft"));
	DEBUG_REPORT_PRINT(true,        ("MM: %9d/%9d/%9d  cur/max/tot allocs\n", ms_allocations, ms_maxAllocations, ms_allocateCalls));
	DEBUG_REPORT_PRINT(true,        ("MM: %9lu/%9lu/%9lu  cur/max/tot bytes\n",  ms_currentBytesAllocated, ms_maxBytesAllocated, ms_allocateBytesTotal));
#endif
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void MemoryManagerNamespace::logAllocationsNextFrame()
{
	if (!ms_debugLogAllocationsNextFrameStarted)
	{
		REPORT_LOG_PRINT(true, ("MemoryManager::logAllocationsNextFrame start\n"));
		ms_debugLogAllocations = true;
		ms_debugLogAllocationsNextFrameStarted = true;
	}
	else
	{
		REPORT_LOG_PRINT(true, ("MemoryManager::logAllocationsNextFrame end\n"));
		ms_debugLogAllocations = false;
		ms_debugLogAllocationsNextFrameStarted = false;
		ms_debugLogAllocationsNextFrame = false;
	}
}
#endif

// ----------------------------------------------------------------------

void MemoryManagerNamespace::emitCharacters(char * & buffer, int blockSize, int & carryOverFree, int & carryOverUsed, int newFree, int newUsed, char const * const bufferOverrunAddress)
{
	while (carryOverFree + carryOverUsed + newFree + newUsed >= blockSize)
	{
		int const absorbNew = blockSize - (carryOverUsed + carryOverFree);
		if (newFree)
		{
			carryOverFree += absorbNew;
			newFree -= absorbNew;
		}
		else
		{
			carryOverUsed += absorbNew;
			newUsed -= absorbNew;
		}

		if (carryOverFree == blockSize)
			*buffer = '_';
		else
			if (carryOverUsed == blockSize)
				*buffer = '*';
			else
			{
				int const letter = (carryOverUsed * 26) / blockSize;
				*buffer = static_cast<char>('A' + letter);
			}
		++buffer;

		FATAL(buffer == bufferOverrunAddress, ("Buffer overrun! 0x%0X", buffer));

		carryOverFree = 0;
		carryOverUsed = 0;
	}

	carryOverFree += newFree;
	carryOverUsed += newUsed;
}

// ----------------------------------------------------------------------
/**
 * Dump an image of the memory state to the report system.
 */

void MemoryManager::debugReportMap()
{
	for (SystemAllocation * systemAllocation = ms_firstSystemAllocation; systemAllocation; systemAllocation = systemAllocation->getNext())
	{
		char buffer[128];
		int const numberOfCharactersPerRow = 64;

		snprintf(buffer, sizeof(buffer)-1, "%8p %3dmb : ", systemAllocation, systemAllocation->getSize() / (1024 * 1024));
		buffer[sizeof(buffer)-1] = '\0';

		int const headerLength = strlen(buffer);
		int const characterSize = systemAllocation->getSize() / numberOfCharactersPerRow;

		int carryOverUsed = 0;
		int carryOverFree = 0;
		char * b = buffer + headerLength;
		char const * const bufferOverrunAddress = buffer + sizeof(buffer);
		for (Block * block = systemAllocation->getFirstMemoryBlock()->getNext(); block != systemAllocation->getLastMemoryBlock(); block = block->getNext())
		{
			if (block->isFree())
				emitCharacters(b, characterSize, carryOverFree, carryOverUsed, block->getSize(), 0, bufferOverrunAddress);
			else
				emitCharacters(b, characterSize, carryOverFree, carryOverUsed, 0, block->getSize(), bufferOverrunAddress);
		}

		if (carryOverFree || carryOverUsed)
			emitCharacters(b, carryOverFree + carryOverUsed, carryOverFree, carryOverUsed, 0, 0, bufferOverrunAddress);

		b[0] = '\0';

		DEBUG_REPORT_PRINT(true, ("%s\n", buffer));
	}
}

// ----------------------------------------------------------------------
/**
 * Add a block to the free memory chain list.
 *
 * @param block  Block of memory that is being
 */

void MemoryManagerNamespace::addToFreeList(Block * block)
{
	NOT_NULL(block);

	// @todo consider making the ternary search tree balanced

	FreeBlock *   freeBlock     = static_cast<FreeBlock *>(block);
	int const     freeBlockSize = freeBlock->getSize();
	FreeBlock *   parent        = nullptr;
	FreeBlock * * next          = &ms_firstFreeBlock;
	FreeBlock *   same          = 0;

	for (FreeBlock * current = ms_firstFreeBlock; current; parent = current, current = *next)
	{
		int const currentBlockSize = current->getSize();
		if (freeBlockSize < currentBlockSize)
			next = &current->m_smallerFreeBlock;
		else
			if (freeBlockSize > currentBlockSize)
				next = &current->m_largerFreeBlock;
			else
			{
				// quick out for same size blocks
				same = current;
				break;
			}
	}

	if (same)
	{

		freeBlock->m_smallerFreeBlock = same->m_smallerFreeBlock;
		same->m_smallerFreeBlock = nullptr;
		if (freeBlock->m_smallerFreeBlock)
			freeBlock->m_smallerFreeBlock->m_parentFreeBlock = freeBlock;

		freeBlock->m_sameFreeBlock = same;
		same->m_parentFreeBlock = freeBlock;

		freeBlock->m_largerFreeBlock = same->m_largerFreeBlock;
		same->m_largerFreeBlock = nullptr;
		if (freeBlock->m_largerFreeBlock)
			freeBlock->m_largerFreeBlock->m_parentFreeBlock = freeBlock;

		*next = freeBlock;
		freeBlock->m_parentFreeBlock  = parent;
	}
	else
	{
		*next = freeBlock;
		freeBlock->m_smallerFreeBlock = nullptr;
		freeBlock->m_sameFreeBlock    = nullptr;
		freeBlock->m_largerFreeBlock  = nullptr;
		freeBlock->m_parentFreeBlock  = parent;
	}

	if (++ms_freeBlocks > ms_maxFreeBlocks)
		ms_maxFreeBlocks = ms_freeBlocks;

#ifdef _DEBUG
	int treeHeight = 0;
	while (freeBlock)
	{
		++treeHeight;
		freeBlock = freeBlock->m_parentFreeBlock;
	}
	if (treeHeight > ms_maxTreeHeight)
		ms_maxTreeHeight = treeHeight;
#endif
}

// ----------------------------------------------------------------------
/**
 * Remove a block from the free memory chain list.
 *
 * @param block  Block of memory that is being allocated
 */

void MemoryManagerNamespace::removeFromFreeList(FreeBlock * block)
{
	NOT_NULL(block);

	// find the pointer that points to block
	FreeBlock * * parentPointer = nullptr;
	FreeBlock * parent = block->m_parentFreeBlock;
	if (parent)
	{
		if (parent->m_smallerFreeBlock == block)
			parentPointer = &parent->m_smallerFreeBlock;
		else
			if (parent->m_sameFreeBlock == block)
				parentPointer = &parent->m_sameFreeBlock;
			else
			{
				DEBUG_FATAL(parent->m_largerFreeBlock != block, ("parent doesn't know about child"));
				parentPointer = &parent->m_largerFreeBlock;
			}
	}
	else
	{
		DEBUG_FATAL(ms_firstFreeBlock != block, ("no parent but not first"));
		parentPointer = &ms_firstFreeBlock;
	}

	// now we need to replace our node with one from underneath us
	FreeBlock * same = block->m_sameFreeBlock;
	if (same)
	{
		// making use of the same size block is pretty easy and doesn't disturb tree balance so its our prefered choice
		same->m_smallerFreeBlock = block->m_smallerFreeBlock;
		if (same->m_smallerFreeBlock)
			same->m_smallerFreeBlock->m_parentFreeBlock = same;

		same->m_largerFreeBlock = block->m_largerFreeBlock;
		if (same->m_largerFreeBlock)
			same->m_largerFreeBlock->m_parentFreeBlock = same;

		same->m_parentFreeBlock = parent;

		*parentPointer = same;
	}
	else
	{
		if (block->m_smallerFreeBlock)
		{
			if (block->m_largerFreeBlock)
			{
				// this is the worst case.  this free block has smaller and larger children, but not any same sized children
				// we're going to take the smallest block off the larger list and use that to replace the current node
				FreeBlock * back = nullptr;
				FreeBlock * replacement = block->m_largerFreeBlock;
				while (replacement->m_smallerFreeBlock)
				{
					back = replacement;
					replacement = replacement->m_smallerFreeBlock;
				}

				// disconnect the link between its parent and out replacement block
				if (back)
				{
					back->m_smallerFreeBlock = replacement->m_largerFreeBlock;
					if (back->m_smallerFreeBlock)
						back->m_smallerFreeBlock->m_parentFreeBlock = back;

					replacement->m_largerFreeBlock = block->m_largerFreeBlock;
					replacement->m_largerFreeBlock->m_parentFreeBlock = replacement;
				}

				// put the replacement block in its new place
				replacement->m_smallerFreeBlock = block->m_smallerFreeBlock;
				replacement->m_smallerFreeBlock->m_parentFreeBlock = replacement;

				replacement->m_parentFreeBlock  = parent;

				*parentPointer = replacement;
			}
			else
			{
				// this block only has one child, which is smaller.  an easy case.
				block->m_smallerFreeBlock->m_parentFreeBlock = parent;
				*parentPointer = block->m_smallerFreeBlock;
			}
		}
		else
		{
			if (block->m_largerFreeBlock)
			{
				// this block only has one child, which is larger. an easy case.
				block->m_largerFreeBlock->m_parentFreeBlock = parent;
				*parentPointer = block->m_largerFreeBlock;
			}
			else
			{
				// this block has no children
				*parentPointer = nullptr;
			}
		}
	}

	// remove all the pointers the block may have had
	block->m_smallerFreeBlock = nullptr;
	block->m_sameFreeBlock    = nullptr;
	block->m_largerFreeBlock  = nullptr;
	block->m_parentFreeBlock  = nullptr;

	--ms_freeBlocks;
}

// ----------------------------------------------------------------------

FreeBlock *MemoryManagerNamespace::searchFreeList(int blockSize)
{
	FreeBlock * result = nullptr;
	FreeBlock * current = ms_firstFreeBlock;
	while (current)
	{
		int const currentBlockSize = current->getSize();

		if (currentBlockSize < blockSize)
		{
			// this block is too small so check blocks larger than this
			current = current->m_largerFreeBlock;
		}
		else
			if (currentBlockSize > blockSize)
			{
				// this block is too large but will be the best fit so far.  continue checking smaller blocks.
				result = current;
				current = current->m_smallerFreeBlock;
			}
			else
			{
				// this block is a perfect fit, return it immediately.
				return current;
			}
	}

	return result;
}

// ----------------------------------------------------------------------
/**
 * Dynamically allocate memory.
 *
 * Users should not call this routine directly.  It should only be called
 * by operator new.
 *
 * @param size  Number of bytes to allocate
 * @param owner  Line number on which the allocation was made
 * @param array  True if the array form of operator new was used, false if the scalar form was used
 */

void * MemoryManager::allocate(size_t size, uint32 owner, bool array, bool leakTest)
{
	if (!ms_installed)
		new(ms_memoryManagerBuffer) MemoryManager;

	UNREF(owner);
	UNREF(array);
	UNREF(leakTest);

#if DISABLED
	return operator new(size);
#else

	DEBUG_FATAL(!ms_installed, ("not installed"));

#ifdef _DEBUG
	if (ms_debugProfileAllocate)
		PROFILER_BLOCK_ENTER(ms_allocateProfilerBlock);

	if (ms_debugVerifyGuardPatterns || ms_debugVerifyFreePatterns)
		verify(ms_debugVerifyGuardPatterns, ms_debugVerifyFreePatterns);

	if (ms_debugReportAllocations || ms_debugLogAllocations)
	{
		char libName[256];
		char fileName[256];
		int line;
		if (ms_allowNameLookup && DebugHelp::lookupAddress(owner, libName, fileName, sizeof(fileName), line))
		{
			DEBUG_REPORT(true, (ms_debugReportAllocations ? Report::RF_print : 0) | (ms_debugLogAllocations ? Report::RF_log : 0), ("%s(%d): alloc %d=bytes %d=array\n", fileName, line, size, static_cast<int>(array)));
		}
		else
		{
			DEBUG_REPORT(true, (ms_debugReportAllocations ? Report::RF_print : 0) | (ms_debugLogAllocations ? Report::RF_log : 0), ("%08x: alloc %d=bytes %d=array\n", static_cast<int>(owner), size, static_cast<int>(array)));
		}
	}
#endif

	ms_criticalSection->enter();

		// get the size of the allocation
		int allocSize = (cms_allocatedBlockSize + cms_guardBandSize + (size ? static_cast<int>(size) : 1) + cms_guardBandSize + 15) & ~15;

		FreeBlock * bestFreeBlock = nullptr;
		for (int tries = 0; !bestFreeBlock && tries < 2; ++tries)
		{
			bestFreeBlock = searchFreeList(allocSize);

			// if the memory allocation failed, try to get some more memory
			if (!bestFreeBlock)
				allocateSystemMemory(convertBytesToMegabytesForSystemAllocation(cms_blockSize + cms_blockSize + allocSize + cms_blockSize));
		}

		// make sure memory was available
		if (!bestFreeBlock)
		{
			if (ConfigSharedFoundation::getMemoryManagerReportOnOutOfMemory())
			{
				// avoid deadlock from FATAL calling free
				(*LogMessage)("Out of memory, dumping current allocations:\n");
				IGNORE_RETURN(MemoryManagerNamespace::report(false));
			}

			ms_criticalSection->leave();
			FATAL(true, ("failed allocation attempt for %d (%d actual)", allocSize, size));
		}

		removeFromFreeList(bestFreeBlock);

		// setup the allocation record
		bestFreeBlock->setFree(false);

		// check to see if we should subdivide this block
		if (bestFreeBlock->getSize() > (allocSize + cms_allocatedBlockSize + cms_guardBandSize + 1 + cms_guardBandSize))
		{
			Block *block = reinterpret_cast<Block *>(reinterpret_cast<byte *>(bestFreeBlock) + allocSize);
			block->setPrevious(bestFreeBlock);
			block->setNext(bestFreeBlock->getNext());
			block->setFree(true);

			bestFreeBlock->getNext()->setPrevious(block);
			bestFreeBlock->setNext(block);

			addToFreeList(block);
		}

		AllocatedBlock * best = reinterpret_cast<AllocatedBlock *>(bestFreeBlock);

#if DO_SCALAR
		best->setAllocatedAsArray(array);
#endif

#if DO_TRACK
		best->setCheckForLeaks(leakTest);
		best->setOwner(0, owner);
#endif
#if DO_TRACK > 1
		{
			enum { OFFSET = 3 };
			uint32 owners[DO_TRACK + OFFSET];
			DebugHelp::getCallStack(owners, DO_TRACK + OFFSET);

			for (int i = 1; i < DO_TRACK; ++i)
			{
 				best->setOwner(i, owners[i + OFFSET]);

#ifdef _DEBUG
				if (ms_debugReportAllocations || ms_debugLogAllocations)
				{
					char libName[256];
					char fileName[256];
					int line;
					if (ms_allowNameLookup && DebugHelp::lookupAddress(owners[i + OFFSET], libName, fileName, sizeof(fileName), line))
					{
						DEBUG_REPORT(true, (ms_debugReportAllocations ? Report::RF_print : 0) | (ms_debugLogAllocations ? Report::RF_log : 0), ("  %s(%d): caller %d\n", fileName, line, i));
					}
					else
					{
						DEBUG_REPORT(true, (ms_debugReportAllocations ? Report::RF_print : 0) | (ms_debugLogAllocations ? Report::RF_log : 0), ("  %08x: caller %d\n", static_cast<int>(owners[i + OFFSET]), i));
					}
				}
#endif
			}
		}
#endif
#if DO_TRACK || DO_GUARDS
		best->setRequestedSize(static_cast<int>(size));
		ms_currentBytesRequested += size;
		DEBUG_FATAL(best->getRequestedSize() != static_cast<int>(size), ("allocated more memory at once than the memory manager supports (%d)", (1 << Block::cms_requestedSizeBits) - 1));
#endif

		// update the size of the allocation because our block may not have been large enough to subdivide
		allocSize = best->getSize();

		// update the number of bytes allocated
		++ms_allocateCalls;
		ms_allocateBytesTotal += allocSize;
		ms_currentBytesAllocated += allocSize;

#if DO_TRACK
		if (!leakTest)
			ms_currentBytesAllocatedNoLeakTest += allocSize;
#endif

		if (++ms_allocations > ms_maxAllocations)
			ms_maxAllocations = ms_allocations;
		if (ms_currentBytesAllocated > ms_maxBytesAllocated)
			ms_maxBytesAllocated = ms_currentBytesAllocated;

		// get another pointer to the memory we allocated so we can tinker with it
		byte * memory = reinterpret_cast<byte *>(best) + cms_allocatedBlockSize + cms_guardBandSize;

#if DO_GUARDS
		// fill the prefix guard band
		memset(memory - cms_guardBandSize, cms_guardFillPattern, cms_guardBandSize);
#endif

		// fill the user memory with the initialize pattern
#if DO_INITIALIZE_FILLS
		memset(memory, cms_initializeFillPattern, size);
#endif

#if DO_GUARDS
		// fill the suffix guard band
		memset(memory+size, cms_guardFillPattern, cms_guardBandSize);
#endif

	ms_criticalSection->leave();

	DEBUG_REPORT_LOG_PRINT(ms_debugReportLogMemoryAllocFreePointers, ("MM::alloc %08x\n", reinterpret_cast<int>(memory)));

#ifdef _DEBUG
	if (ms_debugProfileAllocate)
		PROFILER_BLOCK_LEAVE(ms_allocateProfilerBlock);
#endif

//	DEBUG_REPORT_LOG(ms_logEachAlloc, ("MemoryManager::allocate() requested_size=%d, alloc_size=%d, ptr=%p\n", size, allocSize, memory));

	return memory;
#endif
}

// ----------------------------------------------------------------------

void *MemoryManager::reallocate(void *userPointer, size_t newSize)
{
	AllocatedBlock *allocatedBlock = 0;
	bool array = false;
	int oldSize = 0;

	if (userPointer)
	{
		allocatedBlock = reinterpret_cast<AllocatedBlock *>(reinterpret_cast<byte *>(userPointer) - (cms_allocatedBlockSize + cms_guardBandSize));
		#if DO_SCALAR
		array = allocatedBlock->isAllocatedAsArray();
		#endif
		if (!newSize)
		{
			MemoryManager::free(userPointer, array);

//			DEBUG_REPORT_LOG(ms_logEachAlloc, ("MemoryManager::reallocate() new_requested_size=%d, org ptr=%p, new ptr=nullptr\n", newSize, userPointer));

			return 0;
		}
		#if DO_TRACK || DO_GUARDS
		oldSize = allocatedBlock->getRequestedSize();
		#else
		oldSize = allocatedBlock->getSize()-cms_allocatedBlockSize;
		#endif
	}

	if (newSize <= static_cast<size_t>(oldSize))
	{
//		DEBUG_REPORT_LOG(ms_logEachAlloc, ("MemoryManager::reallocate() new_requested_size=%d, org ptr=%p, new ptr=%p\n", newSize, userPointer, userPointer));

		return userPointer;
	}

#if DO_TRACK
	uint32 owner = allocatedBlock->getOwner(0);
	bool leakTest = allocatedBlock->checkForLeaks();
#else
	uint32 owner = 0;
	bool leakTest = false;
#endif

	void *newPointer = allocate(newSize, owner, array, leakTest);
	if (oldSize)
		memcpy(newPointer, userPointer, oldSize);
	if (userPointer)
		MemoryManager::free(userPointer, array);

//	DEBUG_REPORT_LOG(ms_logEachAlloc, ("MemoryManager::reallocate() new_requested_size=%d, org ptr=%p, new ptr=%p\n", newSize, userPointer, newPointer));

	return newPointer;
}

// ----------------------------------------------------------------------
/**
 * Free dynamically allocated memory.
 *
 * Users should not call this routine directly.  It should only be called
 * by operator delete.
 *
 * This routine should not be called with the nullptr pointer.
 *
 * @param userPointer  Pointer to the memory
 * @param array  True if the array form of operator new was used, false if the scalar form was used
 */

void MemoryManager::free(void * userPointer, bool array)
{
#if DISABLED
	UNREF(array);
	operator delete(userPointer);
	return;
#else

	DEBUG_FATAL(!ms_installed, ("not installed"));
	NOT_NULL(userPointer);

#ifdef _DEBUG
	if (ms_debugVerifyGuardPatterns || ms_debugVerifyFreePatterns)
		verify(ms_debugVerifyGuardPatterns, ms_debugVerifyFreePatterns);
#endif

	DEBUG_REPORT_LOG_PRINT(ms_debugReportLogMemoryAllocFreePointers, ("MM::free %08x\n", reinterpret_cast<int>(userPointer)));

	UNREF(array);

	ms_criticalSection->enter();

		AllocatedBlock * allocatedBlock = reinterpret_cast<AllocatedBlock *>(reinterpret_cast<byte *>(userPointer) - (cms_allocatedBlockSize + cms_guardBandSize));

#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG
		DEBUG_FATAL(allocatedBlock->getNext()->getPrevious() != allocatedBlock,                                      ("Bad free (1) %p", userPointer));
		DEBUG_FATAL(allocatedBlock->getPrevious()->getNext() != allocatedBlock,                                      ("Bad free (2) %p", userPointer));
		DEBUG_FATAL(allocatedBlock->isFree(),                                                                        ("Freeing already free block %p", userPointer));
#endif

#if DO_TRACK
		ms_bytesFreedPerFrame += allocatedBlock->getRequestedSize();
#endif

#if DO_GUARDS
		{
			// verify the guard bands
			byte * guard   = reinterpret_cast<byte *>(allocatedBlock) + cms_allocatedBlockSize;
			bool   corrupt = false;

			// check the prefix guard band
			for (int i = 0; i < cms_guardBandSize; ++i, ++guard)
				if (*guard != cms_guardFillPattern)
				{
					corrupt = true;
					DEBUG_REPORT_LOG_PRINT(true, ("MemoryManager::free corrupted guard prefix at position %3d = %02x\n", i - cms_guardBandSize, static_cast<int>(*guard)));
					DEBUG_OUTPUT_CHANNEL("Foundation\\MemoryManager", ("MemoryManager::free corrupted guard prefix at position %3d = %02x\n", i - cms_guardBandSize, static_cast<int>(*guard)));
				}

			// advance past the user memory
			guard += allocatedBlock->getRequestedSize();

			// check the suffix guard band
			for (int j = 0; j < cms_guardBandSize; ++j, ++guard)
				if (*guard != cms_guardFillPattern)
				{
					corrupt = true;
					DEBUG_REPORT_LOG_PRINT(true, ("MemoryManager::free corrupted guard suffix at position %3d = %02x\n", j, static_cast<int>(*guard)));
					DEBUG_OUTPUT_CHANNEL("Foundation\\MemoryManager", ("MemoryManager::free corrupted guard suffix at position %3d = %02x\n", j, static_cast<int>(*guard)));
				}

			if (corrupt)
			{
				MemoryManagerNamespace::report(allocatedBlock, false);
				ms_criticalSection->leave();
				DEBUG_FATAL(true, ("corrupted guard pattern"));
				return; //lint !e527 // Unreachable
			}
		}
#endif

#if DO_SCALAR
		if (allocatedBlock->isAllocatedAsArray() != array) //lint !e731 // Info -- Boolean argument to equal/not equal
		{
			MemoryManagerNamespace::report(allocatedBlock, false);
			#ifdef _DEBUG
			bool const blockArray = allocatedBlock->isAllocatedAsArray();
			ms_criticalSection->leave();
			FATAL(true, ("allocated %s deleted %s", blockArray ? "array" : "scalar", array ? "array" : "scalar"));   //lint !e731 // Info -- Boolean argument to equal/not equal
			#endif
		}
#endif

		int const memorySize = allocatedBlock->getSize();

		// wipe the user memory
#if DO_FREE_FILLS
		imemset(reinterpret_cast<byte *>(allocatedBlock) + cms_allocatedBlockSize, cms_freeFillPattern, memorySize - cms_allocatedBlockSize);
#endif

		allocatedBlock->setFree(true);
#if DO_SCALAR
		allocatedBlock->setAllocatedAsArray(false);
#endif

		// clear out the block records
#if DO_TRACK
		allocatedBlock->fillOwnerWithFreePattern();
#endif

#if DO_TRACK || DO_GUARDS
		ms_currentBytesRequested -= allocatedBlock->getRequestedSize();
		allocatedBlock->setRequestedSize(0);
		DEBUG_FATAL(allocatedBlock->getRequestedSize() != 0, ("bad size"));
#endif

		// update the number of bytes allocated
		++ms_freeCalls;
		--ms_allocations;

		DEBUG_FATAL((ms_currentBytesAllocated < static_cast<unsigned long>(memorySize)), ("currentBytesAllocated underflow"));
		ms_currentBytesAllocated -= memorySize;
#if DO_TRACK
		if (!allocatedBlock->checkForLeaks())
			ms_currentBytesAllocatedNoLeakTest -= memorySize;
#endif
		DEBUG_FATAL(ms_allocations < 0, ("allocations underflow"));

//		DEBUG_REPORT_LOG(ms_logEachAlloc, ("MemoryManager::free() requested_size=%d, alloc_size=%d, userPointer=%p, allocatedBlock=%p\n", requestedSize, memorySize, userPointer, allocatedBlock));

		Block *block = allocatedBlock;

		// recombine with the previous block
		if (block->getPrevious()->isFree())
		{
			FreeBlock * const previous = static_cast<FreeBlock *>(block->getPrevious());
			removeFromFreeList(previous);
			previous->setNext(block->getNext());
			block->getNext()->setPrevious(previous);

#if DO_FREE_FILLS
			memset(block, cms_freeFillPattern, cms_freeBlockSize);
#endif
			block = previous;
		}

		// recombine with the following block
		if (block->getNext()->isFree())
		{
			FreeBlock * const next = static_cast<FreeBlock *>(block->getNext());
			removeFromFreeList(next);
			block->setNext(next->getNext());
			next->getNext()->setPrevious(block);

#if DO_FREE_FILLS
			memset(next, cms_freeFillPattern, cms_freeBlockSize);
#endif
		}

		addToFreeList(block);

	ms_criticalSection->leave();
#endif
}

// ----------------------------------------------------------------------

int MemoryManagerNamespace::quickGuessIsValidPointerFromHeap(void const * userPointer)
{
	int result = 0;
	AllocatedBlock const * allocatedBlock = reinterpret_cast<AllocatedBlock const *>(reinterpret_cast<byte const *>(userPointer) - (cms_allocatedBlockSize + cms_guardBandSize));

	ms_criticalSection->enter();
		if (allocatedBlock->getNext()->getPrevious() != allocatedBlock) result =  1;
		if (allocatedBlock->getPrevious()->getNext() != allocatedBlock) result =  2;
		if (allocatedBlock->isFree()) result =  3;
	ms_criticalSection->leave();

	return result;
}

// ----------------------------------------------------------------------

int MemoryManagerNamespace::authoritativeIsValidPointerFromHeap(void const * userPointer)
{
	ms_criticalSection->enter();

		int result = quickGuessIsValidPointerFromHeap(userPointer);
		if (result != 0)
			return result;

		result = 16;
		for (SystemAllocation * systemAllocation = ms_firstSystemAllocation; systemAllocation; systemAllocation = systemAllocation->getNext())
			for (Block * block = systemAllocation->getFirstMemoryBlock()->getNext(); result != 0 && block != systemAllocation->getLastMemoryBlock(); block = block->getNext())
				if (!block->isFree())
				{
					byte const * memory = reinterpret_cast<byte const *>(block) + cms_allocatedBlockSize + cms_guardBandSize;
					if (memory == userPointer)
						result = 0;
				}

	ms_criticalSection->leave();

	return result;
}

// ----------------------------------------------------------------------
/**
 * Take ownership for a block of memory.
 *
 * @param userPointer  Memory to take ownership of
 */

void MemoryManager::own(void * userPointer)
{
	UNREF(userPointer);

#if DISABLED || !DO_TRACK
	return;
#else

	DEBUG_FATAL(!ms_installed, ("not installed"));

	if (!userPointer)
		return;

	ms_criticalSection->enter();

		// fetch the block pointer
		AllocatedBlock * block = reinterpret_cast<AllocatedBlock *>(reinterpret_cast<byte *>(userPointer) - (cms_allocatedBlockSize + cms_guardBandSize));
		DEBUG_FATAL(block->isFree(), ("cannot own a free block"));

		// update the owners
		{
			enum { OFFSET = 2 };
			uint32 owners[DO_TRACK + OFFSET];
			DebugHelp::getCallStack(owners, DO_TRACK + OFFSET);

			for (int i = 0; i < DO_TRACK; ++i)
 				block->setOwner(i, owners[i + OFFSET]);
		}

	ms_criticalSection->leave();
#endif
}

// ----------------------------------------------------------------------

void MemoryManager::verify(bool guardPatterns, bool freePatterns)
{
	UNREF(guardPatterns);
	UNREF(freePatterns);

#if DO_FREE_FILLS || DO_GUARDS

	ms_criticalSection->enter();

		// search for the memory pointer
		for (SystemAllocation * systemAllocation = ms_firstSystemAllocation; systemAllocation; systemAllocation = systemAllocation->getNext())
			for (Block * block = systemAllocation->getFirstMemoryBlock()->getNext(); block != systemAllocation->getLastMemoryBlock(); block = block->getNext())
				if (block->isFree())
				{
#if DO_FREE_FILLS
					if (freePatterns)
					{
						bool   corrupt = false;
						byte * memory  = reinterpret_cast<byte *>(block) + cms_freeBlockSize;
						int   freeSize = block->getSize() - cms_freeBlockSize;

						for (int i = 0; i < freeSize; ++i, ++memory)
							if (*memory != cms_freeFillPattern)
							{
								corrupt = true;
								DEBUG_REPORT_LOG_PRINT(true, ("corrupted free pattern at position %3d [membase=0x%x, memaddr=0x%x] = %02x\n", i, reinterpret_cast<unsigned int>(reinterpret_cast<byte *>(block) + cms_freeBlockSize), reinterpret_cast<unsigned int>(reinterpret_cast<byte *>(block) + cms_freeBlockSize + i), static_cast<int>(*memory)));
								DEBUG_OUTPUT_CHANNEL("Foundation\\MemoryManager", ("corrupted free pattern at position %3d = %02x\n", i, static_cast<int>(*memory)));
							}

						if (corrupt)
						{
							ms_criticalSection->leave();
							DEBUG_FATAL(true, ("corrupted free pattern"));
						}
					}
#endif
				}
				else
					if (guardPatterns)
					{
#if DO_GUARDS
						// verify the guard bands
						byte * guard   = reinterpret_cast<byte *>(block) + cms_allocatedBlockSize;
						bool   corrupt = false;

						// check the prefix guard band
						for (int i = 0; i < cms_guardBandSize; ++i, ++guard)
							if (*guard != cms_guardFillPattern)
							{
								corrupt = true;
								DEBUG_REPORT_LOG_PRINT(true, ("corrupted guard prefix at position %3d = %02x\n", i - cms_guardBandSize, static_cast<int>(*guard)));
								DEBUG_OUTPUT_CHANNEL("Foundation\\MemoryManager", ("corrupted guard prefix at position %3d = %02x\n", i - cms_guardBandSize, static_cast<int>(*guard)));
							}

						// advance past the user memory
						guard += static_cast<AllocatedBlock *>(block)->getRequestedSize();

						// check the suffix guard band
						for (int j = 0; j < cms_guardBandSize; ++j, ++guard)
							if (*guard != cms_guardFillPattern)
							{
								corrupt = true;
								DEBUG_REPORT_LOG_PRINT(true, ("corrupted guard suffix at position %3d = %02x\n", j, static_cast<int>(*guard)));
								DEBUG_OUTPUT_CHANNEL("Foundation\\MemoryManager", ("corrupted guard suffix at position %3d = %02x\n", j, static_cast<int>(*guard)));
							}

						if (corrupt)
						{
							MemoryManagerNamespace::report(static_cast<AllocatedBlock *>(block), false);
							ms_criticalSection->leave();
							DEBUG_FATAL(true, ("corrupted guard pattern"));
						}
#endif
					}

	ms_criticalSection->leave();

#endif
}

// ----------------------------------------------------------------------
/**
 * @internal
 */

void MemoryManagerNamespace::report(AllocatedBlock const * block, bool leak)
{
#if DO_TRACK
	uint32 const owner = block->getOwner(0);
#else
	uint32 const owner = 0;
#endif
#if DO_TRACK || DO_GUARDS
	int const requestedSize = block->getRequestedSize();;
#else
	int const requestedSize = 0;
#endif

	char      buffer[256];
	char      libName[256];
	char      fileName[256];
	int       line = 0;
	int const memory = reinterpret_cast<int>(reinterpret_cast<byte const *>(block) + cms_allocatedBlockSize + cms_guardBandSize);

	if (ms_allowNameLookup && DebugHelp::lookupAddress(owner, libName, fileName, sizeof(fileName), line))
	{
		sprintf(buffer, "%s(%d) : %08X memory %s, %d bytes\n", fileName, line, memory, leak ? "leak" : "allocation", static_cast<int>(requestedSize));
	}
	else
	{
		sprintf(buffer, "unknown(0x%08X) : %08X memory %s, %d bytes\n", static_cast<unsigned int>(owner), memory, leak ? "leak" : "allocation", static_cast<int>(requestedSize));
	}

	(*LogMessage)(buffer);

#if DO_TRACK > 1
	{
		for (int i = 1; i < DO_TRACK; ++i)
			if (block->getOwner(i))
			{
				if (ms_allowNameLookup && DebugHelp::lookupAddress(block->getOwner(i), libName, fileName, sizeof(fileName), line))
					sprintf(buffer, "  %s(%d) : caller %d\n", fileName, line, i);
				else
					sprintf(buffer, "  0x%08X : caller %d\n", static_cast<int>(block->getOwner(i)), i);
				(*LogMessage)(buffer);
			}
	}
#endif
}

// ----------------------------------------------------------------------
/**
 * Dump information about all outstanding allocations.
 */

int MemoryManagerNamespace::report(bool leak)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	int count = 0;
	ms_criticalSection->enter();

		// search for the memory pointer
		for (SystemAllocation * systemAllocation = ms_firstSystemAllocation; systemAllocation; systemAllocation = systemAllocation->getNext())
			for (Block * block = systemAllocation->getFirstMemoryBlock()->getNext(); block != systemAllocation->getLastMemoryBlock(); block = block->getNext())
				if (!block->isFree())
				{
#if DO_TRACK
					if (!leak || static_cast<AllocatedBlock*>(block)->checkForLeaks())
#else
					if (!leak)
#endif
					{
						report(static_cast<AllocatedBlock *>(block), leak);
						++count;
					}
 				}

	ms_criticalSection->leave();

	return count;
}

// ----------------------------------------------------------------------
/**
 * Dump information about all outstanding allocations.
 */

void MemoryManager::report()
{
	IGNORE_RETURN(MemoryManagerNamespace::report(false));
}

//-----------------------------------------------------------------------

void MemoryManagerNamespace::logMessageToFd(char const *message)
{
	if (ms_logMessageFd >= 0)
		_write(ms_logMessageFd, message, strlen(message));
}

//-----------------------------------------------------------------------

bool MemoryManagerNamespace::reportToFile(char const *fileName, bool leak)
{
	if (ms_logMessageFd >= 0)
		return false;

	ms_logMessageFd = _open(fileName, O_CREAT | O_TRUNC | O_WRONLY, S_IREAD | S_IWRITE);
	if (ms_logMessageFd < 0)
		return false;

	void (*oldLogMessage)(char const * message) = LogMessage;
	LogMessage = &logMessageToFd;

	report(leak);

	LogMessage = oldLogMessage;

	_close(ms_logMessageFd);
	ms_logMessageFd = -1;

	return true;
}

//-----------------------------------------------------------------------

bool MemoryManager::reportToFile(char const *fileName, bool leak)
{
	return MemoryManagerNamespace::reportToFile(fileName, leak);
}

// ======================================================================
// DISABLE_MEMORY_MANAGER
#else
// ======================================================================
namespace MemoryManagerNamespace
{
	// Memory tracking variables avialble when memory manger is running 
	// in disabled mode for linux
	int                   ms_allocateCalls;
	int                   ms_freeCalls;
	unsigned long         ms_allocateBytesTotal;
#ifndef _WIN32
	int                   ms_processVmSizeKBytes;
#endif
	unsigned long         ms_maxBytesAllocated;	
	int                   ms_allocations;
	int                   ms_maxAllocations;
	bool                  ms_installed;

	struct Statm		//cache for /proc/statm data
	{
		int tpsr;	//total program size resident   
		int rss;	//resident set size share      
	};
	
	Statm s_statm;
	int s_pagesize;

	RecursiveMutex *      ms_criticalSection;
}

using namespace MemoryManagerNamespace;

// ----------------------------------------------------------------------

MemoryManager::MemoryManager()
{
	// Initialize all memory tracking vars 
	MemoryManagerNamespace::ms_allocateCalls = 0;
	MemoryManagerNamespace::ms_freeCalls = 0;
	MemoryManagerNamespace::ms_allocateBytesTotal = 0;
#ifndef _WIN32
	MemoryManagerNamespace::ms_processVmSizeKBytes = 0;
#endif
	MemoryManagerNamespace::ms_maxBytesAllocated = 0;
	MemoryManagerNamespace::ms_allocations = 0;
	MemoryManagerNamespace::ms_maxAllocations= 0;
	
	DEBUG_FATAL(MemoryManagerNamespace::ms_installed, ("already installed"));

	ms_criticalSection = new RecursiveMutex;

	MemoryManagerNamespace::ms_installed = true;

#if defined(PLATFORM_WIN32) && defined(_DEBUG)
   _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);

   int crtHeapFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
   crtHeapFlags |= _CRTDBG_ALLOC_MEM_DF;
   //crtHeapFlags |= _CRTDBG_CHECK_ALWAYS_DF;
   crtHeapFlags |= _CRTDBG_LEAK_CHECK_DF;
   _CrtSetDbgFlag(crtHeapFlags);
#endif
}

// ----------------------------------------------------------------------

MemoryManager::~MemoryManager()
{
	delete ms_criticalSection;
}

// ----------------------------------------------------------------------

void MemoryManager::verify(bool, bool)
{
#if defined(PLATFORM_WIN32) && defined(_DEBUG)
   const int result = _CrtCheckMemory();
   FATAL(result==FALSE, ("Heap corruption detected!\n"));
#endif
}

// ----------------------------------------------------------------------

unsigned long MemoryManager::getCurrentNumberOfBytesAllocated(const int processId)
{
	
#ifdef _WIN32
	UNREF(processId);
	return 0;
#else

	static char filename[30];
	static FILE *inStatmFile;
	static time_t seconds = 0;
	    
	unsigned long retVal = 0;

#if !DISABLED	
	ms_criticalSection->enter();
#endif	

	if (processId) 
	{		
		if ((time (0) - seconds) > 60 )
		{
			if (seconds == 0)
			{
				snprintf(filename, sizeof(filename)-1, "/proc/%d/statm", processId);
				filename[sizeof(filename)-1] = '\0';
				s_pagesize = getpagesize();
			}
			
			seconds = time (0);

			inStatmFile = fopen(filename,"rt");
		
			if (inStatmFile)
			{
				if (fscanf (inStatmFile, "%d %d", &s_statm.tpsr, &s_statm.rss) == 2)
				{
					retVal = static_cast<unsigned long>(s_statm.tpsr) * static_cast<unsigned long>(s_pagesize);
				}
				else
				{
					retVal = 0;
				}
				fclose(inStatmFile);	
			}
			else
			{
				retVal =  0;
			}
		}
		else
		{
			retVal = static_cast<unsigned long>(s_statm.tpsr) * static_cast<unsigned long>(s_pagesize);
		}
	}

#if !DISABLED	
	ms_criticalSection->leave();
#endif	

	return retVal;	
#endif			
}

// ----------------------------------------------------------------------

unsigned long MemoryManager::getCurrentNumberOfBytesAllocatedNoLeakTest()
{
	return 0;
}

// ----------------------------------------------------------------------

unsigned long MemoryManager::getMaximumNumberOfBytesAllocated()
{
#ifdef _WIN32
	return 0;
#else
	return MemoryManagerNamespace::ms_maxBytesAllocated;
#endif	
}

// ----------------------------------------------------------------------

int MemoryManager::getSystemMemoryAllocatedMegabytes()
{
	return 0;
}

// ----------------------------------------------------------------------

int MemoryManager::getCurrentNumberOfAllocations()
{
#ifdef _WIN32
	return 0;
#else
	return MemoryManagerNamespace::ms_allocations;
#endif	
}

// ----------------------------------------------------------------------

int MemoryManager::getMaximumNumberOfAllocations()
{
#ifdef _WIN32
	return 0;
#else	
	return MemoryManagerNamespace::ms_maxAllocations;
#endif	
}

// ----------------------------------------------------------------------

void  MemoryManager::setLimit(int, bool, bool)
{
}

// ----------------------------------------------------------------------

int MemoryManager::getLimit()
{
	return 3072;
}

// ----------------------------------------------------------------------

void MemoryManager::own(void * userPointer)
{
	UNREF(userPointer);
//	__asm int 3;
}

// ----------------------------------------------------------------------

void MemoryManager::setReportAllocations(bool)
{
}

// ----------------------------------------------------------------------

void MemoryManager::registerDebugFlags()
{
}

// ----------------------------------------------------------------------

void MemoryManager::report()
{
}

// ----------------------------------------------------------------------

void * MemoryManager::allocate(size_t size, uint32, bool, bool)
{
#ifdef _WIN32
	return _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__);
	//return (void *)LocalAlloc(LMEM_FIXED, size);
#else
	return ::malloc(size);
#endif	
}

// ----------------------------------------------------------------------

void MemoryManager::free(void * userPointer, bool)
{
#ifdef _WIN32
	_free_dbg(userPointer, _NORMAL_BLOCK);
	//LocalFree((HLOCAL)userPointer);
#else
	::free(userPointer);
	
#endif	
}

// ----------------------------------------------------------------------

void *MemoryManager::reallocate(void *userPointer, size_t newSize)
{
#ifdef _WIN32
	return _realloc_dbg(userPointer, newSize, _NORMAL_BLOCK, __FILE__, __LINE__);
	//return (void *)LocalReAlloc((HLOCAL)userPointer, newSize, 0);
#else	
	return ::realloc(userPointer, newSize);
#endif	
}

// ----------------------------------------------------------------------

void MemoryManager::debugReport(void)
{

}

// ----------------------------------------------------------------------

void MemoryManager::debugReportMap(void)
{

}
#endif

// ----------------------------------------------------------------------
// this function is available regardless of the DISABLE_MEMORY_MANAGER setting

#ifndef _WIN32
int MemoryManager::getProcessVmSizeKBytes(const int processId)
{
	static char filename[30];
	static FILE *inStatusFile;
	static char buffer[128];
	static time_t seconds = 0;

	ms_criticalSection->enter();

	if (processId)
	{
		if ((time (0) - seconds) > 60)
		{
			MemoryManagerNamespace::ms_processVmSizeKBytes = 0;

			if (seconds == 0)
			{
				snprintf(filename, sizeof(filename)-1, "/proc/%d/status", processId);
				filename[sizeof(filename)-1] = '\0';
			}

			seconds = time(0);

			inStatusFile = fopen(filename,"rt");

			if (inStatusFile)
			{
				int temp;

				while (fgets(buffer, sizeof(buffer), inStatusFile))
				{
					if (1 == sscanf(buffer, "VmSize: %d", &temp))
					{
						MemoryManagerNamespace::ms_processVmSizeKBytes = temp;
						break;
					}
				}

				fclose(inStatusFile);
			}
		}
	}

	ms_criticalSection->leave();

	return MemoryManagerNamespace::ms_processVmSizeKBytes;
}
#endif

// ======================================================================
