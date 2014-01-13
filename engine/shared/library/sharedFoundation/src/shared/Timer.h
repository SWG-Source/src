// ======================================================================
//
// Timer.h
// Copyright 1998 Bootprint Entertainment
// Copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Timer_H
#define INCLUDED_Timer_H

// ======================================================================

/**
 * Convienent countdown timer class.
 *
 * Timer objects do not automatically update off the clock.  They must be
 * told that time is elapsing by calling one of the update*() functions.
 * The update routine will return true when the timer expires.
 */

class Timer
{
public:

	Timer();
	explicit Timer(real seconds);
	Timer(const Timer &rhs);
	Timer &operator =(const Timer &rhs);

	void setExpireTime(real seconds);
	void extendExpireTime(real seconds);

	void reset();
	void setElapsedTime(float seconds);

	bool updateNoReset(real elapsedTime);
	bool updateZero(real elapsedTime);
	bool updateSubtract(real elapsedTime);

	bool isExpired() const;
	real getExpireTime() const;
	real getElapsedRatio() const;
	real getRemainingRatio() const;
	real getExpirationOvershoot() const;
	real getRemainingTime() const;

private:

	real  expireTime;
	real  time;
};

// ======================================================================
/** Construct a timer object
 *
 * The timer is constructed with the belief that no time has elapsed.
 * This constructor will set the expire-time to 0.
 */

inline Timer::Timer()
: expireTime(0), 
	time(0)
{
}

// ----------------------------------------------------------------------
/**
 * Construct a timer object.
 * 
 * The timer is constructed with the belief that no time has elapsed.
 * 
 * @param seconds  Time (in seconds) that the timer should expire at
 */

inline Timer::Timer(real seconds)
: expireTime(seconds), 
	time(0)
{
}

// ----------------------------------------------------------------------
/**
 * Construct a timer object.
 * 
 * Copy constructor.
 */

inline Timer::Timer(const Timer &rhs)
: expireTime(rhs.expireTime), 
	time(rhs.time)
{
}

// ----------------------------------------------------------------------
/**
 * Assignment operator.
 * 
 * Assignment operator.
 */

inline Timer &Timer::operator =(const Timer &rhs)
{
//	if (this != &rhs)
	{
		expireTime = rhs.expireTime;
		time       = rhs.time;
	}

	return *this;
}

// ----------------------------------------------------------------------
/**
 * Set a new expire time for the timer.
 * 
 * This routine changes the time at which the timer will expire.  It has
 * no effect on the amount of time that the timer believes has already
 * elapsed.
 * 
 * @param seconds  Time (in seconds) that the timer should expire at
 */

inline void Timer::setExpireTime(real seconds)
{
	expireTime = seconds;
}

// ----------------------------------------------------------------------
/**
 * Extends the expire time for the timer.
 * 
 * This routine changes the time at which the timer will expire.  It has
 * no effect on the amount of time that the timer believes has already
 * elapsed.
 * 
 * @param seconds  Time (in seconds) to add to the current expire time
 */

inline void Timer::extendExpireTime(real seconds)
{
	expireTime += seconds;
}

// ----------------------------------------------------------------------
/**
 * Reset the timer.
 * 
 * This resets the timer to believe that no time has elapsed.
 */

inline void Timer::reset()
{
	time = 0.0f;
}

// ----------------------------------------------------------------------
/**
 * Inform the timer that time has elapsed, but not not reset the timer.
 * 
 * When the timer expires, it will remain expired until reset() is called
 * or one of the update*() functions is called that will reduce the amount
 * of elapsed time.
 * 
 * @param elapsedTime  Amount of time (in seconds) that has elapsed
 * @return True if the timer has expired, false otherwise.
 * @see Timer::reset(), Timer::updateZero(), Timer::updateSubtract()
 */

inline bool Timer::updateNoReset(real elapsedTime)
{
	time += elapsedTime;
	return (time >= expireTime);
}

// ----------------------------------------------------------------------
/**
 * Inform the timer that time has elapsed, and reset the time if it expired.
 * 
 * When this routine detects the timer has expired, it will reset the timer.
 * This will guarentee at least the amount of time has passed between
 * expirations, but will not necessarily behave properly over an extended
 * period of time.
 * 
 * @return True if the timer has expired, false otherwise.
 * @see Timer::reset(), Timer::updateNoReset(), Timer::updateSubtract()
 */

inline bool Timer::updateZero(real elapsedTime)
{
	if (updateNoReset(elapsedTime))
	{
		reset();
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------
/**
 * Inform the timer that time has elapsed, and reduce the time if it expired.
 * 
 * When this routine detects the timer has expired, it will subtract the
 * timer expiration time from the current time value.  This routine will
 * tend to remain more accurate over a longer period of time, but will
 * not necessarily guarentee that the specified time will elapse before the
 * next expiration (because of carry-over from the previous update).
 * 
 * @return True if the timer has expired, false otherwise.
 * @see Timer::reset(), Timer::updateNoReset(), Timer::updateSubtract()
 */

inline bool Timer::updateSubtract(real elapsedTime)
{
	if (updateNoReset(elapsedTime))
	{
		time -= expireTime;
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------
/**
 * Check if the timer is expired.
 * 
 * This routine does not update the timer's current time.
 *
 * @return true if the timer has expired, otherwise false.
 */

inline bool Timer::isExpired() const
{
	return time >= expireTime;
}

// ----------------------------------------------------------------------
/**
 * Return the expire time
 * 
 * @return expire time
 */

inline real Timer::getExpireTime() const
{
	return expireTime;
}

// ----------------------------------------------------------------------
/**
 * Calculate ratio (from 0 to 1) of time elapsed.
 * 
 * Should not be called if the expire time has not been set, as the result
 * is undefined.
 * 
 * @return ratio (from 0 to 1) of time elapsed
 */

inline real Timer::getElapsedRatio() const
{
	return expireTime != 0.0f ? time / expireTime : 0.0f;
}

// ----------------------------------------------------------------------
/**
 * Calculate ratio (from 0 to 1) of time remaining.
 * 
 * Should not be called if the expire time has not been set, as the result
 * is undefined.
 * 
 * @return ratio (from 0 to 1) of time remaining
 */

inline real Timer::getRemainingRatio() const
{
	return expireTime != 0.0f ? ((expireTime - time) / expireTime) : 0.0f;
}

// ----------------------------------------------------------------------
/**
 * Return the amount of time beyond the expiration time we are.
 * 
 * The value returned is negative if we haven't expired yet.
 *
 * @return the overshoot time
 */

inline real Timer::getExpirationOvershoot() const
{
	return time - expireTime;
}

// ----------------------------------------------------------------------
/**
 * Return the amount of time until expiration.
  *
 * @return the overshoot time
 */

inline real Timer::getRemainingTime() const
{
	return expireTime - time;
}

// ======================================================================

#endif
