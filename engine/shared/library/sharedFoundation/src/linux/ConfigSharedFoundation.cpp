// ======================================================================
//
// ConfigSharedFoundation.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ConfigSharedFoundation.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

const int c_defaultFatalCallStackDepth   = 32;
const int c_defaultWarningCallStackDepth = 8;

// ======================================================================

namespace ConfigSharedFoundationNamespace
{
	real ms_frameRateLimit;
	real ms_minFrameRate;
	bool ms_noExceptionHandling;

	bool ms_fpuExceptionPrecision;
	bool ms_fpuExceptionUnderflow;
	bool ms_fpuExceptionOverflow;
	bool ms_fpuExceptionZeroDivide;
	bool ms_fpuExceptionDenormal;
	bool ms_fpuExceptionInvalid;

	bool ms_demoMode;

	bool ms_useRemoteDebug;
	int  ms_defaultRemoteDebugPort;

	bool ms_profilerExpandAllBranches = true;

	bool ms_memoryManagerReportAllocations;
	bool ms_memoryManagerReportOnOutOfMemory;

	bool ms_useMemoryBlockManager;
	bool ms_memoryBlockManagerDebugDumpOnRemove;

	int  ms_fatalCallStackDepth;
	int  ms_warningCallStackDepth;
	bool ms_lookUpCallStackNames;

	bool ms_alwaysCanSeeWorldCell;

	bool ms_verboseWarnings;

	float ms_debugReportLongFrameTime;
}

using namespace ConfigSharedFoundationNamespace;

// ======================================================================

#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("SharedFoundation", #a, b))
#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedFoundation", #a, b))
#define KEY_FLOAT(a,b)    (ms_ ## a = ConfigFile::getKeyFloat("SharedFoundation", #a, b))
// #define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("SharedFoundation", #a, b))

// ======================================================================
// Determine the Platform-specific configuration information
//
// Remarks:
//
//   This routine inspects the ConfigFile class to set some variables for rapid access
//   by the rest of the engine.

void ConfigSharedFoundation::install (const Defaults &defaults)
{
	KEY_BOOL(noExceptionHandling,             false);

	KEY_BOOL(fpuExceptionPrecision,           false);
	KEY_BOOL(fpuExceptionUnderflow,           false);
	KEY_BOOL(fpuExceptionOverflow,            false);
	KEY_BOOL(fpuExceptionZeroDivide,          false);
	KEY_BOOL(fpuExceptionDenormal,            false);
	KEY_BOOL(fpuExceptionInvalid,             false);

	KEY_BOOL(demoMode,                        false);

	KEY_FLOAT(frameRateLimit,                  defaults.frameRateLimit);
	KEY_FLOAT(minFrameRate,                    0.0f);

	KEY_BOOL(useRemoteDebug,                  false);
	KEY_INT(defaultRemoteDebugPort,           4445);

	KEY_BOOL(profilerExpandAllBranches,       true);
	KEY_BOOL(memoryManagerReportAllocations,  true);
	KEY_BOOL(memoryManagerReportOnOutOfMemory, true);

	KEY_BOOL(useMemoryBlockManager,               true);
	KEY_BOOL(memoryBlockManagerDebugDumpOnRemove, false);

	KEY_INT(fatalCallStackDepth,              c_defaultFatalCallStackDepth);
	KEY_INT(warningCallStackDepth,            c_defaultWarningCallStackDepth);
	KEY_BOOL(lookUpCallStackNames,            true);

	KEY_BOOL(alwaysCanSeeWorldCell,           false);

	KEY_BOOL(verboseWarnings,                 false);

	KEY_FLOAT(debugReportLongFrameTime,       0.25f);
}

// ======================================================================
/**
 * Return the frame rate limit value for the game.
 *
 * @return The initial frame rate limiter value
 */

real ConfigSharedFoundation::getFrameRateLimit(void)
{
	return ms_frameRateLimit;
}

// ----------------------------------------------------------------------
/**
 * Return the minimum frame rate limit value for the game.  Frame that take longer
 * will be set to the minimum and a message will be logged.
 *
 * @return The minimum frame rate value
 */

real ConfigSharedFoundation::getMinFrameRate(void)
{
	return ms_minFrameRate;
}

// ----------------------------------------------------------------------
/**
 * Return whether to run with exception handling enabled.
 *
 * @return True to run without exception handling
 */

bool ConfigSharedFoundation::getNoExceptionHandling(void)
{
	return ms_noExceptionHandling;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionPrecision(void)
{
	return ms_fpuExceptionPrecision;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionUnderflow(void)
{
	return ms_fpuExceptionUnderflow;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionOverflow(void)
{
	return ms_fpuExceptionOverflow;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionZeroDivide(void)
{
	return ms_fpuExceptionZeroDivide;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionDenormal(void)
{
	return ms_fpuExceptionDenormal;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getFpuExceptionInvalid(void)
{
	return ms_fpuExceptionInvalid;
}

// ----------------------------------------------------------------------

int ConfigSharedFoundation::getFatalCallStackDepth()
{
	return ms_fatalCallStackDepth;
}

// ----------------------------------------------------------------------

int ConfigSharedFoundation::getWarningCallStackDepth()
{
	return ms_warningCallStackDepth;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getProfilerExpandAllBranches()
{
	return ms_profilerExpandAllBranches;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getUseRemoteDebug()
{
	return ms_useRemoteDebug;
}

// ----------------------------------------------------------------------

int ConfigSharedFoundation::getDefaultRemoteDebugPort()
{
	return ms_defaultRemoteDebugPort;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getDemoMode()
{
	return ms_demoMode;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getAlwaysCanSeeWorldCell()
{
	return ms_alwaysCanSeeWorldCell;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getLookUpCallStackNames()
{
	return ms_lookUpCallStackNames;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getMemoryManagerReportAllocations()
{
	return ms_memoryManagerReportAllocations;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getMemoryManagerReportOnOutOfMemory()
{
	return ms_memoryManagerReportOnOutOfMemory;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getUseMemoryBlockManager()
{
	return ms_useMemoryBlockManager;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getMemoryBlockManagerDebugDumpOnRemove ()
{
	return ms_memoryBlockManagerDebugDumpOnRemove;
}

// ----------------------------------------------------------------------

bool ConfigSharedFoundation::getVerboseWarnings()
{
	return ms_verboseWarnings;
}

// ----------------------------------------------------------------------

float ConfigSharedFoundation::getDebugReportLongFrameTime()
{
	return ms_debugReportLongFrameTime;
}

// ======================================================================
