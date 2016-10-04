// ======================================================================
//
// Clock.cpp
//
// Portions copyright 1998 Bootprint Entertainment
// Portsion copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/Clock.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedDebug/RemoteDebug.h"
#include "sharedSynchronization/Mutex.h"
#include "sharedSynchronization/Semaphore.h"
#include "sharedThread/RunThread.h"
#include "sharedThread/ThreadHandle.h"
#include <string>

#ifdef PLATFORM_WIN32 //@todo code reorg
#include "sharedFoundation/WindowsWrapper.h"
#endif

// ======================================================================

real Clock::ms_lastFrameRate;

// ======================================================================

namespace ClockNamespace
{	
	__int64 ms_lastPoll; // the last time the clock was polled at the start of a frame	
	real  ms_lastFrameTime; // the time (in seconds) that the last frame took	
	real  ms_maxFrameTime; // the maximum time a frame is allowed to be.  If exceeded, a warning is logged.	
	unsigned long ms_applicationStartTime; // the time the application was started
	double ms_realTimeMilliseconds;
	bool  ms_useSleep;
#if PRODUCTION == 0
	bool  ms_debugReportFlag;
	bool  ms_debugReportLogFlag;
#endif
	bool  ms_debugReportLongFrames;
	int   ms_previousNumberOfFilesOpenedTotal;
	int   ms_previousSizeOfFilesOpenedTotal;
	int   ms_timeZone;
	ThreadHandle ms_threadHandle;
	Mutex ms_mutex;
	Semaphore ms_shutdownEvent;
	bool ms_shutdown = false;
	unsigned int const ms_recalibrationInterval = 1000;
	unsigned int const ms_recalibrationMultiplier = 1000 / ms_recalibrationInterval;
	real ms_realFrequency; // periodically calibrated clock frequency, updated from clock thread
	real ms_frameFrequency; // clock frequency being used during the current frame, updated from realFrequency each frame
	real ms_frameRateLimit;
	
	static const double cms_secondsPerMillisecond = 0.001;

#if PRODUCTION == 0
	static void reportMinimumFrameTime();

	bool  ms_reportMinimumFrameTime;
	bool  ms_resetMinimumFrameTime;
	float ms_minimumFrameTime;
	bool  ms_useRecalibrationThread;
	bool  ms_lastUseRecalibrationThread;
#endif

	bool ms_longFramesWarningAllowed = true;

	void threadRoutine();
}
using namespace ClockNamespace;

// ======================================================================
// Install the Clock subsystem
//
// Remarks:
//
//   The Clock currently requires the use of the high performance counter in Windows.
//   If the high performance counter is unavailable, this routine will call Fatal.
//
//   This routine will also add Clock::remove to the ExitChain.
//
// See Also:
//
//   Clock::remove()

void Clock::install(bool newUseSleep, bool useRecalibrationThread)
{
	BOOL result;

	ms_useSleep = newUseSleep;

#if PRODUCTION == 0
	ms_useRecalibrationThread = useRecalibrationThread;
	ms_lastUseRecalibrationThread = useRecalibrationThread;
#endif

	ExitChain::add(Clock::remove, "Clock::remove");

	__int64 frequency;
	result = QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&frequency));
	FATAL(!result, ("Clock::install QPF failed"));

	result = QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&ms_lastPoll));
	FATAL(!result, ("Clock::install QPC failed"));

	ms_realFrequency = static_cast<real>(frequency);
	ms_frameFrequency = ms_realFrequency;

	if (ConfigSharedFoundation::getFrameRateLimit() != 0.0f)
		setFrameRateLimit(ConfigSharedFoundation::getFrameRateLimit());
	setMinFrameRate(ConfigSharedFoundation::getMinFrameRate());

#if PRODUCTION == 0
	DebugFlags::registerFlag(ms_debugReportFlag,        "SharedFoundation", "reportClock", debugReport, -1000);
	DebugFlags::registerFlag(ms_reportMinimumFrameTime, "SharedFoundation", "reportMinimumFrameTime", reportMinimumFrameTime, -990);
	DebugFlags::registerFlag(ms_resetMinimumFrameTime,  "SharedFoundation", "resetMinimumFrameTime");
	DebugFlags::registerFlag(ms_debugReportLogFlag,     "SharedFoundation", "logClock");
	DebugFlags::registerFlag(ms_debugReportLongFrames,  "SharedFoundation", "debugReportLongFrames");
	DebugFlags::registerFlag(ms_useRecalibrationThread, "SharedFoundation", "useClockRecalibrationThread");
#endif

	time_t t = 0;
	localtime(&t);
	ms_timeZone = timezone;
	if(ms_timeZone < (12 * 60 * 60))
	{
		ms_timeZone = -ms_timeZone;
	}
	ms_realTimeMilliseconds = static_cast<double>(ms_lastPoll / ms_frameFrequency * 1000.f);
	ms_applicationStartTime = timeSeconds();

	if (useRecalibrationThread)
	{
		ms_shutdown = false;
		ms_threadHandle = runNamedThread("Clock", threadRoutine);
		ms_threadHandle->setPriority(Thread::kCritical);
	}
}

// ----------------------------------------------------------------------
/**
 * Remove the clock subsystem.
 *
 * This routine should not be called directly by users.  Clock::install() will add
 * this routine onto the ExitChain.
 *
 * @see Clock::install()
 */

void Clock::remove(void)
{
#if PRODUCTION == 0
	DebugFlags::unregisterFlag(ms_debugReportFlag);
	DebugFlags::unregisterFlag(ms_debugReportLogFlag);
	DebugFlags::unregisterFlag(ms_debugReportLongFrames);
#endif

	if (ms_threadHandle)
	{
		ms_mutex.enter();
			ms_shutdown = true;
		ms_mutex.leave();
		ms_shutdownEvent.signal();
		ms_threadHandle->wait();
	}
}

// ----------------------------------------------------------------------
/**
 * Update the clock.
 *
 * This routine will recompute the amount of time the last frame took to complete.
 * If this routine is not called, the values returned by Clock::frameTime() and
 * Clock::framesPerSecond() will not change.
 *
 * @see Clock::frameTime(), Clock::framesPerSecond()
 */

void Clock::update(void)
{
	__int64 newPoll;
	BOOL    result;

#if PRODUCTION == 0
	if (ms_lastUseRecalibrationThread != ms_useRecalibrationThread)
	{
		if (ms_useRecalibrationThread)
		{
			ms_shutdown = false;
			ms_threadHandle = runNamedThread("Clock", threadRoutine);
			ms_threadHandle->setPriority(Thread::kCritical);
		}
		else
		{
			ms_mutex.enter();
			ms_shutdown = true;
			ms_mutex.leave();
			ms_shutdownEvent.signal();
			ms_threadHandle->wait();
		}
		ms_lastUseRecalibrationThread = ms_useRecalibrationThread;
	}
#endif

	result = QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&newPoll));
	FATAL(!result, ("Clock::update QPC failed"));

	// We need to consider time before this frame as having passed with a
	// potentially different frequency, and also need to consider portions
	// of a millisecond.

	real const frameTime = static_cast<real>(newPoll - ms_lastPoll) / ms_frameFrequency;
	ms_realTimeMilliseconds += static_cast<double>(frameTime*1000.f);

	ms_mutex.enter();
	ms_frameFrequency = ms_realFrequency;
	ms_mutex.leave();

	if (!Os::wasFocusLost())
	{
		ms_lastFrameTime = frameTime;

#if PRODUCTION == 0
		float const debugReportLongFrameTime = ConfigSharedFoundation::getDebugReportLongFrameTime();
		if (ms_debugReportLongFrames && ms_lastFrameTime > debugReportLongFrameTime)
		{
			REPORT_LOG(true, ("------------------------------------------------\n"));
			REPORT_LOG(true, ("WARNING: Clock::update[%d] Last Frame took %f seconds\n", Os::getNumberOfUpdates(), ms_lastFrameTime));
			REPORT_LOG(true, ("Files: %4d opened, %4d KB\n", TreeFile::getNumberOfFilesOpenedTotal() - ms_previousNumberOfFilesOpenedTotal, (TreeFile::getSizeOfFilesOpenedTotal() - ms_previousSizeOfFilesOpenedTotal) / 1024));
			Profiler::setTemporaryExpandAll(true);
			Profiler::printLastFrameData();
			REPORT_LOG(true, ("%s\n", Profiler::getLastFrameData()));
			Profiler::setTemporaryExpandAll(false);
		}
#endif

		if (ms_lastFrameTime > ms_maxFrameTime)
		{
			DEBUG_REPORT_LOG(ms_longFramesWarningAllowed, ("WARNING: Clock::update[%d] Last Frame took %f seconds\n", Os::getNumberOfUpdates(), ms_lastFrameTime));
			ms_lastFrameTime = ms_maxFrameTime;
		}

		ms_lastFrameRate = RECIP(ms_lastFrameTime);
	}

	ms_lastPoll = newPoll;


#if PRODUCTION == 0
	ms_previousNumberOfFilesOpenedTotal = TreeFile::getNumberOfFilesOpenedTotal();
	ms_previousSizeOfFilesOpenedTotal = TreeFile::getSizeOfFilesOpenedTotal();

	if (ms_resetMinimumFrameTime || ms_lastFrameTime < ms_minimumFrameTime)
	{
		ms_resetMinimumFrameTime = false;
		ms_minimumFrameTime = ms_lastFrameTime;
	}
#endif
}

// ----------------------------------------------------------------------
/**
 * Display debugging information for the clock.
 */

void Clock::debugReport(void)
{
#if PRODUCTION == 0
	REPORT(true, Report::RF_print | (ms_debugReportLogFlag ? Report::RF_log : 0), ("Clock %6.2f=fps %8.6f=time\n", ms_lastFrameRate, ms_lastFrameTime));
	DEBUG_OUTPUT_STATIC_VIEW_BEGINFRAME("Foundation\\Clock");
	DEBUG_OUTPUT_STATIC_VIEW("Foundation\\Clock", ("Clock %6.2f=fps %6.4f=time\n", ms_lastFrameRate, ms_lastFrameTime));
	DEBUG_OUTPUT_STATIC_VIEW_ENDFRAME("Foundation\\Clock");
#endif
}

// ----------------------------------------------------------------------

#if PRODUCTION == 0

void ClockNamespace::reportMinimumFrameTime(void)
{
	REPORT(true, Report::RF_print, ("min frame time %8.6f\n", ms_minimumFrameTime));
}

#endif

// ----------------------------------------------------------------------
// Get the amount of time the last frame took
//
// Return Value:
//
//   Time, in seconds, for the last frame
//
// Remarks:
//
//   This routine currently does not take in to account if the game was
//   task-switched out of.
//
// See Also:
//
//   Clock::framesPerSecond()

real Clock::frameTime(void)
{
	return ms_lastFrameTime;
}

// ----------------------------------------------------------------------
/**
 * Disable the frame rate limiter.
 *
 * Calling this routine will make the Clock subsystem allow the game to run at
 * the fastest rate possible.
 *
 * @see Clock::setFrameRateLimit()
 */

void Clock::noFrameRateLimit(void)
{
	ms_frameRateLimit = 0.f;
}

// ----------------------------------------------------------------------
/**
 * Enable the frame rate limiter to a specific frame rate value.
 *
 * This routine will set the maximum number of frame-per-second that the game
 * will be allowed to run.  If the game could run faster than that, the Clock
 * subsystem will wait when Clock::limitFrameRate() is called.
 *
 * @param newFrameRateLimit  Frames per second for the clock to limit the game to
 * @see Clock::noFrameRateLimit(), Clock::limitFrameRate()
 */

void Clock::setFrameRateLimit(real newFrameRateLimit)
{
	DEBUG_FATAL(newFrameRateLimit < 1.0f, ("Clock::setFrameRateLimit can not be under 1, %f", newFrameRateLimit));
	ms_frameRateLimit = newFrameRateLimit;
}

// ----------------------------------------------------------------------
/**
 * Set a minimum frame rate for the game.
 *
 * This routine sets a minimum frame rate for the game.  Any frames that take longer will
 * log a warning then have the next frame time set to the given minimum value.
 *
 */

void Clock::setMinFrameRate(real newMinFrameRate)
{
	if (newMinFrameRate == 0)
		ms_maxFrameTime = REAL_MAX;
	else
		ms_maxFrameTime = RECIP(newMinFrameRate);
}


// ----------------------------------------------------------------------
/**
 * Wait if the game is running too fast.
 *
 * If the frame rate limiter has been enabled by calling Clock::setFrameRateLimit(), this
 * routine will make sure that the game never exceeds the framerate specified at
 * that function call.
 *
 * If the frame rate limiter is not enabled, or has been disabled by calling
 * Clock::noFrameRateLimit(), this routine will do nothing.
 *
 * @see Clock::noFrameRateLimit(), Clock::setFrameRateLimit()
 */

void Clock::limitFrameRate(void)
{
	BOOL result;

	// check if the frame rate limiter is on
	if (ms_frameRateLimit)
	{
		__int64 const frameRateLimitTicks = static_cast<__int64>(ms_frameFrequency / ms_frameRateLimit);
		__int64 newPoll, delta;

		for (;;)
		{
			// get the current time
			result = QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&newPoll));
			FATAL(!result, ("Clock::limitFrameRate QPC failed"));

			// compute elapsed time
			delta = newPoll - ms_lastPoll;
			if (delta >= frameRateLimitTicks)
				break;

			//@todo it looks like if delta is huge we will sleep a looong time.  shouldn't we check delta and the frameRatelimit before sleeping?
			if (ms_useSleep)
			{
				const DWORD sleepTicks = static_cast<DWORD>(static_cast<real>((frameRateLimitTicks - delta) / ms_frameFrequency) * 1000.0f);
				if (sleepTicks > 2)
					Sleep(sleepTicks);
			}
		}
	}
}

//-----------------------------------------------------------------------

const unsigned long Clock::timeMs()
{
	__int64 newPoll;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&newPoll));
	double const frameTime = static_cast<double>(newPoll - ms_lastPoll) / ms_frameFrequency;
	return static_cast<unsigned long>(ms_realTimeMilliseconds + frameTime*1000.f);
}

//-----------------------------------------------------------------------

const unsigned long Clock::timeSeconds()
{
	__int64 newPoll;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&newPoll));
	double const frameTime = static_cast<double>(newPoll - ms_lastPoll) / ms_frameFrequency;
	return static_cast<unsigned long>(ms_realTimeMilliseconds/1000.f + frameTime);
}

// ----------------------------------------------------------------------

const unsigned long Clock::getFrameStartTimeMs()
{
	return static_cast<unsigned long>(ms_realTimeMilliseconds);
}

// ----------------------------------------------------------------------

double Clock::getCurrentTime()
{
	__int64 newPoll;
	
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&newPoll));
	
	double const frameTime = static_cast<double>(newPoll - ms_lastPoll) / ms_frameFrequency;

	return getFrameStartTime() + frameTime;
}

// ----------------------------------------------------------------------

double Clock::getFrameStartTime()
{
	return ms_realTimeMilliseconds * cms_secondsPerMillisecond;
}
	
// ----------------------------------------------------------------------

const unsigned long Clock::getSecondsSinceStart()
{
	return timeSeconds() - ms_applicationStartTime;
}

// ----------------------------------------------------------------------

const int Clock::getTimeZone()
{
	return ms_timeZone;
}

// ----------------------------------------------------------------------

void ClockNamespace::threadRoutine()
{
	bool done = false;

	while (!done)
	{
		__int64 startCounter, endCounter;

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&startCounter));
		ms_shutdownEvent.wait(ms_recalibrationInterval);
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&endCounter));

		__int64 newFrequency = (endCounter-startCounter)*ms_recalibrationMultiplier;

		ms_mutex.enter();
		if (ms_shutdown)
			done = true;
		else if (newFrequency)
			ms_realFrequency = static_cast<real>(newFrequency);
		ms_mutex.leave();
	}
}

//----------------------------------------------------------------------

void Clock::setLongFramesWarningAllowed(bool allowed)
{
	ms_longFramesWarningAllowed = allowed;
}

// ======================================================================

