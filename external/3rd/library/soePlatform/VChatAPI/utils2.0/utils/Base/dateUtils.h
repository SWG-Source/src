#ifndef _SOE_DATE_UTILS_H_
#define _SOE_DATE_UTILS_H_

#include <time.h>

#ifdef USE_ICU
	#include <unicode/calendar.h>
	#include <unicode/timezone.h>
	#include <unicode/utypes.h>
#endif

#define DST_OFFSET_MINUTES_USA 60 // For USA, offset is 1 hour (60 minutes) for Daylight Saving Time
namespace soe 
{
	inline time_t startOfWeek(time_t now, int offset)
    {
        time_t result = now;

        result += (offset * 7 * 24 * 60 * 60);//adjust time by #seconds in week for every week offset specifies (could be negative)

        tm nowTm;

        //get tm struct
        memcpy(&nowTm, localtime(&now), sizeof(tm));

        //reduce input time_t by seconds applied to the current week in progress
        result -= (nowTm.tm_wday * 24 * 60 * 60);//reduce by whole days that have passed since beginning of week
        result -= (nowTm.tm_hour * 60 * 60);//reduce by seconds that have passed today in complete hours today
        result -= (nowTm.tm_min * 60);//reduce by seconds that have passed in partial hours today
        result -= (nowTm.tm_sec);//reduce by seconds that have passed in partial minutes today

        return result;
    }

	inline time_t startOfDay(time_t now, int offset)
    {
        time_t result = now;

        now += (offset * 24 * 60 * 60);//adjust time by #seconds in day for every day offset specifies (could be negative)

        tm nowTm;

        //get tm struct
        memcpy(&nowTm, localtime(&now), sizeof(tm));

        //reset hour/min/sec so go to beginning of day
        nowTm.tm_hour = 0;
        nowTm.tm_min = 0;
        nowTm.tm_sec = 0;

        //turn that into time_t result
        result = mktime(&nowTm);

        return result;
    }

	inline time_t startOfMonth(time_t now, int offset)
    {
        //TODO:  seriously check this algorithm
        time_t result = now;

        //note:  can't do pre-adjustment for offset of months, since months can vary in length

        tm nowTm;

        //get tm struct
        memcpy(&nowTm, localtime(&now), sizeof(tm));

        nowTm.tm_hour = 0;
        nowTm.tm_min = 0;
        nowTm.tm_sec = 0;
        nowTm.tm_mday = 1;//day of month will be 1, since we want time_t of beginning of month


        //calc new month (and possibly year) from offset
        if (offset < 0)
        {
            if (nowTm.tm_mon >= (-1 * offset))
            {
                //stay in current year
                nowTm.tm_mon += offset;
            }
            else
            {
            
                nowTm.tm_mon = nowTm.tm_mon + (offset % 12) - 12;

                //need to adjust year also
                if (-12 <= offset)
                {
                    nowTm.tm_year -= 1;
                }
                else
                {
                    nowTm.tm_year += (offset / 12);
                }
            }
        }
        else
        {
            if (11 - nowTm.tm_mon <= offset)
            {
                //stay in current year
                nowTm.tm_mon += offset;
            }
            else
            {
            
                nowTm.tm_mon = (nowTm.tm_mon + (offset % 12)) - 12;
            
                //need to adjust year also
                if (12 >= offset)
                {
                    nowTm.tm_year += 1;
                }
                else
                {
                    nowTm.tm_year += (offset / 12);
                }
            }
        }


        //turn that into time_t result
        result = mktime(&nowTm);

        return result;
    }

	inline bool isTimeBeforeDay(time_t now, time_t check, int offset=0)
    {
        return (startOfDay(now, offset) > check);
    }

    inline bool isTimeBeforeWeek(time_t now, time_t check, int offset=0)
    {
        return (startOfWeek(now, offset) > check);
    }

    inline bool isTimeBeforeMonth(time_t now, time_t check, int offset=0)
    {
        return (startOfMonth(now, offset) > check);
    }

	inline time_t localTimeToGmt(int dstOffsetMinutes = DST_OFFSET_MINUTES_USA)
	{
		time_t localTime = time(nullptr);
		struct tm gt = *(gmtime(&localTime));
		struct tm lt = *(localtime(&localTime));
		bool isDst = (lt.tm_isdst? true:false);
		return (mktime(&gt) - (isDst? (60 * dstOffsetMinutes):0));
	}	

#ifdef USE_ICU
	inline int getOffsets(const char *id, time_t gmtTime, bool isLocal, int & timeZoneOffsetSecs, int & dstOffsetSecs)
	{
		double d = gmtTime * 1000.00;
		UDate date(d);
		UBool local(isLocal);
		int32_t rawOffsetMS;
		int32_t dstOffsetMS;
		UErrorCode ec = U_ZERO_ERROR;
		int retval(0);

		TimeZone *tz;
		if (!id)
			tz = TimeZone::createDefault();
		else
			tz = TimeZone::createTimeZone(id);

		Calendar *cal = Calendar::createInstance(tz, ec);

		if (U_FAILURE(ec)) 
		{
			//cerr<<"cal failed: ec=("<<ec<<"): "<<u_errorName(ec)<<endl;
			return ec;
		}
		//date = cal->getNow();
		tz->getOffset(date, local, rawOffsetMS, dstOffsetMS, ec);
		if (U_FAILURE(ec)) 
		{
			//cerr<<"tz failed: ec=("<<ec<<"): "<<u_errorName(ec)<<endl;
			return ec;
		}

		timeZoneOffsetSecs = rawOffsetMS/1000;
		dstOffsetSecs = dstOffsetMS/1000;
		return retval;
	}
	inline int getGMT(time_t & theTime, const char *id = nullptr)
	{
		UDate date;
		UErrorCode ec;
		int retval(0);
		UBool isLocal(true);
		int32_t rawOffsetMS;
		int32_t dstOffsetMS;

		//UnicodeString ID = UnicodeString(id);
		TimeZone *tz;
		if (!id)
			tz = TimeZone::createDefault();
		else
			tz = TimeZone::createTimeZone(id);

		Calendar *cal = Calendar::createInstance(tz, ec);

		if (U_FAILURE(ec)) 
		{
			//cerr<<"cal failed: ec=("<<ec<<"): "<<u_errorName(ec)<<endl;
			return ec;
		}
		date = cal->getNow();		//FIXME: this is not returning UTC. it is our time in UTC.
		//date = cal->getTime(ec);
		tz->getOffset(date, isLocal, rawOffsetMS, dstOffsetMS, ec);
	
		UDate tmpDate = date + rawOffsetMS + (cal->inDaylightTime(ec) ? dstOffsetMS : 0);
		theTime = (long)(tmpDate/1000.00);
		
		return retval;
	}

#endif
}

#endif

