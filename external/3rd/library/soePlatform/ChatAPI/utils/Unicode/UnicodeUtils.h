// ======================================================================
//
// UnicodeUtils.h
// copyright (c) 2001 Sony Online Entertainment
//
// jwatson
//
// Basic Unicode string handling/manipulating functions
// ======================================================================

#ifndef INCLUDED_PlatUnicodeUtils_H
#define INCLUDED_PlatUnicodeUtils_H

#if WIN32
// stl warning func not inlined
#pragma warning (disable:4710)
#pragma warning (disable:4786)
#endif

#include "Unicode.h"
#include <Base/Archive.h>
#include <ctype.h>

//-----------------------------------------------------------------

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE
{
#endif

	namespace Plat_Unicode
	{
		String                     narrowToWide(const NarrowString & nstr);
		String &                   narrowToWide(const NarrowString & nstr, String & str); //lint !e1929 // function returning a reference

		NarrowString               wideToNarrow(const String & nstr);
		NarrowString &             wideToNarrow(const String & nstr, NarrowString & str); //lint !e1929 // function returning a reference
		NarrowString               toLower(const NarrowString & nstr);
		NarrowString               toUpper(const NarrowString & nstr);
		String                     toLower(const String & nstr);
		String                     toUpper(const String & nstr);

		const String               getTrim(const String & str, const unicode_char_t * white = whitespace);
		String &                   trim(String & str, const unicode_char_t * white = whitespace);

		bool                       getFirstToken(const String & str, size_t pos, size_t & endpos, String & token, const unicode_char_t * sepChars = whitespace);
		bool                       getNthToken(const String & str, const size_t n, size_t & pos, size_t & endpos, String & token, const unicode_char_t * sepChars = whitespace);
		size_t                     skipWhitespace(const String & str, size_t pos, const unicode_char_t * white = whitespace);

		const NarrowString         getTrim(const NarrowString & str, const char * white = ascii_whitespace);
		NarrowString &             trim(NarrowString & str, const char * white = ascii_whitespace);

		bool                       getFirstToken(const NarrowString & str, size_t pos, size_t & endpos, NarrowString & token, const char * sepChars = ascii_whitespace);
		bool                       getNthToken(const NarrowString & str, const size_t n, size_t & pos, size_t & endpos, NarrowString & token, const char * sepChars = ascii_whitespace);
		size_t                     skipWhitespace(const NarrowString & str, size_t pos, const char * white = ascii_whitespace);

		enum FieldAlignment
		{
			FA_LEFT,
			FA_RIGHT,
			FA_CENTER
		};

		String &                   appendStringField(String & dst, const String & src, size_t width, FieldAlignment fa = FA_LEFT, unicode_char_t pad = ' ', bool truncate = false);
		String &                   appendStringField(String & dst, const NarrowString & src, size_t width, FieldAlignment fa = FA_LEFT, unicode_char_t pad = ' ', bool truncate = false);
		// ======================================================================

			//-----------------------------------------------------------------
			/**
			* Hacky code to correctly handle Cyrillic.
			*/

		inline unicode_char_t trueUpper(unicode_char_t letter)
		{
			if ((cyrillic_lower_first <= letter) && (letter <= cyrillic_lower_last)) {
				return letter + (cyrillic_upper_first - cyrillic_lower_first);
			}
			else {
				return static_cast<unicode_char_t>(toupper(letter));
			}
		}

		inline unicode_char_t trueLower(unicode_char_t letter)
		{
			if ((cyrillic_upper_first <= letter) && (letter <= cyrillic_upper_last)) {
				return letter - (cyrillic_upper_first - cyrillic_lower_first);
			}
			else {
				return static_cast<unicode_char_t>(tolower(letter));
			}
		}

		/**
		*  Compare substrings of str2 and str1, each starting with pos and containing n characters
		*/

		/**
		*  Compares str1 and str2, where str1 is a String and str2 is templated,
		*  thus could be a std::string as well.  Set reverseCompare to true if you
		*  want to begin comparison at end of string--a useful optimization if your
		*  strings tend to differ at the end.
		*/
		template <typename T> bool caseInsensitiveCompare(const String & str1, const T & str2, bool reverseCompare = false)
		{
			const size_t len1 = str1.size();
			const size_t len2 = str2.size();

			if (len1 != len2)
			{
				return false;
			}

			if (!reverseCompare)
			{
				for (size_t i = 0; i < len1; i++)
				{
					if (trueLower(str1[i]) != trueLower(str2[i]))
						return false;
				}
			}
			else
			{
				for (size_t i = len1; i > 0; i--)
				{
					if (trueLower(str1[i - 1]) != trueLower(str2[i - 1]))
						return false;
				}
			}

			return true;
		}

		/**
		*  Compares str1 and str2, where str1 is a String and str2 is templated,
		*  thus could be a std::string as well.  Unlike caseInsensitiveCompare, this
		*  version returns an int for < or > comparisons, and comparison must start
		*  at the front.  Note that this kind of comparison does not allow the shortcut
		*  of first comparing sizes--every character must be compared up to the last one.
		*/
		template <typename T> int caseInsensitiveCompareInt(const String & str1, const T & str2)
		{
			const size_t len1 = str1.size();
			const size_t len2 = str2.size();

			size_t len;
			if (len1 < len2)
				len = len1;
			else
				len = len2;

			// iterate over smallest length
			for (size_t i = 0; i < len; i++)
			{
				if (trueLower(str1[i]) < trueLower(str2[i]))
					return -1;
				else if (trueLower(str1[i]) > trueLower(str2[i]))
					return 1;
			}

			// Equal so far, thus: if len1 < len2, the result is less-than, else
			// if len1 = len2, the result is equal, else the result is greater-than.

			if (len1 < len2)
				return -1;
			else if (len1 == len2)
				return 0;
			else
				return 1;
		}

		/**
		* Optimized implementation of isWhitespace.  Must be kept in line with ::whitespace array
		*/

		template <typename T> bool isWhitespace(T c)
		{
			return c == ' ' || c == '\n' || c == '\r' || c == '\t';
		}

		template <typename T> class CompareNoCasePredicate
		{
		public:
			bool operator()(T & a, T & b) const
			{
				return caseInsensitiveCompare(a, b) < 0;
			};
		};

		template <typename T> class EqualsNoCasePredicate
		{
		public:
			bool operator()(T & a, T & b) const
			{
				return caseInsensitiveCompare(a, b) == 0;
			};
		};

		//-----------------------------------------------------------------
		//-- implementation
		//-----------------------------------------------------------------

			//-----------------------------------------------------------------
			/**
			* Utility to convert a string and obtain the result by value
			*/

		inline String          narrowToWide(const NarrowString & nstr)
		{
			return String(nstr.begin(), nstr.end());			// STL original
		}

		//-----------------------------------------------------------------
		/**
		* Utility to convert a string and obtain the result by reference
		*/

		inline String &        narrowToWide(const NarrowString & nstr, String & str)
		{
			return str.assign(nstr.begin(), nstr.end());		// STL original
		}

		//-----------------------------------------------------------------
		/**

		* Utility to convert a string and obtain the result by value

		* This should only be used when the Unicode string is known to contain only 8 bit assignable values

		*/

		inline NarrowString    wideToNarrow(const String & str)
		{
			return NarrowString(str.begin(), str.end());		// STL original
		}

		//-----------------------------------------------------------------
		/**

		* Utility to convert a string and obtain the result by reference

		* This should only be used when the Unicode string is known to contain only 8 bit assignable  values

		*/

		inline NarrowString &  wideToNarrow(const String & str, NarrowString & nstr)
		{
			return nstr.assign(str.begin(), str.end());		// STL original
		}

		/**
		* Get the trimmed version of str by value.
		*/

		inline const String getTrim(const String & str, const unicode_char_t * white)
		{
			const size_t first_nonspace = str.find_first_not_of(white);
			const size_t last_nonspace = str.find_last_not_of(white);
			return (first_nonspace == str.npos ? str : str.substr(first_nonspace, last_nonspace == str.npos ? last_nonspace : (last_nonspace - first_nonspace + 1)));
		}

		//-----------------------------------------------------------------
		/**
		* Trim the specified string and return a reference to it.
		*/

		inline String & trim(String & str, const unicode_char_t * white)
		{
			return (str = getTrim(str, white));
		}

		/**
		* Get the trimmed version of str by value.
		*/

		inline const NarrowString getTrim(const NarrowString & str, const char * white)
		{
			const size_t first_nonspace = str.find_first_not_of(white);
			const size_t last_nonspace = str.find_last_not_of(white);
			return (first_nonspace == str.npos ? str : str.substr(first_nonspace, last_nonspace == str.npos ? last_nonspace : (last_nonspace - first_nonspace + 1)));
		}

		//-----------------------------------------------------------------
		/**
		* Trim the specified string and return a reference to it.
		*/
		inline NarrowString & trim(NarrowString & str, const char * white)
		{
			return (str = getTrim(str, white));
		}
		/**
		* Return the first non-white position starting with pos.  returns str.npos if there is no non-white characer after pos
		*/
		inline size_t skipWhitespace(const String & str, size_t pos, const unicode_char_t * white)
		{
			return str.find_first_not_of(white, pos);
		}

		//-----------------------------------------------------------------
		/**
		* Return the first non-white position starting with pos.  returns str.npos if there is no non-white characer after pos
		*/

		inline size_t skipWhitespace(const NarrowString & str, size_t pos, const char * white)
		{
			return str.find_first_not_of(white, pos);
		}

		//-----------------------------------------------------------------
		/**
		* Append src to dst, padding the field as needed, and truncating the field if desired.
		*/

		inline String & appendStringField(String & dst, const NarrowString & src, size_t width, FieldAlignment fa, unicode_char_t pad, bool truncate)
		{
			return appendStringField(dst, narrowToWide(src), width, fa, pad, truncate);
		}

		// ======================================================================
	};

	// ======================================================================
	// Extensions to Base/Archive for String
	// ======================================================================
	//class Base::ByteStream;
	//-----------------------------------------------------------------------
	namespace Base
	{
		extern unsigned get(ByteStream::ReadIterator & source, Plat_Unicode::String & target);
		extern void put(ByteStream & target, const Plat_Unicode::String & source);

		//---------------------------------------------------------------------

		 // namespace Base
	};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif
