// ======================================================================
//
// MemoryBlockManager.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedSynchronization/Mutex.h"

#include <algorithm>
#include <map>
#include <vector>

#define PASS_THROUGH_TO_GLOBAL_MEMORY_MANAGER 0

// ======================================================================

class MemoryBlockManager::Allocator
{
public:

	Allocator(int elementSize);
	Allocator(int elementSize, int elementsPerBlock, int minimumBlocks, int maximumNumberOfBlocks);
	~Allocator();

	int    getElementSize();

	void   fetch();
	bool   release();
	int    getReferenceCount();

	bool   isFull() const;
	int    getNumberOfAllocatedElements();
	void  *allocate();
	void   free(void *pointer);

#ifdef _DEBUG
	void   debugDump () const;
#endif

private:

	struct Block
	{
		Block *next;
		byte  *data;
	};

	struct Element
	{
		Element *next;
	};

	void  *realAlloc();
	void   realFree(void *pointer);

	void   placeOnFreeList(Element *element);
	void   allocateNewBlock();

private:

	Allocator();
	Allocator(Allocator const &);
	Allocator &operator =(Allocator const &);

private:

	int          m_referenceCount;

	// first block on the allocated block list
	Block       *m_firstBlock;

	// first free element
	Element     *m_firstFreeElement;

	// size of each element
	int          m_elementSize;

	// number of elements per block
	int          m_elementsPerBlock;

	// current number of blocks in existance
	int          m_currentNumberOfBlocks;

	// current number of blocks that are allowed to exist, 0 for unliminted
	int          m_maximumNumberOfBlocks;

	// number of outstanding allocations
	int          m_numberOfAllocatedElements;

#ifdef _DEBUG
	int          m_lifetimeAllocatedElements;
#endif
};

// ======================================================================

namespace MemoryBlockManagerNamespace
{
	bool ms_debugDumpOnRemove = false;
	bool ms_forceAllNonShared = false;
#ifdef _DEBUG
	bool ms_debugReport              = false;
	int  ms_debugNumberOfAllocations = 0;
	int  ms_debugNumberOfBytes       = 0;
	bool ms_debugDumpAllNextFrame    = false;
	typedef std::vector<MemoryBlockManager*> MemoryBlockManagerList;
	MemoryBlockManagerList ms_memoryBlockManagerList;
#endif

	typedef std::map<int, MemoryBlockManager::Allocator *> Allocators;
	Allocators ms_allocators;

	Mutex ms_globalCriticalSection;

#ifdef _DEBUG
	bool         m_usingMemoryBlockManager = false;
#endif

	void remove();
#ifdef _DEBUG
	void debugReport();
	void debugDumpAll ();
#endif
}

using namespace MemoryBlockManagerNamespace;

// ======================================================================

MemoryBlockManager::Allocator::Allocator(const int elementSize)
:
	m_referenceCount(0),
	m_firstBlock(nullptr),
	m_firstFreeElement(nullptr),
	m_elementSize(elementSize),
	m_elementsPerBlock(64),
	m_currentNumberOfBlocks(0),
	m_maximumNumberOfBlocks(0),
	m_numberOfAllocatedElements(0)
#ifdef _DEBUG
	,
	m_lifetimeAllocatedElements(0)
#endif
{
}

// ----------------------------------------------------------------------

MemoryBlockManager::Allocator::Allocator(int elementSize, int elementsPerBlock, int minimumBlocks, int maximumNumberOfBlocks)
:
	m_referenceCount(0),
	m_firstBlock(nullptr),
	m_firstFreeElement(nullptr),
	m_elementSize(elementSize),
	m_elementsPerBlock(elementsPerBlock),
	m_currentNumberOfBlocks(0),
	m_maximumNumberOfBlocks(maximumNumberOfBlocks),
	m_numberOfAllocatedElements(0)
#ifdef _DEBUG
	,
	m_lifetimeAllocatedElements(0)
#endif
{
	for (int i = 0; i < minimumBlocks; ++i)
		allocateNewBlock();
}

// ----------------------------------------------------------------------

MemoryBlockManager::Allocator::~Allocator()
{
	if (ExitChain::isFataling())
		return;

	while (m_firstBlock)
	{
		Block * block = m_firstBlock;
		m_firstBlock = m_firstBlock->next;
		delete [] block->data;
		delete block;
	}

	m_firstBlock = nullptr;
	m_firstFreeElement = nullptr;
}

// ----------------------------------------------------------------------

void MemoryBlockManager::Allocator::fetch()
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

bool MemoryBlockManager::Allocator::release()
{
	if (--m_referenceCount <= 0)
	{
		DEBUG_FATAL(m_referenceCount < 0, ("reference count went negative"));
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

inline int MemoryBlockManager::Allocator::getElementSize()
{
	return m_elementSize;
}

// ----------------------------------------------------------------------

int MemoryBlockManager::Allocator::getReferenceCount()
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------

bool MemoryBlockManager::Allocator::isFull() const
{
	return m_maximumNumberOfBlocks && m_numberOfAllocatedElements >= m_elementsPerBlock * m_maximumNumberOfBlocks;
}

// ----------------------------------------------------------------------

int MemoryBlockManager::Allocator::getNumberOfAllocatedElements()
{
	return m_numberOfAllocatedElements;
}

// ----------------------------------------------------------------------

void *MemoryBlockManager::Allocator::allocate()
{
	void *result = 0;

	// check if there are any free elements
	if (!m_firstFreeElement)
		allocateNewBlock();

	result = m_firstFreeElement;
	m_firstFreeElement = m_firstFreeElement->next;
	++m_numberOfAllocatedElements;

#ifdef _DEBUG
	m_lifetimeAllocatedElements += 1;
	ms_debugNumberOfAllocations += 1;
	ms_debugNumberOfBytes += getElementSize();
#endif

	return result;
}

// ----------------------------------------------------------------------

void MemoryBlockManager::Allocator::free(void *pointer)
{
	DEBUG_FATAL(m_numberOfAllocatedElements <= 0, ("freeing an element when already empty"));
	Element *element = reinterpret_cast<Element *>(pointer);
	placeOnFreeList(element);
	--m_numberOfAllocatedElements;
}

// ----------------------------------------------------------------------
/**
 * Places an element on the free element list
 */

void MemoryBlockManager::Allocator::placeOnFreeList(Element *element)
{
	NOT_NULL(element);
	element->next = m_firstFreeElement;
	m_firstFreeElement = element;
}

// ----------------------------------------------------------------------
/**
 * Get a new block and put its elements on the free list.
 * 
 * Will not allocate a new block if the maximum number of blocks has already
 * been reached.
 */

void MemoryBlockManager::Allocator::allocateNewBlock()
{
	// allocate a new block
	Block *block = new Block;
	block->data  = new byte[static_cast<size_t>(m_elementSize *	m_elementsPerBlock)];

	// put the block on the block list
	block->next  = m_firstBlock;
	m_firstBlock = block;

	// put all the new block elements on the free list
	{
		int   i;
		byte *data;

		for (i = m_elementsPerBlock, data = block->data; i; --i, data += m_elementSize)
			placeOnFreeList(reinterpret_cast<Element *>(data));  //lint !e826  // suspicious pointer conversion (area too small)
	}

	// count the new block
	++m_currentNumberOfBlocks;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void MemoryBlockManager::Allocator::debugDump() const
{
	const int currentUsageInKilobytes = m_numberOfAllocatedElements * m_elementSize / 1024;
	const int totalUsageInKilobytes = (m_currentNumberOfBlocks * m_elementsPerBlock * m_elementSize) / 1024;
	const int percent = totalUsageInKilobytes ? ((currentUsageInKilobytes * 100) / totalUsageInKilobytes) : 0;
	DEBUG_REPORT_LOG(true, ("MBMA elementSize=%d elements=%d elementsPerBlock=%d blocks=%d mem=%dk full=%d%%\n", m_elementSize, m_numberOfAllocatedElements, m_elementsPerBlock, m_currentNumberOfBlocks, totalUsageInKilobytes, percent));
}
#endif

// ======================================================================

void MemoryBlockManager::install(bool debugDumpOnRemove)
{
	ms_debugDumpOnRemove = debugDumpOnRemove;

#if PASS_THROUGH_TO_GLOBAL_MEMORY_MANAGER
	DEBUG_WARNING(true, ("MemoryBlockManager enabled PASS_THROUGH_TO_GLOBAL_MEMORY_MANAGER"));
#endif

#ifdef _DEBUG
	m_usingMemoryBlockManager = ConfigSharedFoundation::getUseMemoryBlockManager();
#endif
#if PASS_THROUGH_TO_GLOBAL_MEMORY_MANAGER
	m_usingMemoryBlockManager = false;
#endif
	
	DebugFlags::registerFlag(ms_forceAllNonShared,     "SharedFoundation", "memoryBlockManagerForceAllNonShared");
	REPORT_LOG(ms_forceAllNonShared, ("MemoryBlockManager enabled memoryBlockManagerForceAllNonShared\n"));

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_debugReport,           "SharedFoundation", "reportMemoryBlockManager",   debugReport);
	DebugFlags::registerFlag(ms_debugDumpAllNextFrame, "SharedFoundation", "memoryBlockManagerDebugDumpAllNextFrame", debugDumpAll);
#endif

	ExitChain::add(MemoryBlockManagerNamespace::remove, "MemoryBlockManager::remove");
}

// ----------------------------------------------------------------------

void MemoryBlockManagerNamespace::remove()
{
	ms_globalCriticalSection.enter();

		Allocators::iterator iEnd = ms_allocators.end();
		for (Allocators::iterator i = ms_allocators.begin(); i != iEnd; ++i)
		{
			DEBUG_WARNING(true, ("Unfreed MBM %d=size %d=ref %d=items", i->second->getElementSize(), i->second->getReferenceCount(), i->second->getNumberOfAllocatedElements()));
		}

#ifdef _DEBUG
		if (ms_debugDumpOnRemove)
		{
			debugDumpAll();
		}
#endif

	ms_globalCriticalSection.leave();
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void MemoryBlockManagerNamespace::debugReport()
{
	ms_globalCriticalSection.enter();
		int const numberOfAllocations = ms_debugNumberOfAllocations;
		ms_debugNumberOfAllocations = 0;
		int const numberOfBytes = ms_debugNumberOfBytes;
		ms_debugNumberOfBytes = 0;
	ms_globalCriticalSection.leave();

	DEBUG_REPORT_PRINT(true, ("mbm %d=alloc %d=bytes\n", numberOfAllocations, numberOfBytes));
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void MemoryBlockManagerNamespace::debugDumpAll ()
{
	{
		// dump the memory block managers and their private allocators
		uint i;
		for (i = 0; i < ms_memoryBlockManagerList.size (); ++i)
			ms_memoryBlockManagerList [i]->debugDump ();
	}

	{
		// dump the shared allocators
		ms_globalCriticalSection.enter();

			Allocators::iterator iEnd = ms_allocators.end();
			for (Allocators::iterator i = ms_allocators.begin(); i != iEnd; ++i)
				i->second->debugDump();

		ms_globalCriticalSection.leave();
	}

	ms_debugDumpAllNextFrame = false;
}
#endif

// ======================================================================

MemoryBlockManager::MemoryBlockManager(char const * name, bool shared, int elementSize, int elementsPerBlock, int minimumNumberOfBlocks, int maximumNumberOfBlocks)
: m_name(name),
	m_shared(shared),
	m_currentNumberOfElements(0),
	m_allocator(nullptr)
{
	//-- Handle config option where we force all MemoryBlockManagers to be non-shared.
	if (shared && ms_forceAllNonShared)
	{
		m_shared = false;
		shared   = false;

		if (elementsPerBlock <= 0)
		{
			//-- Adjust elements per block: caller specified as 0 because it thought it was going to be shared.
			elementsPerBlock = 100;
		}
	}

	if (elementSize < isizeof(void*))
		elementSize = isizeof(void*);

	if (m_shared)
	{
		DEBUG_FATAL(elementsPerBlock,      ("elementsPerBlock must be 0 if shared"));
		DEBUG_FATAL(minimumNumberOfBlocks, ("minimumNumberOfBlocks must be 0 if shared"));
		DEBUG_FATAL(maximumNumberOfBlocks, ("maximumNumberOfBlocks must be 0 if shared"));

		ms_globalCriticalSection.enter();

			Allocators::iterator i = ms_allocators.find(elementSize);
			if (i == ms_allocators.end())
			{
				// construct it and put it on the map
				i = ms_allocators.insert(Allocators::value_type(elementSize, new Allocator(elementSize))).first;
			}

			m_allocator = i->second;
			m_allocator->fetch();

		ms_globalCriticalSection.leave();
	}
	else
	{
		ms_globalCriticalSection.enter();

			DEBUG_FATAL(elementsPerBlock == 0, ("elementsPerBlock may not be 0 if shared"));
			m_allocator = new Allocator(elementSize, elementsPerBlock, minimumNumberOfBlocks, maximumNumberOfBlocks);
			m_allocator->fetch();

		ms_globalCriticalSection.leave();
	}

#ifdef _DEBUG
	{
		ms_globalCriticalSection.enter();

			ms_memoryBlockManagerList.push_back (this);

		ms_globalCriticalSection.leave();
	}
#endif
}

// ----------------------------------------------------------------------

MemoryBlockManager::~MemoryBlockManager()
{
	if (ExitChain::isFataling())
		return;

	ms_globalCriticalSection.enter();

#ifdef _DEBUG
		{
			MemoryBlockManagerList::iterator iter = std::find (ms_memoryBlockManagerList.begin (), ms_memoryBlockManagerList.end (), this);
			if (iter != ms_memoryBlockManagerList.end ())
				ms_memoryBlockManagerList.erase (iter);
		}
#endif

		if (m_currentNumberOfElements)
		{
			DEBUG_WARNING(!ConfigSharedFoundation::getDemoMode(), ("MemoryBlockManager::~ %d elements still allocated in %s", m_currentNumberOfElements, m_name));
			ms_globalCriticalSection.leave();
			return;
		}

		if (m_allocator->release())
		{
			if (m_shared)
			{
				Allocators::iterator i = ms_allocators.find(getElementSize());
				DEBUG_FATAL(i == ms_allocators.end(), ("Allocator not found"));
				DEBUG_FATAL(i->second != m_allocator, ("wrong allocator"));
				ms_allocators.erase(i);
			}

			delete m_allocator;
		}

	ms_globalCriticalSection.leave();

	m_allocator = 0;
	m_name = 0;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void MemoryBlockManager::debugDump() const
{
	DEBUG_REPORT_LOG(true, ("MBM %s: size=%d elements=%d\n", m_name ? m_name : "<unnamed>", m_allocator->getElementSize(), m_currentNumberOfElements));
	if (!m_shared)
		m_allocator->debugDump();
}
#endif

// ----------------------------------------------------------------------

int MemoryBlockManager::getElementSize() const
{
	return m_allocator->getElementSize();
}

// ----------------------------------------------------------------------

int MemoryBlockManager::getElementCount() const
{
#ifdef _DEBUG
	if(!m_usingMemoryBlockManager)
	{
		return 0;
	}
#endif
	ms_globalCriticalSection.enter();
		int const result = m_currentNumberOfElements;
	ms_globalCriticalSection.leave();
	return result;
}

// ----------------------------------------------------------------------

bool MemoryBlockManager::isFull() const
{
#ifdef _DEBUG
	if(!m_usingMemoryBlockManager)
	{
		return false;
	}
#endif
	ms_globalCriticalSection.enter();
		const bool result = m_allocator->isFull();
	ms_globalCriticalSection.leave();
	return result;
}

// ----------------------------------------------------------------------

void *MemoryBlockManager::allocate(bool returnNullOnFailure)
{
	UNREF(returnNullOnFailure);

#ifdef _DEBUG
	if(!m_usingMemoryBlockManager)
	{
		return operator new(getElementSize());
	}
#endif
	ms_globalCriticalSection.enter();

		if (m_allocator->isFull())
		{
			ms_globalCriticalSection.leave();
			DEBUG_FATAL(!returnNullOnFailure, ("MBM %s is full %d", m_name, m_currentNumberOfElements));
			return nullptr;
		}

		++m_currentNumberOfElements;
		void * result = m_allocator->allocate();

	ms_globalCriticalSection.leave();

	return result;

}

// ----------------------------------------------------------------------

void MemoryBlockManager::free(void *pointer)
{
	if (pointer)
	{
#ifdef _DEBUG
		if(!m_usingMemoryBlockManager)
		{
			operator delete(pointer);
			return;
		}
#endif
	ms_globalCriticalSection.enter();

		if (--m_currentNumberOfElements < 0)
			DEBUG_FATAL(true, ("MBM %s number of allocated elements went negative", m_name));
		m_allocator->free(pointer);

	ms_globalCriticalSection.leave();

	}
}

// ======================================================================
