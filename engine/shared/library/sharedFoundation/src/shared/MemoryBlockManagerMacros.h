// ======================================================================
//
// MemoryBlockManagerMacros.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MemoryBlockManagerMacros_H
#define INCLUDED_MemoryBlockManagerMacros_H

// ======================================================================

class MemoryBlockManager;

// ======================================================================

#define MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL \
	public: \
		static void                   install(); \
		static void * operator        new(size_t size); \
		static void * operator        new(size_t size, void * placement); \
		static void   operator        delete(void * pointer); \
	private: \
		static void                   remove(); \
		static MemoryBlockManager *   ms_memoryBlockManager

// ----------------------------------------------------------------------

#define MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(className, shared, elementsPerBlock, minimumNumberOfBlocks, maximumNumberOfBlocks) \
	void className::install() \
	{ \
		ms_memoryBlockManager = new MemoryBlockManager( #className " memoryBlockManager", shared, sizeof(className), elementsPerBlock, minimumNumberOfBlocks, maximumNumberOfBlocks); \
		ExitChain::add(&remove, #className "::remove"); \
	} \
	void className::remove() \
	{ \
		DEBUG_FATAL(!ms_memoryBlockManager,(#className " is not installed")); \
		delete ms_memoryBlockManager; \
		ms_memoryBlockManager = 0; \
	} \
	void * className::operator new(size_t size) \
	{ \
		UNREF(size); \
		DEBUG_FATAL(size != sizeof(className),("Incorrect allocation size for class " #className ": %d/%d", size, sizeof(className))); \
		DEBUG_FATAL(!ms_memoryBlockManager, (#className " is not installed")); \
		return ms_memoryBlockManager->allocate(); \
	} \
	void * className::operator new(size_t, void * placement) \
	{ \
		return placement; \
	}\
	void className::operator delete(void * pointer) \
	{ \
		DEBUG_FATAL(!ms_memoryBlockManager,(#className " is not installed")); \
		if (pointer) \
			ms_memoryBlockManager->free(pointer); \
	} \
	MemoryBlockManager * className::ms_memoryBlockManager

// ======================================================================

#define MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL \
	public: \
		static void * operator        new(size_t size); \
		static void * operator        new(size_t size, void * placement); \
		static void   operator        delete(void * pointer); \
	private: \
		static void                   installMemoryBlockManager(); \
		static void                   removeMemoryBlockManager(); \
		static MemoryBlockManager *   ms_memoryBlockManager

// ----------------------------------------------------------------------

#define MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(className, shared, elementsPerBlock, minimumNumberOfBlocks, maximumNumberOfBlocks) \
	void className::installMemoryBlockManager() \
	{ \
		ms_memoryBlockManager = new MemoryBlockManager( #className " memoryBlockManager", shared, sizeof(className), elementsPerBlock, minimumNumberOfBlocks, maximumNumberOfBlocks); \
	} \
	void className::removeMemoryBlockManager() \
	{ \
		DEBUG_FATAL(!ms_memoryBlockManager,(#className " is not installed")); \
		delete ms_memoryBlockManager; \
		ms_memoryBlockManager = 0; \
	} \
	void * className::operator new(size_t size) \
	{ \
		UNREF(size); \
		DEBUG_FATAL(size != sizeof(className),("Incorrect allocation size for class " #className ": %d/%d", size, sizeof(className))); \
		DEBUG_FATAL(!ms_memoryBlockManager, (#className " is not installed")); \
		return ms_memoryBlockManager->allocate(); \
	} \
	void * className::operator new(size_t, void * placement) \
	{ \
		return placement; \
	}\
	void className::operator delete(void * pointer) \
	{ \
		DEBUG_FATAL(!ms_memoryBlockManager,(#className " is not installed")); \
		if (pointer) \
			ms_memoryBlockManager->free(pointer); \
	} \
	MemoryBlockManager * className::ms_memoryBlockManager

// ======================================================================

#endif
