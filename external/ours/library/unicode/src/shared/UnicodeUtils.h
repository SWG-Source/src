// ======================================================================
//
// UnicodeUtils.h
// copyright (c) 2001 Sony Online Entertainment
//
// jwatson
//
// Basic Unicode string handling/manipulating functions
// ======================================================================

#ifndef INCLUDED_UnicodeUtils_H
#define INCLUDED_UnicodeUtils_H

#if WIN32
// stl warning func not inlined
#pragma warning (disable:4710)
#pragma warning (disable:4786)
#endif

#include <cstring>
#include <vector>

#include "Unicode.h"
#include "utf8.h"

//-----------------------------------------------------------------

namespace Unicode
{
	typedef std::string UTF8String;
	typedef std::vector<Unicode::String> UnicodeStringVector;
	typedef std::vector<Unicode::NarrowString> UnicodeNarrowStringVector;

	String                     narrowToWide (const char * const nstr);
	String                     narrowToWide (const NarrowString & nstr);
	String &                   narrowToWide (const NarrowString & nstr, String & str); //lint !e1929 // function returning a reference
	String &                   narrowToWide (const char * const nstr,   String & str); //lint !e1929 // function returning a reference

	NarrowString               wideToNarrow (const String & nstr);
	NarrowString &             wideToNarrow (const String & nstr, NarrowString & str); //lint !e1929 // function returning a reference

	String                     utf8ToWide   (const UTF8String & nstr);
	String &                   utf8ToWide   (const UTF8String & nstr, String & str); //lint !e1929 // function returning a reference
	UTF8String                 wideToUTF8   (const String & nstr);
	UTF8String &               wideToUTF8   (const String & nstr, UTF8String & str); //lint !e1929 // function returning a reference

	NarrowString               toLower (const NarrowString & nstr);
	NarrowString               toUpper (const NarrowString & nstr);
	String                     toLower (const String & nstr);
	String                     toUpper (const String & nstr);

	float                      toFloat (const Unicode::String &str);
	int                        toInt (const Unicode::String &str);
	bool                       toBool (const Unicode::String &str);

	String                     intToWide(int value);
	void                       intToWide(int value, String & str);

	const String               getTrim        (const Unicode::String & str, const unicode_char_t * white = whitespace);
	String &                   trim           (Unicode::String & str, const unicode_char_t * white = whitespace);

	bool                       getFirstToken  (const Unicode::String & str, size_t pos, size_t & endpos, Unicode::String & token, const unicode_char_t * sepChars = whitespace);
	bool                       getNthToken    (const Unicode::String & str, const size_t n, size_t & pos, size_t & endpos, Unicode::String & token, const unicode_char_t * sepChars = whitespace);
	size_t                     skipWhitespace (const Unicode::String & str, size_t pos, const unicode_char_t * white = whitespace);

	const NarrowString         getTrim        (const Unicode::NarrowString & str, const char * white = ascii_whitespace);
	NarrowString &             trim           (Unicode::NarrowString & str, const char * white = ascii_whitespace);

	bool                       getFirstToken  (const Unicode::NarrowString & str, size_t pos, size_t & endpos, Unicode::NarrowString & token, const char * sepChars = ascii_whitespace);
	bool                       getNthToken    (const Unicode::NarrowString & str, const size_t n, size_t & pos, size_t & endpos, Unicode::NarrowString & token, const char * sepChars = ascii_whitespace);
	size_t                     skipWhitespace (const Unicode::NarrowString & str, size_t pos, const char * white = ascii_whitespace);

	bool                       tokenize (const Unicode::String & theStr, UnicodeStringVector & result, Unicode::String const * const delimiters = nullptr, Unicode::String const * const separators = nullptr);
	bool                       tokenize (const Unicode::String & theStr, UnicodeStringVector & result, const size_t tokenPos, size_t & whichToken, size_t & token_start, size_t & token_end, Unicode::String const * const delimiters = nullptr, Unicode::String const * const separators = nullptr);

	bool                       isUnicode (const Unicode::String & theStr);

	unsigned int               utf8Size(const Unicode::String & str);
	unsigned short             utf8CharSize(const Unicode::unicode_char_t value);
	void                       truncateToUTF8Size(Unicode::String & str, unsigned int size);

	enum FieldAlignment
	{
		FA_LEFT,
		FA_RIGHT,
		FA_CENTER
	};

	String &                   appendStringField (String & dst, const String & src, size_t width, FieldAlignment fa = FA_LEFT, unicode_char_t pad = ' ', bool truncate = false);
	String &                   appendStringField (String & dst, const NarrowString & src, size_t width, FieldAlignment fa = FA_LEFT, unicode_char_t pad = ' ', bool truncate = false);

	/**
	*  Compare substrings of str2 and str1, each starting with pos and containing n characters
	*/

	template <typename T> bool caseInsensitiveCompare (const String & str1, const T & str2, size_t pos, size_t n)
	{
		const size_t len1 = str1.size ();
		const size_t len2 = str2.size ();

		if (pos >= len1 || pos >= len2 || pos + n > len1 || pos + n > len2)
			return false;

		const size_t end = pos + n;
		for (size_t i = pos; i < end; ++i)
		{
			if (tolower (str1 [i]) != tolower (str2 [i]))
				return false;
		}

		return true;
	}

	/**
	*  Compare str1 and str2.
	*/

	template <typename T> bool caseInsensitiveCompare (const String & str1, const T & str2)
	{
		const size_t len1 = str1.size ();
		
		if (len1 != str2.size ())
			return false;

		return caseInsensitiveCompare (str1, str2, 0, len1);
	}

	/**
	* Optimized implementation of isWhitespace.  Must be kept in line with Unicode::whitespace array
	*/

	template <typename T> bool isWhitespace   (T c)
	{
		return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == 0x3000;
	}
	
	/*
	* @todo: uncomment when caseInsensitiveCompare matures
	*
	template <typename T> class CompareNoCasePredicate
	{
	public:
		bool operator()( T & a, T & b ) const
		{ 
			return caseInsensitiveCompare (a, b) < 0;
		};
	};

	template <typename T> class EqualsNoCasePredicate
	{
	public:
		bool operator()( T & a, T & b ) const
		{ 
			return caseInsensitiveCompare (a, b) == 0;
		};
	};

  */
}

//-----------------------------------------------------------------
//-- implementation
//-----------------------------------------------------------------

namespace Unicode
{
	//-----------------------------------------------------------------

	/**
	* Utility to convert a string and obtain the result by value
	*/	

	inline String narrowToWide (const char * const nstr)
	{
		String result;
		narrowToWide (nstr, result);
		return result;
	}

	//----------------------------------------------------------------------

	/**
	* Utility to convert a string and obtain the result by value
	*/
	
	inline String narrowToWide (const NarrowString & nstr)
	{
		String result;
		narrowToWide (nstr, result);
		return result;
	}
	
	//----------------------------------------------------------------------

	inline String & narrowToWide (const char * const nstr,   String & str)
	{
		std::basic_string<unsigned char> ucstr (nstr, nstr + strlen (nstr));
		return str.assign (ucstr.begin (), ucstr.end ());
	}

	//-----------------------------------------------------------------

	/**
	* Utility to convert a string and obtain the result by reference
	*/

	inline String & narrowToWide (const NarrowString & nstr, String & str)
	{ //lint !e1929 // function returning a reference
		std::basic_string<unsigned char> ucstr (nstr.begin (), nstr.end ());
		return str.assign (ucstr.begin (), ucstr.end ());
	}
	
	//-----------------------------------------------------------------

	/**
	* Utility to convert a string and obtain the result by value
	* This should only be used when the Unicode string is known to contain only 8 bit assignable values
	*/
	
	inline NarrowString wideToNarrow (const String & str)
	{
		return NarrowString (str.begin (), str.end ());
	}

	//-----------------------------------------------------------------

	/**
	* Utility to convert a string and obtain the result by reference
	* This should only be used when the Unicode string is known to contain only 8 bit assignable  values
	*/
	inline NarrowString &  wideToNarrow (const String & str, NarrowString & nstr)
	{ //lint !e1929 // function returning a reference
		return nstr.assign (str.begin (), str.end ());
	}

	/**
	* Get the trimmed version of str by value.
	*/

	inline const String getTrim (const Unicode::String & str, const unicode_char_t * white)
	{
		size_t const first_nonspace = str.find_first_not_of(white);

		if (first_nonspace == str.npos)
		{
			// There is only whitespace

			return String();
		}

		size_t const last_nonspace = str.find_last_not_of(white);

		if (last_nonspace == str.npos)
		{
			// There is only leading whitespace

			return str.substr(first_nonspace, last_nonspace);
		}

		// There is both leading and trailing whitespace

		return str.substr(first_nonspace, (last_nonspace - first_nonspace) + 1);
	}

	//-----------------------------------------------------------------

	/**
	* Trim the specified string and return a reference to it.
	*/

	inline String & trim (Unicode::String & str, const unicode_char_t * white)
	{
		return (str = getTrim (str, white));
	}

	/**
	* Get the trimmed version of str by value.
	*/

	inline const NarrowString getTrim (const Unicode::NarrowString & str, const char * white)
	{
		size_t const first_nonspace = str.find_first_not_of(white);

		if (first_nonspace == str.npos)
		{
			// There is only whitespace

			return "";
		}

		size_t const last_nonspace = str.find_last_not_of(white);

		if (last_nonspace == str.npos)
		{
			// There is only leading whitespace

			return str.substr(first_nonspace, last_nonspace);
		}

		// There is both leading and trailing whitespace

		return str.substr(first_nonspace, (last_nonspace - first_nonspace) + 1);
	}

	//-----------------------------------------------------------------

	/**
	* Trim the specified string and return a reference to it.
	*/
	
	inline NarrowString & trim (Unicode::NarrowString & str, const char * white)
	{
		return (str = getTrim (str, white));
	}

	/**
	* Return the first non-white position starting with pos.  returns str.npos if there is no non-white characer after pos
	*/

	inline size_t skipWhitespace (const Unicode::String & str, size_t pos, const unicode_char_t * white)
	{
		return str.find_first_not_of (white, pos);
	}

	//-----------------------------------------------------------------

	/**
	* Return the first non-white position starting with pos.  returns str.npos if there is no non-white characer after pos
	*/

	inline size_t skipWhitespace (const Unicode::NarrowString & str, size_t pos, const char * white)
	{
		return str.find_first_not_of (white, pos);
	}

	//-----------------------------------------------------------------

	/**
	* Append src to dst, padding the field as needed, and truncating the field if desired.
	*/

	inline String & appendStringField (String & dst, const NarrowString & src, size_t width, FieldAlignment fa, unicode_char_t pad, bool truncate)
	{
		return appendStringField (dst, narrowToWide (src), width, fa, pad, truncate);
	}

	//-----------------------------------------------------------------

	/**
	 * Return the size of the single character represented by the
	 * given two byte value if it were stored as UTF-8.
	 */
	inline unsigned short utf8CharSize(const Unicode::unicode_char_t value)
	{
		if (value < 0x80)
			return 1;
		else if (value < 0x800)
			return 2;
		else
			return 3;
	}
}

// ======================================================================


#endif


