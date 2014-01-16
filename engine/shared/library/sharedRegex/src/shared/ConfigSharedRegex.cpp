// ======================================================================
//
// ConfigSharedRegex.cpp
// copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedRegex/FirstSharedRegex.h"
#include "sharedRegex/ConfigSharedRegex.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedRegex", #a,   (b)))

// ======================================================================

namespace ConfigSharedRegexNamespace
{
	bool ms_sample;
}

using namespace ConfigSharedRegexNamespace;

// ======================================================================

void ConfigSharedRegex::install()
{
	KEY_BOOL(sample, false);
}

// ----------------------------------------------------------------------

bool ConfigSharedRegex::getSample()
{
	return ms_sample;
}

// ======================================================================


