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
	static const unsigned long timeMs();
	static const unsigned long timeSeconds();
	static const unsigned long getFrameStartTimeMs();
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

// ======================================================================

#endif

