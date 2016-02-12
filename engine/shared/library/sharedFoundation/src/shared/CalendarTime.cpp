// ======================================================================
//
// CalendarTime.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/CalendarTime.h"

#include <string>

// ======================================================================

std::string CalendarTime::convertEpochToTimeStringGMT(time_t epoch)
{
	std::string result;

	struct tm * timeinfo = ::gmtime(&epoch);
	if (timeinfo == nullptr)
		return result;

	char * asciiTime = ::asctime(timeinfo);
	if (asciiTime == nullptr)
		return result;

	static char resultBuffer[512];
	char * buffer = resultBuffer;
	if (strlen(asciiTime) >= sizeof(resultBuffer))
		buffer = new char[strlen(asciiTime) + 1];

	strcpy(buffer, asciiTime);
	buffer[strcspn(buffer, "\r\n")] = '\0';
	result = buffer;

	if (buffer != resultBuffer)
		delete[] buffer;

	result += " GMT";

	return result;
}

// ======================================================================

std::string CalendarTime::convertEpochToTimeStringGMT_YYYYMMDDHHMMSS(time_t epoch)
{
	std::string result;

	struct tm * timeinfo = ::gmtime(&epoch);
	if (timeinfo == nullptr)
		return result;

	static char resultBuffer[512];
	snprintf(resultBuffer, sizeof(resultBuffer), "%d-%02d-%02d %02d:%02d:%02d GMT", (timeinfo->tm_year + 1900), (timeinfo->tm_mon + 1), timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	return std::string(resultBuffer);
}

//-----------------------------------------------------------------

std::string CalendarTime::convertEpochToTimeStringLocal(time_t epoch)
{
	std::string result;

	struct tm * timeinfo = ::localtime(&epoch);
	if (timeinfo == nullptr)
		return result;

	char * asciiTime = ::asctime(timeinfo);
	if (asciiTime == nullptr)
		return result;

	static char resultBuffer[512];
	char * buffer = resultBuffer;
	if (strlen(asciiTime) >= sizeof(resultBuffer))
		buffer = new char[strlen(asciiTime) + 1];

	strcpy(buffer, asciiTime);
	buffer[strcspn(buffer, "\r\n")] = '\0';
	result = buffer;

	if (buffer != resultBuffer)
		delete[] buffer;

	// get local time zone string
	static char buffer2[256];
	if (0 < ::strftime(buffer2, sizeof(buffer2)-1, "%Z", timeinfo))
	{
		result += " ";
		result += buffer2;
	}
	else
	{
		result += " local";
	}

	return result;
}

//-----------------------------------------------------------------

std::string CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(time_t epoch)
{
	std::string result;

	struct tm * timeinfo = ::localtime(&epoch);
	if (timeinfo == nullptr)
		return result;

	static char resultBuffer[512];
	snprintf(resultBuffer, sizeof(resultBuffer), "%d-%02d-%02d %02d:%02d:%02d ", (timeinfo->tm_year + 1900), (timeinfo->tm_mon + 1), timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	result = resultBuffer;

	if (0 < ::strftime(resultBuffer, sizeof(resultBuffer)-1, "%Z", timeinfo))
	{
		result += resultBuffer;
	}
	else
	{
		result += "local";
	}

	return result;
}

//-----------------------------------------------------------------

std::string CalendarTime::convertSecondsToDHMS(unsigned int numSeconds)
{
	char buffer[128];

	snprintf(buffer, sizeof(buffer)-1, "%ud:%uh:%um:%us",
		numSeconds / (60 * 60 * 24),
		(numSeconds % (60 * 60 * 24)) / (60 * 60),
		(numSeconds % (60 * 60)) / 60,
		numSeconds % 60);

	buffer[sizeof(buffer)-1] = '\0';

	return std::string(buffer);
}

//-----------------------------------------------------------------

std::string CalendarTime::convertSecondsToHMS(unsigned int numSeconds)
{
	char buffer[128];

	snprintf(buffer, sizeof(buffer)-1, "%uh:%um:%us",
		numSeconds / (60 * 60),
		(numSeconds % (60 * 60)) / 60,
		numSeconds % 60);

	buffer[sizeof(buffer)-1] = '\0';

	return std::string(buffer);
}

//-----------------------------------------------------------------

std::string CalendarTime::convertSecondsToMS(unsigned int numSeconds)
{
	char buffer[128];

	snprintf(buffer, sizeof(buffer)-1, "%um:%us",
		numSeconds / 60,
		numSeconds % 60);

	buffer[sizeof(buffer)-1] = '\0';

	return std::string(buffer);
}

//-----------------------------------------------------------------
// given a starting Epoch GMT time, returns the Epoch GMT time when the next specified
// GMT day of week, hour, minute, and second will occur; note that the day of week, hour,
// minute, and second specified will all be interpreted in terms of GMT time
//
// returns -1 if invalid parameters are specified or there are any system errors
// in making the calculation
//
// day of week is 0=Sunday, 1=Monday, 2=Tuesday, 3=Wednesday, 4=Thursday, 5=Friday, 6=Saturday
// if you just want to find out when the next hour, minute, second will occur, specify -1 for
// day of week
//
// hour must be between 0-23
// if you just want to find out when the next minute, second will occur, specify -1 for day
// of week and -1 for hour
//
// minute must be between 0-59
//
// second must be between 0-59

time_t CalendarTime::getNextGMTTimeOcurrence(time_t const startTime, int const dayOfWeek, int const hour, int const minute, int const second)
{
	// don't allow start time to be too far in the past
	if (startTime < CalendarTimeNamespace::g_CalendarTimeEarliestTime)
		return -1;

	if ((dayOfWeek < -1) || (dayOfWeek > 6))
		return -1;

	if ((hour < -1) || (hour > 23))
		return -1;

	if ((hour == -1) && (dayOfWeek != -1))
		return -1;

	if ((minute < 0) || (minute > 59))
		return -1;

	if ((second < 0) || (second > 59))
		return -1;

	// convert the start time into a broken down time containing
	// the year, month, day, day of week, hour, minute, second
	// in GMT time
	struct tm * timeinfo = ::gmtime(&startTime);
	if (timeinfo == nullptr)
		return -1;

	// get number of days until the specified day of week to search for
	if (dayOfWeek >= 0)
	{
		int daysUntil = 0;

		// calculate the Epoch GMT time for the specified start time
		// at the specified hour, minute, and second
		time_t startTimeAtSpecifiedHourMinuteSecond = startTime + (((hour * 60 * 60) + (minute * 60) + second) - ((timeinfo->tm_hour * 60 * 60) + (timeinfo->tm_min * 60) + timeinfo->tm_sec));

		if (timeinfo->tm_wday != dayOfWeek)
		{
			if (timeinfo->tm_wday < dayOfWeek)
				daysUntil = dayOfWeek - timeinfo->tm_wday;
			else
				// wraparound
				daysUntil = (6 - timeinfo->tm_wday) + dayOfWeek + 1;
		}
		else
		{
			// if the specified day of week matches the start day of week, but
			// the specified time has already passed, then we want go forward
			// to next week
			if (startTimeAtSpecifiedHourMinuteSecond <= startTime)
				daysUntil = 7;
		}

		// the next time the that the day/time will match
		startTimeAtSpecifiedHourMinuteSecond += (60 * 60 * 24 * daysUntil);

		return startTimeAtSpecifiedHourMinuteSecond;
	}

	// dayOfWeek == -1
	if (hour >= 0)
	{
		// calculate the Epoch GMT time for the specified start time
		// at the specified hour, minute, and second
		time_t startTimeAtSpecifiedHourMinuteSecond = startTime + (((hour * 60 * 60) + (minute * 60) + second) - ((timeinfo->tm_hour * 60 * 60) + (timeinfo->tm_min * 60) + timeinfo->tm_sec));

		// no day of week specified, we want when the time will
		// match on the day of the start time, or if the time has
		// already passed for the day of the start time, then we
		// when the time will match for the day following the day
		// of the start time
		if (startTimeAtSpecifiedHourMinuteSecond <= startTime)
			startTimeAtSpecifiedHourMinuteSecond += (60 * 60 * 24);

		return startTimeAtSpecifiedHourMinuteSecond;
	}

	// dayOfWeek == -1 and hour == -1
	time_t startTimeAtSpecifiedHourMinuteSecond = startTime + (((minute * 60) + second) - ((timeinfo->tm_min * 60) + timeinfo->tm_sec));

	// no day of week or hour specified, we want when the minute:second
	// will match on the hour of the start time, or if the minute:second
	// has already passed for the hour of the start time, then we want
	// when the minute:second will match for the hour following the
	// hour of the start time
	if (startTimeAtSpecifiedHourMinuteSecond <= startTime)
		startTimeAtSpecifiedHourMinuteSecond += (60 * 60);

	return startTimeAtSpecifiedHourMinuteSecond;
}

//-----------------------------------------------------------------
// given a starting Epoch GMT time, returns the Epoch GMT time when the next specified
// GMT month, day of month, hour, minute, and second will occur; note that the month,
// day of month, hour, minute, and second specified will all be interpreted in terms
// of GMT time
//
// returns -1 if invalid parameters are specified or there are any system errors
// in making the calculation
//
// month must be between 1-12
// if you just want to find out when the next day of month, hour, minute, and second will occur,
// specify -1 for month
//
// dayOfMonth must be between 1-31
//
// hour must be between 0-23
//
// minute must be between 0-59
//
// second must be between 0-59
time_t CalendarTime::getNextGMTTimeOcurrence(time_t const startTime, int const month, int const dayOfMonth, int const hour, int const minute, int const second)
{
	// don't allow start time to be too far in the past
	if (startTime < CalendarTimeNamespace::g_CalendarTimeEarliestTime)
		return -1;

	if (month == 0)
		return -1;

	if ((month < -1) || (month > 12))
		return -1;

	if ((dayOfMonth < 1) || (dayOfMonth > 31))
		return -1;

	if ((hour < 0) || (hour > 23))
		return -1;

	if ((minute < 0) || (minute > 59))
		return -1;

	if ((second < 0) || (second > 59))
		return -1;

	// convert the start time into a broken down time containing
	// the year, month, day, day of week, hour, minute, second
	// in GMT time
	struct tm * timeinfo = ::gmtime(&startTime);
	if (timeinfo == nullptr)
		return -1;

	// calculate the Epoch GMT time for the specified start time
	// at the specified hour, minute, and second
	time_t startTimeAtSpecifiedHourMinuteSecond = startTime + (((hour * 60 * 60) + (minute * 60) + second) - ((timeinfo->tm_hour * 60 * 60) + (timeinfo->tm_min * 60) + timeinfo->tm_sec));

	// timeinfo uses 0-based month
	int const targetMonth = ((month == -1) ? -1 : (month - 1)); 

	// to catch infinite loop caused by stuff like "April 31", but
	// make sure we loop long enough to include the next leap day "February 29"
	int const maxIterations = (((month == 2) && (dayOfMonth == 29)) ? (365*4+1) : (365+1));
	int sentinel = 0;

	while (true)
	{
		if (((targetMonth == -1) || (targetMonth == timeinfo->tm_mon)) && (dayOfMonth == timeinfo->tm_mday) && (startTimeAtSpecifiedHourMinuteSecond > startTime))
			break;

		// start adding 1 day at a time until the target month and day of month is reached
		// TODO: find a better way of doing this rather than looping 1 day at a time
		startTimeAtSpecifiedHourMinuteSecond += (60 * 60 * 24);

		timeinfo = ::gmtime(&startTimeAtSpecifiedHourMinuteSecond);
		if (timeinfo == nullptr)
			return -1;

		if (++sentinel > maxIterations)
			return -1;
	}

	return startTimeAtSpecifiedHourMinuteSecond;
}

//-----------------------------------------------------------------

std::string CalendarTime::getCharacerBirthDateString(int characterBirthDate)
{
	// the birth date (for characters created on live and TC clusters) is the
	// number of days since 00:00:00 Jan 1st, 2001 PST
	// (see the server's PlayerObject::getCurrentBornDate())
	// 978336000 is the epoch value corresponding to 00:00:00 Jan 1st, 2001 PST
	time_t const baseTime = 978336000 + static_cast<time_t>(characterBirthDate * 60 * 60 * 24) - (60 * 60 * 12);

	struct tm const * const birthDateTimeData = ::localtime(&baseTime);

	// get local time zone string
	std::string timeZone("local");

	char buffer[256];
	if (0 < ::strftime(buffer, sizeof(buffer)-1, "%Z", birthDateTimeData))
	{
		timeZone = buffer;
	}

	snprintf(buffer, sizeof(buffer)-1, "%02d %02d %d %s", (birthDateTimeData->tm_mon + 1), birthDateTimeData->tm_mday, (birthDateTimeData->tm_year + 1900), timeZone.c_str());
	buffer[sizeof(buffer)-1] = '\0';

	return std::string(buffer);
}

// ======================================================================
