// ======================================================================
//
// ConfigSharedPathfinding.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "ConfigSharedPathfinding.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConfigFile.h"

#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("SharedPathfinding", #a, b))
#define KEY_BOOL(a,b)    {ms_ ## a = ConfigFile::getKeyBool("SharedPathfinding", #a, b); DebugFlags::registerFlag(ms_ ## a, "SharedPathfinding", #a);}
#define KEY_FLOAT(a,b)   (ms_ ## a = ConfigFile::getKeyFloat("SharedPathfinding", #a, b))
#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("SharedPathfinding", #a, b))

bool  ConfigSharedPathfinding::ms_autoCreateBuildingPathNodes = false;
bool  ConfigSharedPathfinding::ms_useCityRegions = true;
bool  ConfigSharedPathfinding::ms_jitterCityWaypoints = false;
bool  ConfigSharedPathfinding::ms_enableDirtyBoxes = false;
bool  ConfigSharedPathfinding::ms_enablePathScrubber = false;

// ----------------------------------------------------------------------

void ConfigSharedPathfinding::install ()
{
	KEY_BOOL(autoCreateBuildingPathNodes,false);
	KEY_BOOL(useCityRegions,true);
	KEY_BOOL(jitterCityWaypoints,false);
	KEY_BOOL(enableDirtyBoxes,false);
	KEY_BOOL(enablePathScrubber,false);
}

// ======================================================================
