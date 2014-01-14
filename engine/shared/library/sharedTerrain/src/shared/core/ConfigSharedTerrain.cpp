//===================================================================
//
// ConfigSharedTerrain.cpp
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ConfigSharedTerrain.h"

#include "sharedFoundation/ConfigFile.h"

//===================================================================

namespace
{
	int  ms_maximumNumberOfChunksAllowed;
	bool ms_disableGetHeight;
	bool ms_debugReportInstall;
	bool ms_debugReportLogPrint;
	bool ms_disableFloraCaching;
	float ms_maximumValidHeightInMeters;
}

//===================================================================

int ConfigSharedTerrain::getMaximumNumberOfChunksAllowed ()
{
	return ms_maximumNumberOfChunksAllowed;
}

//-------------------------------------------------------------------

bool ConfigSharedTerrain::getDisableGetHeight ()
{
	return ms_disableGetHeight;
}

//-------------------------------------------------------------------

bool ConfigSharedTerrain::getDebugReportInstall ()
{
	return ms_debugReportInstall;
}

//-------------------------------------------------------------------

bool ConfigSharedTerrain::getDebugReportLogPrint ()
{
	return ms_debugReportLogPrint;
}

//-------------------------------------------------------------------

bool ConfigSharedTerrain::getDisableFloraCaching ()
{
	return ms_disableFloraCaching;
}

//-------------------------------------------------------------------

float ConfigSharedTerrain::getMaximumValidHeightInMeters ()
{
	return ms_maximumValidHeightInMeters;
}

//===================================================================

#define KEY_BOOL(a,b) (ms_ ## a = ConfigFile::getKeyBool ("SharedTerrain", #a, b))
#define KEY_INT(a,b) (ms_ ## a = ConfigFile::getKeyInt ("SharedTerrain", #a, b))
#define KEY_FLOAT(a,b) (ms_ ## a = ConfigFile::getKeyFloat ("SharedTerrain", #a, b))
//#define KEY_STRING (a,b) (ms_ ## a = ConfigFile::getKeyString ("SharedTerrain", #a, b))

//===================================================================

void ConfigSharedTerrain::install ()
{
	KEY_INT (maximumNumberOfChunksAllowed, 40 * 1024);
	KEY_BOOL (disableGetHeight, false);
	KEY_BOOL (debugReportInstall, false);
	KEY_BOOL (debugReportLogPrint, false);
	KEY_BOOL (disableFloraCaching, false);
	KEY_FLOAT (maximumValidHeightInMeters, 16000.0f);

	DEBUG_REPORT_LOG_PRINT (ms_debugReportInstall, ("ConfigSharedTerrain::install\n"));
}

//===================================================================
