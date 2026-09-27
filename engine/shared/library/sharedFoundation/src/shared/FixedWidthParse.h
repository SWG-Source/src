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
}

// ======================================================================

#endif
