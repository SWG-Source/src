// ======================================================================
//
// ConfigSharedFoundation.h
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
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
		int  screenHeight;
		int  screenWidth;
		bool windowed;
		real frameRateLimit;
		bool demoMode;
		bool verboseWarnings;
	};

public:

	static void  install(const Defaults &defaults);

	static bool           getNoExceptionHandling();

	static bool           getFpuExceptionPrecision();
	static bool           getFpuExceptionUnderflow();
	static bool           getFpuExceptionOverflow();
	static bool           getFpuExceptionZeroDivide();
	static bool           getFpuExceptionDenormal();
	static bool           getFpuExceptionInvalid();

	static bool           getDemoMode();

	static real           getFrameRateLimit();
	static real           getMinFrameRate();

	static bool           getUseRemoteDebug();
	static int            getDefaultRemoteDebugPort();

	static bool           getProfilerExpandAllBranches();

	static bool           getMemoryManagerReportAllocations();
	static bool           getMemoryManagerReportOnOutOfMemory();

	static bool           getUseMemoryBlockManager();
	static bool           getMemoryBlockManagerDebugDumpOnRemove();

	static int            getFatalCallStackDepth();
	static int            getWarningCallStackDepth();
	static bool           getLookUpCallStackNames();

	static int            getProcessPriority();

	static DLLEXPORT bool getVerboseHardwareLogging();
	static bool           getVerboseWarnings();
	static void           setVerboseWarnings(bool verboseWarnings);

	static bool           getCauseAccessViolation();

	static float          getDebugReportLongFrameTime();
};

// ======================================================================

#endif
