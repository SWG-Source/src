#include "date.h"

#ifdef  WIN32
#	include <sys/types.h>
#	include <sys/timeb.h>
#endif

#include <stdio.h>
#include <time.h>

#ifdef WIN32
#define snprintf _snprintf
#endif


namespace soe
{
	static time_t date_timegm(struct tm *T)
	{
#ifdef WIN32
		struct _timeb t;

		memset((void*)&t, 0, sizeof(t));
		t.timezone = 0;
		t.dstflag = -1;
		_ftime(&t);
		T->tm_min -= t.timezone;
		T->tm_isdst = 0;

		return mktime(T);
#else
		return timegm(T);
#endif
	}

	Date::Date(time_t src)
	: mYear(1900)
	, mMonth(1)
	, mDay(1)
	, mHour(0)
	, mMinute(0)
	, mSecond(0)
	{
		struct tm timeStruct;
		struct tm *gotTime = nullptr;
#ifdef WIN32
		gotTime = gmtime(&src);
#else
		gotTime = gmtime_r(&src, &timeStruct);
#endif

		if (gotTime) {
#ifdef WIN32
			timeStruct = *gotTime;
#endif
			mYear = timeStruct.tm_year + 1900;
			mMonth = timeStruct.tm_mon + 1;
			mDay = timeStruct.tm_mday;
			mHour = timeStruct.tm_hour;
			mMinute = timeStruct.tm_min;
			mSecond = timeStruct.tm_sec;
		}
	}

	Date::Date(const std::string &src)
	: mYear(0)
	, mMonth(0)
	, mDay(0)
	, mHour(0)
	, mMinute(0)
	, mSecond(0)
	{
#ifdef WIN32
		// This code should actually be portable to Linux as well - rlsmith 05/31/2006
		int year = 0;
		int month = 0;
		int day = 0;
		int hour = 0;
		int minute = 0;
		int second = 0;
		int res(0);
		if (
			(6 == (res=sscanf(src.c_str(), "%4d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second))) ||
			(6 == (res=sscanf(src.c_str(), "%4d/%02d/%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second))) ||
			(6 == (res=sscanf(src.c_str(), "%02d-%02d-%4d %02d:%02d:%02d", &month, &day, &year, &hour, &minute, &second))) ||
			(6 == (res=sscanf(src.c_str(), "%02d/%02d/%4d %02d:%02d:%02d", &month, &day, &year, &hour, &minute, &second))) ||
			(3 == (res=sscanf(src.c_str(), "%02d-%02d-%4d", &month, &day, &year))) ||
			(3 == (res=sscanf(src.c_str(), "%02d/%02d/%4d", &month, &day, &year))) ||
			(3 == (res=sscanf(src.c_str(), "%4d-%02d-%02d", &year, &month, &day))) ||
			(3 == (res=sscanf(src.c_str(), "%4d/%02d/%02d", &year, &month, &day)))
			)
		{
			if (res < 6) // not all time values were parsed, so reset time to 00:00:00
			{
				mHour = 0;
				mMinute = 0;
				mSecond = 0;
			}
			mYear = year;
			mMonth = month;
			mDay = day;
			mHour = hour;
			mMinute = minute;
			mSecond = second;
		}
#else
		tm timeStruct;
		memset(&timeStruct, 0, sizeof(timeStruct));

		if (
			strptime(src.c_str(), "%Y-%m-%d %H:%M:%S", &timeStruct) || strptime(src.c_str(), "%Y/%m/%d %H:%M:%S", &timeStruct)
			|| strptime(src.c_str(), "%m-%d-%Y %H:%M:%S", &timeStruct) || strptime(src.c_str(), "%m/%d/%Y %H:%M:%S", &timeStruct)
			|| strptime(src.c_str(), "%Y-%m-%d", &timeStruct) || strptime(src.c_str(), "%Y/%m/%d", &timeStruct)
			|| strptime(src.c_str(), "%m-%d-%Y", &timeStruct) || strptime(src.c_str(), "%m/%d/%Y", &timeStruct)
			)
		{
			mYear = timeStruct.tm_year + 1900;
			mMonth = timeStruct.tm_mon + 1;
			mDay = timeStruct.tm_mday;
			mHour = timeStruct.tm_hour;
			mMinute = timeStruct.tm_min;
			mSecond = timeStruct.tm_sec;
		}
#endif
	}

	Date::operator std::string() const
	{
		char buffer[256];

		snprintf(buffer, sizeof(buffer), PRINT_FORMAT(), mYear, mMonth, mDay, mHour, mMinute, mSecond);

		return std::string(buffer);
	}

	Date::operator time_t() const
	{
		struct tm timeStruct;

		timeStruct.tm_year = mYear - 1900;
		timeStruct.tm_mon = mMonth - 1;
		timeStruct.tm_mday = mDay;
		timeStruct.tm_hour = mHour;
		timeStruct.tm_min = mMinute;
		timeStruct.tm_sec = mSecond;

		time_t t = date_timegm(&timeStruct);

		return t;
	}

#define COMPARE_MEMBER(__member_name__, __oper__)						\
		if (__member_name__ != other.__member_name__) {					\
			return (__member_name__ __oper__ other.__member_name__);	\
		} else

#define COMPARE_MEMBER_END(__member_name__, __oper__)					\
		{																\
			return (__member_name__ __oper__ other.__member_name__);	\
		}

#define COMPARE_DATE_OPER_IMPL(__oper__)								\
	bool Date::operator __oper__(const Date & other) const				\
	{																	\
		COMPARE_MEMBER(mYear, __oper__)									\
		COMPARE_MEMBER(mMonth, __oper__)								\
		COMPARE_MEMBER(mDay, __oper__)									\
		COMPARE_MEMBER(mHour, __oper__)									\
		COMPARE_MEMBER(mMinute, __oper__)								\
		COMPARE_MEMBER_END(mSecond, __oper__)							\
	}

	COMPARE_DATE_OPER_IMPL(>)
	COMPARE_DATE_OPER_IMPL(>=)
	COMPARE_DATE_OPER_IMPL(==)
	COMPARE_DATE_OPER_IMPL(<=)
	COMPARE_DATE_OPER_IMPL(<)
}
