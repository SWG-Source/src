// ======================================================================
//
// ConfigSharedFoundation.h
// copyright 1998 Bootprint Entertainment
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConfigSharedFoundation_H
#define INCLUDED_ConfigSharedFoundation_H

// ======================================================================

class ConfigSharedFoundation
{
public:

	struct Defaults
	{
		real frameRateLimit;
	};

public:

	static void  install (const Defaults &defaults);

	static real  getFrameRateLimit();
	static real  getMinFrameRate();
	static bool  getNoExceptionHandling();

	static bool  getFpuExceptionPrecision();
	static bool  getFpuExceptionUnderflow();
	static bool  getFpuExceptionOverflow();
	static bool  getFpuExceptionZeroDivide();
	static bool  getFpuExceptionDenormal();
	static bool  getFpuExceptionInvalid();

	static bool  getDemoMode();

	static bool  getUseRemoteDebug();
	static int   getDefaultRemoteDebugPort();

	static bool  getProfilerExpandAllBranches();

	static bool  getMemoryManagerReportAllocations();
	static bool  getMemoryManagerReportOnOutOfMemory();

	static bool  getUseMemoryBlockManager();
	static bool  getMemoryBlockManagerDebugDumpOnRemove();

	static int   getFatalCallStackDepth();
	static int   getWarningCallStackDepth();
	static bool  getLookUpCallStackNames();

	static bool  getAlwaysCanSeeWorldCell();

	static bool  getVerboseWarnings();

	static float getDebugReportLongFrameTime();
};

// ======================================================================

#endif
