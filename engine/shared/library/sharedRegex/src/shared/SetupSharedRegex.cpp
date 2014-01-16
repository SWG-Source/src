// ======================================================================
//
// SetupSharedRegex.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedRegex/FirstSharedRegex.h"
#include "sharedRegex/SetupSharedRegex.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedRegex/ConfigSharedRegex.h"
#include "sharedRegex/RegexServices.h"
#include "pcre.h"

// ======================================================================

namespace SetupSharedRegexNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;

	void *(*s_oldMalloc)(size_t);
	void (*s_oldFree)(void*);
}

using namespace SetupSharedRegexNamespace;

// ======================================================================

void SetupSharedRegexNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("SetupSharedRegex not installed."));
	s_installed = false;

	//-- Restore malloc and free functions.
	pcre_malloc = s_oldMalloc;
	pcre_free   = s_oldFree;
}

// ======================================================================

void SetupSharedRegex::install()
{
	InstallTimer const installTimer("SetupSharedRegex::install");

	DEBUG_FATAL(s_installed, ("SetupSharedRegex already installed."));

	ConfigSharedRegex::install();

	//-- Save old malloc and free functions.
	s_oldMalloc = pcre_malloc;
	s_oldFree   = pcre_free;

	//-- Hook up PCRE malloc and free to our memory manager.
	pcre_malloc = &RegexServices::allocateMemory;
	pcre_free   = &RegexServices::freeMemory;

	s_installed = true;
	ExitChain::add(remove, "SetupSharedRegex");
}

// ======================================================================
