// ======================================================================
//
// CalendarTime.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CalendarTime_H
#define INCLUDED_CalendarTime_H

// ======================================================================

// Sat Jan 01 00:00:00 1980 (just an arbitrary value)
namespace CalendarTimeNamespace
{
	const time_t g_CalendarTimeEarliestTime = 315550800;
};

class CalendarTime
{
private:

	CalendarTime(void);
	CalendarTime(const CalendarTime &);
	CalendarTime &operator =(const CalendarTime &);

public:

	static std::string convertEpochToTimeStringGMT(time_t epoch);
	static std::string convertEpochToTimeStringGMT_YYYYMMDDHHMMSS(time_t epoch);
	static std::string convertEpochToTimeStringLocal(time_t epoch);
	static std::string convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(time_t epoch);
	static std::string convertSecondsToDHMS(unsigned int numSeconds);
	static std::string convertSecondsToHMS(unsigned int numSeconds);
	static std::string convertSecondsToMS(unsigned int numSeconds);
	static time_t getNextGMTTimeOcurrence(time_t startTime, int dayOfWeek, int hour, int minute, int second);
	static time_t getNextGMTTimeOcurrence(time_t startTime, int month, int dayOfMonth, int hour, int minute, int second);

	static std::string getCharacerBirthDateString(int characterBirthDate);
};

#endif
