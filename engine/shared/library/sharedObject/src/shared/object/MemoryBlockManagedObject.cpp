// ======================================================================
//
// MemoryBlockManagedObject.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/MemoryBlockManagedObject.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(MemoryBlockManagedObject, true, 0, 0, 0);

// ======================================================================

MemoryBlockManagedObject::MemoryBlockManagedObject() :
	Object()
{
}

// ======================================================================
