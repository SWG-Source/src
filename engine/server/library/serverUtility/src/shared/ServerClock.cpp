// ServerClock.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/ServerClock.h"
#include "sharedLog/Log.h"
#include <time.h>
#include <string>

// ----------------------------------------------------------------------

const int32_t ServerClock::cms_endOfTime = static_cast<int32_t>(-1);

//-----------------------------------------------------------------------

ServerClock::ServerClock() :
serverFrame(0),
subtractInterval(0),
lastTime(time(0)),
heldBack(false)
{
}

//-----------------------------------------------------------------------

ServerClock::~ServerClock()
{
}

//-----------------------------------------------------------------------

const uint32_t ServerClock::getGameTimeSeconds() const
{
	FATAL(!isSet(), ("ServerClock::getGameTimeSeconds: Clock was not set."));
	return static_cast<uint32_t>(lastTime - subtractInterval);
}

//-----------------------------------------------------------------------

void ServerClock::incrementServerFrame()
{
	const time_t currentTime = time(0);

	// Game time must never go backwards: timestamps already handed out
	// (idle timers, stored expiry times) assume it only increases. If the
	// host clock steps back, hold game time until the host clock catches up.
	if (currentTime < lastTime)
	{
		if (!heldBack)
		{
			LOG("ServerClock", ("Host clock stepped back by %ld seconds; holding game time until it catches up", static_cast<long>(lastTime - currentTime)));
			heldBack = true;
		}
	}
	else
	{
		if (heldBack)
		{
			LOG("ServerClock", ("Host clock caught up; game time is advancing again"));
			heldBack = false;
		}
		else if (currentTime > lastTime + 120)
			LOG("ServerClock", ("Clock changed by more than 120 seconds in a frame (%ld to %ld)!", static_cast<long>(lastTime), static_cast<long>(currentTime)));
		lastTime = currentTime;
	}

	serverFrame++;
}

//-----------------------------------------------------------------------

void ServerClock::setSubtractInterval(const uint32_t newSubtractInterval)
{
	subtractInterval = newSubtractInterval;
}

//-----------------------------------------------------------------------

void ServerClock::setGameTimeSeconds(const uint32_t newGameTime)
{
	// Base the interval on lastTime, which game time is computed from, so
	// game time is exactly newGameTime now. lastTime never moves backwards.
	const time_t currentTime = time(0);
	if (currentTime > lastTime)
		lastTime = currentTime;
	subtractInterval = static_cast<uint32_t>(lastTime - newGameTime);
	LOG("ServerClock", ("Game time set to %u (subtract interval %u)", newGameTime, subtractInterval));
}

//-----------------------------------------------------------------------

ServerClock &ServerClock::getInstance()
{
	static ServerClock theInstance;
	return theInstance;
}

// ----------------------------------------------------------------------

/**
 * Given a time in seconds, make a string that expresses how long it is.
 * For debug output only.  In English, not localized.  Not thread-safe.
 */
std::string ServerClock::getDebugPrintableTimeframe(uint32_t const timeInSeconds)
{
	uint32_t const dayInSeconds = 60 * 60 * 24;
	uint32_t const hourInSeconds = 60 * 60;
 	uint32_t const minuteInSeconds = 60;

	static char buffer[256];
	
	if (timeInSeconds > dayInSeconds)
	{
		snprintf(buffer, 256, "~ %u days", timeInSeconds / dayInSeconds);
	}
	else if (timeInSeconds > hourInSeconds)
	{
		snprintf(buffer, 256, "~ %u hours", timeInSeconds / hourInSeconds);
	}
	else if (timeInSeconds > minuteInSeconds)
	{
		snprintf(buffer, 256, "~ %u minutes", timeInSeconds / minuteInSeconds);
	}
	else
	{
		snprintf(buffer, 256, "%u seconds", timeInSeconds);
	}

	buffer[255]='\0';
	return std::string(buffer);
}

// ======================================================================

