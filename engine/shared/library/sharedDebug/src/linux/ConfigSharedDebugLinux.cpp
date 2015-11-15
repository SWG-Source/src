// ======================================================================
//
// ConfigSharedDebugLinux.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/ConfigSharedDebugLinux.h"

#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConfigFile.h"

//===================================================================

namespace ConfigSharedDebugLinuxNamespace
{
	bool        s_useTty;
	const char *s_debugMonitorOutputFilename;
	bool        s_logTtySetup;
}

using namespace ConfigSharedDebugLinuxNamespace;

//===================================================================

#define KEY_BOOL(a,b)   (s_ ## a = ConfigFile::getKeyBool("SharedDebugLinux", #a, b))
#define KEY_STRING(a,b) (s_ ## a = ConfigFile::getKeyString("SharedDebugLinux", #a, b))
// #define KEY_INT(a,b)    (s_ ## a = ConfigFile::getKeyInt("ClientAnimation", #a,  b))
// #define KEY_FLOAT(a,b)  (s_ ## a = ConfigFile::getKeyFloat("ClientAnimation", #a, b))

//===================================================================

void ConfigSharedDebugLinux::install(void)
{
	KEY_BOOL(useTty, false);
	KEY_STRING(debugMonitorOutputFilename, "ttySpecifier.txt");
	KEY_BOOL(logTtySetup, false);

	//-- Handle startup profiler handling here.
	//   @todo this should move into ConfigSharedDebug or be loaded
	//         by Profiler.  Profiler can't load this config file option
	//         because the Profiler is installed prior to the config file
	//         system.
	Profiler::enableProfilerOutput(ConfigFile::getKeyBool("SharedDebugLinux", "reportProfiler", false));
}

// ------------------------------------------------------------------

bool ConfigSharedDebugLinux::getUseTty()
{
	return s_useTty;
}

// ------------------------------------------------------------------

char const *ConfigSharedDebugLinux::getDebugMonitorOutputFilename()
{
	return s_debugMonitorOutputFilename;
}

// ------------------------------------------------------------------

bool ConfigSharedDebugLinux::getLogTtySetup()
{
	return s_logTtySetup;
}

//===================================================================
