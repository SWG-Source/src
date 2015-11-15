// ======================================================================
//
// MemoryBlockManagedObject.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MemoryBlockManagedObject_H
#define INCLUDED_MemoryBlockManagedObject_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/Object.h"

class MemoryBlockManager;

// ======================================================================
/**
 * Provide a MemoryBlockManaged version of Object.  
 *
 * This class adds no new behavior to Object except for the per-instance 
 * memory allocation scheme.
 *
 * The client frequently creates base Object instances for use in client-side
 * effects.  Object is not mem block managed.  Making Object mem block managed
 * with the existing macros will force us to mem block manage all Object-derived
 * classes and will require changes to MemoryBlockManagerMacros implementation
 * (due to the static ms_memoryBlockManager instance that is part of the class
 * interface).  ALS and TRF decided this was the most effective solution.
 */

class MemoryBlockManagedObject: public Object
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	MemoryBlockManagedObject();

private:

	// Disabled.
	MemoryBlockManagedObject(MemoryBlockManagedObject const&);
	MemoryBlockManagedObject &operator =(MemoryBlockManagedObject const&);

};

// ======================================================================

#endif
