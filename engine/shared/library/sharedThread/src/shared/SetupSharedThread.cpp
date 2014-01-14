// ======================================================================
//
// SetupSharedThread.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedThread/FirstSharedThread.h"
#include "sharedThread/SetupSharedThread.h"

#include "sharedFoundation/PerThreadData.h"
#include "sharedThread/Thread.h"

// ======================================================================

void SetupSharedThread::install()
{
	// and the per-thread-data is used early on
	PerThreadData::install();

	// Install the thread support stuff
	Thread::install();
}

// ----------------------------------------------------------------------

void SetupSharedThread::remove()
{
	// and the per-thread-data is used early on
	PerThreadData::remove();
}

// ======================================================================
