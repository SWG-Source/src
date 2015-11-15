// ======================================================================
//
// ConfigSharedMath.cpp
// copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/ConfigSharedMath.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

#define KEY_BOOL(a,b)   (ms_ ## a = ConfigFile::getKeyBool("SharedMath", #a, (b)))

// ======================================================================

namespace ConfigSharedMathNamespace
{
	bool ms_reportRangeLoopWarnings;
}

using namespace ConfigSharedMathNamespace;

// ======================================================================

void ConfigSharedMath::install()
{
	KEY_BOOL(reportRangeLoopWarnings, false);
}

// ----------------------------------------------------------------------

bool ConfigSharedMath::getReportRangeLoopWarnings()
{
	return ms_reportRangeLoopWarnings;
}

// ======================================================================


