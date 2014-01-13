// ======================================================================
//
// StaticCallbackEntry.cpp
// Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/StaticCallbackEntry.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(StaticCallbackEntry, true, 0, 0, 0);

// ======================================================================

StaticCallbackEntry::StaticCallbackEntry(unsigned long when, Callback cb, const void *context) :
m_expireCount(when),
m_staticCallbackFunction(cb),
m_context(context)
{
}

//-----------------------------------------------------------------------

StaticCallbackEntry::~StaticCallbackEntry()
{
	// This instance doesn't own these pointers.
	m_staticCallbackFunction = 0;
	m_context                = 0;
}

//-----------------------------------------------------------------------

void StaticCallbackEntry::expired() const
{
	if(m_staticCallbackFunction)
		m_staticCallbackFunction(m_context);
}

// ======================================================================
