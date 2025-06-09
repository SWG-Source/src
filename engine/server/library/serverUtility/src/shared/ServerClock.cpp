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
lastTime(time(0))
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

	if (currentTime < lastTime-120 || currentTime > lastTime+120)
		LOG("ServerClock", ("Clock changed by more than 120 seconds in a frame (%lu to %lu)!", lastTime, currentTime));
	lastTime = currentTime;

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
	subtractInterval = static_cast<uint32_t>(time(0) - newGameTime);
	LOG("ServerClock", ("Game time set to %lu (subtract interval %lu)", newGameTime, subtractInterval));
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
		snprintf(buffer, 256, "~ %lu days", timeInSeconds / dayInSeconds);
	}
	else if (timeInSeconds > hourInSeconds)
	{
		snprintf(buffer, 256, "~ %lu hours", timeInSeconds / hourInSeconds);
	}
	else if (timeInSeconds > minuteInSeconds)
	{
		snprintf(buffer, 256, "~ %lu minutes", timeInSeconds / minuteInSeconds);
	}
	else
	{
		snprintf(buffer, 256, "%lu seconds", timeInSeconds);
	}

	buffer[255]='\0';
	return std::string(buffer);
}

// ======================================================================

