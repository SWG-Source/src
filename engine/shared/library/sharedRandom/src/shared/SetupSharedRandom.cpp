// ======================================================================
//
// SetupSharedRandom.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedRandom/FirstSharedRandom.h"
#include "sharedRandom/SetupSharedRandom.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedRandom/ConfigSharedRandom.h"
#include "sharedRandom/Random.h"

// ======================================================================

void SetupSharedRandom::install(uint32 seed)
{
	InstallTimer const installTimer("SetupSharedRandom::install");

	ConfigSharedRandom::install();
	Random::install(seed);
}

// ======================================================================
