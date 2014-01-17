#ifndef SOE__DATE_H
#define SOE__DATE_H

#include "types.h"
#include "serializeClasses.h"

namespace soe 
{

	struct Date
	{
		Date()
			: mYear(0)
			, mMonth(0)
			, mDay(0)
			, mHour(0)
			, mMinute(0)
			, mSecond(0)
		{ }

		Date(time_t src);
		Date(const std::string &src);

		operator std::string() const;
		operator time_t() const;

		bool operator >(const Date & other) const;
		bool operator >=(const Date & other) const;
		bool operator ==(const Date & other) const;
		bool operator <=(const Date & other) const;
		bool operator <(const Date & other) const;

		static const char * const DATA_FORMAT() { return "YYYY-MM-DD HH24:MI:SS"; }
		static const char * const PRINT_FORMAT() { return "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d"; }

		soe::uint16 mYear;
		soe::uint8 mMonth;
		soe::uint8 mDay;
		soe::uint8 mHour;
		soe::uint8 mMinute;
		soe::uint8 mSecond;
	};

	inline unsigned Read(const unsigned char * stream, unsigned size, Date & data, unsigned maxLen = 1, unsigned version = 0)
	{
        unsigned bytesTotal = 0;
		unsigned bytes = 0;

		bytesTotal += (bytes = Read(stream + bytesTotal, size - bytesTotal, data.mYear));
        if (!bytes) return 0;
		bytesTotal += (bytes = Read(stream + bytesTotal, size - bytesTotal, data.mMonth)); 
        if (!bytes) return 0;
		bytesTotal += (bytes = Read(stream + bytesTotal, size - bytesTotal, data.mDay)); 
        if (!bytes) return 0;
		bytesTotal += (bytes = Read(stream + bytesTotal, size - bytesTotal, data.mHour));
        if (!bytes) return 0;
		bytesTotal += (bytes = Read(stream + bytesTotal, size - bytesTotal, data.mMinute)); 
        if (!bytes) return 0;
		bytesTotal += (bytes = Read(stream + bytesTotal, size - bytesTotal, data.mSecond));
        if (!bytes) return 0;

		return bytesTotal;
	}

	inline unsigned Write(unsigned char * stream, unsigned size, const Date & data, unsigned version = 0)
	{
        unsigned bytesTotal = 0;
		unsigned bytes = 0;

		bytesTotal += (bytes = Write(stream + bytesTotal, size - bytesTotal, data.mYear));
        if (!bytes) return 0;
		bytesTotal += (bytes = Write(stream + bytesTotal, size - bytesTotal, data.mMonth));
        if (!bytes) return 0;
		bytesTotal += (bytes = Write(stream + bytesTotal, size - bytesTotal, data.mDay));
        if (!bytes) return 0;
		bytesTotal += (bytes = Write(stream + bytesTotal, size - bytesTotal, data.mHour));
        if (!bytes) return 0;
		bytesTotal += (bytes = Write(stream + bytesTotal, size - bytesTotal, data.mMinute));
        if (!bytes) return 0;
		bytesTotal += (bytes = Write(stream + bytesTotal, size - bytesTotal, data.mSecond));
        if (!bytes) return 0;

		return bytesTotal;
	}
#ifdef PRINTABLE_MESSAGES
	inline int Print(char * stream, unsigned size, const Date & data, unsigned maxDepth = INT_MAX)
	{
		return snprintf(stream, size, Date::PRINT_FORMAT(), data.mYear, data.mMonth, data.mDay, data.mHour, data.mMinute, data.mSecond);
	}
#endif
}

#endif

