// ======================================================================
//
// ConfigSharedIoWin.cpp
// copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedIoWin/FirstSharedIoWin.h"
#include "sharedIoWin/ConfigSharedIoWin.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedIoWin", #a,   (b)))

// ======================================================================

namespace ConfigSharedIoWinNamespace
{
	bool ms_sample;
}

using namespace ConfigSharedIoWinNamespace;

// ======================================================================

void ConfigSharedIoWin::install()
{
	KEY_BOOL(sample, false);
}

// ----------------------------------------------------------------------

bool ConfigSharedIoWin::getSample()
{
	return ms_sample;
}

// ======================================================================


