// ======================================================================
//
// FixedWidthParse.h
//
// Parses the text of a 32-bit integer field, such as a datatable cell or a
// template parameter, into exactly that width on every platform. The
// result does not depend on the width of long: text whose value does not
// fit the field is rejected rather than clamped or wrapped. errno is left
// unchanged.
//
// ======================================================================

#ifndef INCLUDED_FixedWidthParse_H
#define INCLUDED_FixedWidthParse_H

// ======================================================================

#include <cerrno>
#include <cstdint>
#include <cstdlib>

// ======================================================================

namespace FixedWidthParse
{
	// strtol() syntax. Returns false if the value is outside the int32 range.
	inline bool toInt32(char const * text, char ** end, int base, int32_t & value)
	{
		int const savedErrno = errno;
		errno = 0;
		long long const v = strtoll(text, end, base);
		bool const inRange = (errno != ERANGE) && (v >= INT32_MIN) && (v <= INT32_MAX);
		errno = savedErrno;
		if (inRange)
			value = static_cast<int32_t>(v);
		return inRange;
	}

	// strtoul() syntax without a sign. Returns false if the text is negative
	// or the value is above UINT32_MAX.
	inline bool toUint32(char const * text, char ** end, int base, uint32_t & value)
	{
		char const * p = text;
		while (*p == ' ' || (*p >= '\t' && *p <= '\r'))
			++p;
		if (*p == '-')
		{
			if (end)
				*end = const_cast<char *>(text);
			return false;
		}
		int const savedErrno = errno;
		errno = 0;
		unsigned long long const v = strtoull(text, end, base);
		bool const inRange = (errno != ERANGE) && (v <= UINT32_MAX);
		errno = savedErrno;
		if (inRange)
			value = static_cast<uint32_t>(v);
		return inRange;
	}

	// ----------------------------------------------------------------------
	// Whole-text parsers. The entire text, optionally surrounded by
	// whitespace, must be exactly one integer that fits the destination.
	// Anything else (empty text, trailing characters, out-of-range values,
	// a sign on an unsigned value) returns false and leaves value unchanged.

	namespace Detail
	{
		// True if only whitespace remains.
		inline bool isEndOfText(char const * end)
		{
			while (*end == ' ' || (*end >= '\t' && *end <= '\r'))
				++end;
			return *end == '\0';
		}
	}

	inline bool parseInt32(char const * text, int base, int32_t & value)
	{
		if (!text)
			return false;
		char * end = nullptr;
		int32_t v = 0;
		if (!toInt32(text, &end, base, v) || end == text || !Detail::isEndOfText(end))
			return false;
		value = v;
		return true;
	}

	inline bool parseUint32(char const * text, int base, uint32_t & value)
	{
		if (!text)
			return false;
		char * end = nullptr;
		uint32_t v = 0;
		if (!toUint32(text, &end, base, v) || end == text || !Detail::isEndOfText(end))
			return false;
		value = v;
		return true;
	}
}

// ======================================================================

#endif
