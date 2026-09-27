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
#include <string>

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

	// A 32-bit pattern such as a CRC, whose text may have been written as a
	// signed or as an unsigned decimal value. Accepts INT32_MIN..UINT32_MAX
	// and stores the value modulo 2^32, so "-1" and "4294967295" both give
	// 0xffffffff. Otherwise the same whole-text rules as above.
	inline bool parseBits32(char const * text, int base, uint32_t & value)
	{
		int32_t v = 0;
		if (parseInt32(text, base, v))
		{
			value = static_cast<uint32_t>(v);
			return true;
		}
		return parseUint32(text, base, value);
	}

	// ----------------------------------------------------------------------
	// Leading-integer parser, for text that carries a unit or other suffix
	// after the number ("49m"). strtol() syntax: returns false if the text
	// does not start with an integer or if the integer does not fit int32.
	// Whatever follows the integer is ignored.

	inline bool parseLeadingInt32(char const * text, int base, int32_t & value)
	{
		if (!text)
			return false;
		char * end = nullptr;
		int32_t v = 0;
		if (!toInt32(text, &end, base, v) || end == text)
			return false;
		value = v;
		return true;
	}

	// ----------------------------------------------------------------------
	// The same parsers for text held in a std::basic_string of any code unit
	// type, such as a std::string or a Unicode::String. Integer text is
	// ASCII: the whole-text parsers reject any other code unit and any
	// embedded nul, so narrowing a wide code unit cannot manufacture a digit
	// and a nul cannot hide the rest of the text. The leading-integer parser
	// only looks at the text before the first such code unit.

	namespace Detail
	{
		template <typename CharT>
		inline bool isAsciiCodeUnit(CharT c)
		{
			// A negative signed code unit converts to a large value, so it is
			// rejected too. (No <type_traits>: the client builds with STLport.)
			unsigned long const u = static_cast<unsigned long>(c);
			return u != 0 && u <= 0x7f;
		}

		// Returns false if text holds a code unit that is not ASCII or is nul.
		template <typename CharT, typename Traits, typename Alloc>
		inline bool toAsciiText(std::basic_string<CharT, Traits, Alloc> const & text, std::string & narrow)
		{
			narrow.clear();
			narrow.reserve(text.size());
			for (typename std::basic_string<CharT, Traits, Alloc>::const_iterator i = text.begin(); i != text.end(); ++i)
			{
				if (!isAsciiCodeUnit(*i))
					return false;
				narrow.push_back(static_cast<char>(*i));
			}
			return true;
		}

		// Copies the ASCII text before the first code unit that is not ASCII or is nul.
		template <typename CharT, typename Traits, typename Alloc>
		inline void toAsciiPrefix(std::basic_string<CharT, Traits, Alloc> const & text, std::string & narrow)
		{
			narrow.clear();
			for (typename std::basic_string<CharT, Traits, Alloc>::const_iterator i = text.begin(); i != text.end() && isAsciiCodeUnit(*i); ++i)
				narrow.push_back(static_cast<char>(*i));
		}
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline bool parseInt32(std::basic_string<CharT, Traits, Alloc> const & text, int base, int32_t & value)
	{
		std::string narrow;
		return Detail::toAsciiText(text, narrow) && parseInt32(narrow.c_str(), base, value);
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline bool parseUint32(std::basic_string<CharT, Traits, Alloc> const & text, int base, uint32_t & value)
	{
		std::string narrow;
		return Detail::toAsciiText(text, narrow) && parseUint32(narrow.c_str(), base, value);
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline bool parseBits32(std::basic_string<CharT, Traits, Alloc> const & text, int base, uint32_t & value)
	{
		std::string narrow;
		return Detail::toAsciiText(text, narrow) && parseBits32(narrow.c_str(), base, value);
	}

	template <typename CharT, typename Traits, typename Alloc>
	inline bool parseLeadingInt32(std::basic_string<CharT, Traits, Alloc> const & text, int base, int32_t & value)
	{
		std::string narrow;
		Detail::toAsciiPrefix(text, narrow);
		return parseLeadingInt32(narrow.c_str(), base, value);
	}
}

// ======================================================================

#endif
