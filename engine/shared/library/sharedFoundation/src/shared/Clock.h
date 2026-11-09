// ======================================================================
//
// Clock.h
//
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Clock_H
#define INCLUDED_Clock_H

// ======================================================================

#include <limits>

// ======================================================================

class Clock
{
private:

	Clock(void);
	Clock(const Clock &);
	Clock &operator =(const Clock &);

private:

	// the instantaneous frame rate of the last frame (1 / lastFrameTime)
	static real    ms_lastFrameRate;

public:

	static void                install(bool newUseSleep, bool useRecalibrationThread);
	static void                remove(void);

	static void                update(void);

	static void                debugReport();

	static DLLEXPORT real      frameTime(void);
	static real                framesPerSecond(void);

	static void                noFrameRateLimit(void);
	static void                setFrameRateLimit(real newFrameRateLimit);
	static void                limitFrameRate(void);

	static void                setMinFrameRate(real newMinFrameRate);
	// Milliseconds on a monotonic clock (host uptime on Linux). The value
	// passes 2^32 after about 49.7 days, so it is 64-bit on every platform.
	// Store absolute values as uint64_t; use durationMs() to narrow a
	// difference into a smaller duration type.
	static const uint64_t      timeMs();
	static const unsigned long timeSeconds();
	static const uint64_t      getFrameStartTimeMs();
	template <typename T>
	static T                   durationMs(uint64_t startMs, uint64_t endMs);
	static double              getCurrentTime();
	static double              getFrameStartTime();

	static const unsigned long getSecondsSinceStart();
	static const int           getTimeZone();

	static void setLongFramesWarningAllowed(bool allowed);
};

// ======================================================================
/**
 * Get the instantaneous frame rate.
 * 
 * This number is really only useful for determining game performance
 * 
 * @return The number of frames per second for the last frame
 * @see Clock::frameTime()
 */

inline real Clock::framesPerSecond(void)
{
	return ms_lastFrameRate;
}

// ----------------------------------------------------------------------
/**
 * Convert the time between two timeMs()/getFrameStartTimeMs() values into a
 * duration type T. The result saturates at T's maximum instead of wrapping,
 * and is 0 if endMs precedes startMs.
 */

template <typename T>
inline T Clock::durationMs(uint64_t const startMs, uint64_t const endMs)
{
	if (endMs <= startMs)
		return static_cast<T>(0);

	uint64_t const elapsedMs = endMs - startMs;
	T const maxValue = (std::numeric_limits<T>::max)();
	if (elapsedMs > static_cast<uint64_t>(maxValue))
		return maxValue;
	return static_cast<T>(elapsedMs);
}

// ======================================================================

#endif

