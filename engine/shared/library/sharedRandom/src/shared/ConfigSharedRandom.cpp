// ======================================================================
//
// ConfigSharedRandom.cpp
// copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedRandom/FirstSharedRandom.h"
#include "sharedRandom/ConfigSharedRandom.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedRandom", #a,   (b)))

// ======================================================================

namespace ConfigSharedRandomNamespace
{
	bool ms_sample;
}

using namespace ConfigSharedRandomNamespace;

// ======================================================================

void ConfigSharedRandom::install()
{
	KEY_BOOL(sample, false);
}

// ----------------------------------------------------------------------

bool ConfigSharedRandom::getSample()
{
	return ms_sample;
}

// ======================================================================


