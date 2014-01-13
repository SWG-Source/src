// ======================================================================
//
// SetupSharedDebug.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/SetupSharedDebug.h"

#include "sharedDebug/CallStackCollector.h"
#include "sharedDebug/DebugMonitor.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/PixCounter.h"
#include "sharedDebug/ProfilerTimer.h"
#include "sharedDebug/Profiler.h"

#include "sharedFoundation/Production.h"

#ifdef _WIN32
#include "sharedDebug/VTune.h"
#endif

// ======================================================================

void SetupSharedDebug::install(const int maxProfilerEntries)
{
	Profiler::install();
	UNREF(maxProfilerEntries);

	ProfilerTimer::install();
	PerformanceTimer::install();
	CallStackCollector::install();

#if PRODUCTION == 0
	PixCounter::install();
#ifdef _WIN32
	VTune::install();
#endif
#endif
}

// ======================================================================
