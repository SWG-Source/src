// ======================================================================
//
// Unicode.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstUnicode.h"
#include "UnicodeUtils.h"

#include <ctype.h>
#include <algorithm>
#include <vector>


// ======================================================================

namespace Unicode
{

	//-----------------------------------------------------------------

	String utf8ToWide (const UTF8String & nstr)
	{
		String result;
		utf8ToWide (nstr, result);
		return result;
	}

	String & utf8ToWide (const UTF8String & nstr, String & str)
	{
		str.clear();
		const UTF8String::const_iterator end = nstr.end();
		for (UTF8String::const_iterator from = nstr.begin(); from != end; ++from)
		{
			char first = *from;
			const int len = UTF8_charSize(&first);
			// if 1-byte, do 1-byte pad
			if (len == 1)
			{
				str += static_cast<Unicode::unicode_char_t>(first);
			}
			// else if 3-byte conversion
			else if (len == 3)
			{
				++from;
				if (from != end &&
					*from != 0)
				{
					const char second = *from++;
					if (from != end &&
						*from != 0)
					{
						str += static_cast<Unicode::unicode_char_t>((static_cast<Unicode::unicode_char_t>( first & 0x0F ) << 12 ) +
							                                        (static_cast<Unicode::unicode_char_t>( second & 0x3F ) << 6 ) +
														            (( *from & 0x3F )));
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			// else if 2-byte conversion
			else
			{
				++from;
				// Make sure we didn't reach the end of a string in the middle of a character
				if (from != end &&
					*from != 0)
				{
					str += static_cast<Unicode::unicode_char_t>((static_cast<Unicode::unicode_char_t>(first & 0x1F) << 6 ) +
						                                        (*from &0x3F));
				}
				else
				{
					break;
				}
			}
		}

		return str;
	}


	/**
	 * Convert a Unicode string to UTF8 format.
	 */
	UTF8String & wideToUTF8(const String & nstr, UTF8String & str)
	{
		str.clear();
		str.reserve(nstr.size() * 3 + 2);
		for (String::const_iterator from = nstr.begin(); from != nstr.end(); ++from)
		{
			if (*from >= 0x0001 && *from <= 0x007f)
			{
				// 1-byte packing
				str += static_cast<char>(((*from) & 0x007f));
			}
			else if (*from == 0x0000)
			{
				// nullptr character
				str += static_cast<char>(0x0000);
			}
			else if (*from >= 0x0800)
			{
				// 3-byte packing
				str += static_cast<char>(0x00e0 | (((*from) >> 12) & 0x0f));
				str += static_cast<char>(0x0080 | (((*from) >> 6) & 0x3f));
				str += static_cast<char>(0x0080 | ((*from) & 0x3f));
			}
			else
			{
				// 2-byte packing
				str += static_cast<char>(0x00c0 | (((*from) >> 6) & 0x1f));
				str += static_cast<char>(0x0080 | ((*from) & 0x3f));
			}
		}
		return str;
	}

	UTF8String wideToUTF8 (const String & nstr)
	{
		UTF8String result;
		wideToUTF8 (nstr, result);
		return result;
	}

	/**
	* Return by value a lowered version of nstr
	*/

	NarrowString    toLower (const NarrowString & nstr)
	{
		NarrowString retval (nstr);
		std::transform (retval.begin (), retval.end (), retval.begin (), tolower);
		return retval;
	}

	//-----------------------------------------------------------------

	/**
	* Return by value an uppered version of nstr
	*/

	NarrowString    toUpper (const NarrowString & nstr)
	{
		NarrowString retval (nstr);
		std::transform (retval.begin (), retval.end (), retval.begin (), toupper);
		return retval;
	}

	//-----------------------------------------------------------------

	wchar_t toLowerWide (wchar_t inp)
	{
		if((inp >= wchar_t('A')) && (inp <= wchar_t('Z')))
			return static_cast<wchar_t>((inp - wchar_t('A') + wchar_t('a')));
		return inp;
	}
	
	//-----------------------------------------------------------------

	wchar_t toUpperWide (wchar_t inp)
	{
		if((inp >= wchar_t('a')) && (inp <= wchar_t('z')))
			return static_cast<wchar_t>((inp - wchar_t('a') + wchar_t('A')));
		return inp;
	}

	/**
	* Return by value a lowered version of str.
	* This currently only works properly with ASCII.
	* @todo: implement this for all unicode blocks.
	*/

	String    toLower (const String & nstr)
	{
		String retval (nstr);
		std::transform (retval.begin (), retval.end (), retval.begin (), toLowerWide);
		return retval;
	}

	//-----------------------------------------------------------------

	/**
	* Return by value an uppered version of str.
	* This currently only works properly with ASCII.
	* @todo: implement this for all unicode blocks.
	*/

	String    toUpper (const String & nstr)
	{
		String retval (nstr);
		std::transform (retval.begin (), retval.end (), retval.begin (), toUpperWide);
		return retval;
	} 

	//-----------------------------------------------------------------

	/**
	* Append src to dst, padding the field as needed, and truncating the field if desired.
	*/

	String & appendStringField (String & dst, const String & src, size_t width, FieldAlignment fa, unicode_char_t pad, bool truncate)
	{

		if (src.length () > width && truncate)
			return dst.append (src.substr (0, width));
		
		if (src.length () >= width)
			return dst.append (src);

		const size_t diff = width - src.length ();

		if (fa == FA_RIGHT)
			dst.append (diff, pad);
		else if (fa == FA_CENTER)
			dst.append (diff/2, pad);

		dst.append (src);

		if (fa == FA_LEFT)
			dst.append (diff, pad);
		else if (fa == FA_CENTER)
			dst.append (diff - diff/2, pad);
		
		return dst;
	}

	//-----------------------------------------------------------------

	template <typename T, typename U> bool getFirstTokenTemplate (const T & str, size_t pos, size_t & endpos, T & token, const U * sepChars)
	{
		const size_t first_nonspace = str.find_first_not_of ( sepChars, pos );

		if (first_nonspace != str.npos)
		{
			const size_t first_space    = str.find_first_of   (sepChars, first_nonspace );

			if (first_space != str.npos)
				token = str.substr (first_nonspace, first_space - first_nonspace);
			else
				token = str.substr (first_nonspace);

			endpos = first_space;
			return true;
		}

		return false;
	}

	//-----------------------------------------------------------------

	/**
	* Find the first token in str, starting at pos.  The value of the token is placed in the token field.  endpos is updated to point past the token.
	*/

	bool getFirstToken (const Unicode::String & str, size_t pos, size_t & endpos, Unicode::String & token, const unicode_char_t * sepChars)
	{
		return getFirstTokenTemplate (str, pos, endpos, token, sepChars);
	}

	//-----------------------------------------------------------------

	/**
	* Find the first token in str, starting at pos.  The value of the token is placed in the token field.  endpos is updated to point past the token.
	*/

	bool getFirstToken (const Unicode::NarrowString & str, size_t pos, size_t & endpos, Unicode::NarrowString & token, const char * sepChars)
	{
		return getFirstTokenTemplate (str, pos, endpos, token, sepChars);
	}

	//-----------------------------------------------------------------

	/**
	* Find the nth token in str, starting at pos.  The value of the token is placed in the token field.  endpos is updated to point past the token.
	* pos is updated to point to the first character of token in str.
	*/

	template <typename T, typename U> bool getNthTokenTemplate (const T & str, const size_t n, size_t & pos, size_t & endpos, T & token, const U * sepChars)
	{
		size_t desired_first_nonspace = 0;
		size_t space = 0;

		//check for trivial case
		if(n == 0)
			desired_first_nonspace = pos;
		else
		{
			for (size_t i = 0; i < n; ++i)
			{
				//find the first whitespace character
				space = str.find_first_of ( sepChars, pos );
				if (space == str.npos)
					return false;
				//now find the next non-whitespace character after it
				desired_first_nonspace = str.find_first_not_of( sepChars, space );
				if (desired_first_nonspace == str.npos)
					return false;
				pos = desired_first_nonspace;
			}
		}
		//find the end of the token
		const size_t first_space    = str.find_first_of   (sepChars, desired_first_nonspace );
		//now get that token
		if (first_space != str.npos)
			token = str.substr (desired_first_nonspace, first_space - desired_first_nonspace);
		else
			token = str.substr (desired_first_nonspace);

		endpos = first_space;

		if (pos == endpos)
			return false;

		return true;
	}

	//----------------------------------------------------------------------

	/**
	* Find the nth token in str, starting at pos.  The value of the token is placed in the token field.  endpos is updated to point past the token.
	* pos is updated to point to the first character of token in str.
	*/

	bool getNthToken (const Unicode::String & str, const size_t n, size_t & pos, size_t & endpos, Unicode::String & token, const unicode_char_t * sepChars)
	{
		return getNthTokenTemplate (str, n, pos, endpos, token, sepChars);
	}

	//-----------------------------------------------------------------

	/**
	* Find the nth token in str, starting at pos.  The value of the token is placed in the token field.  endpos is updated to point past the token.
	* pos is updated to point to the first character of token in str.
	*/
	
	bool getNthToken (const Unicode::NarrowString & str, const size_t n, size_t & pos, size_t & endpos, Unicode::NarrowString & token, const char * sepChars)
	{
		return getNthTokenTemplate (str, n, pos, endpos, token, sepChars);
	}

	//-----------------------------------------------------------------

	bool tokenize (const Unicode::String & theStr, UnicodeStringVector & result, Unicode::String const * const delimiters, Unicode::String const * const separators)
	{
		size_t tokenPos = 0;
		size_t whichToken = 0;
		size_t token_start = 0;
		size_t token_end = 0;

		return tokenize(theStr, result, tokenPos, whichToken, token_start, token_end, delimiters, separators);
	}

	//-----------------------------------------------------------------


	bool tokenize (const Unicode::String & theStr, UnicodeStringVector & result, const size_t tokenPos, size_t & whichToken, size_t & token_start, size_t & token_end, Unicode::String const * const delimiters, Unicode::String const * const separators)
	{
		size_t end_pos = 0;
		size_t start_pos = 0;
		whichToken = 0;

		result.clear ();

		Unicode::String str = Unicode::getTrim (theStr);

		if(str.empty())
			return false;

		//-- skip the command character
		if (str [0] == '/')
			str.erase (0, 1);

		//-- Setup the separator marker.
		size_t sep_pos = static_cast<size_t>(str.npos);

		for (;;)
		{
			if (end_pos >= str.size ())
				break;

			if(separators)
			{
				sep_pos = str.find_first_of(*separators, end_pos);
			}

			if(delimiters)
			{
				start_pos = str.find_first_not_of(*delimiters, end_pos);
			}
			else
			{
				start_pos = str.find_first_not_of (' ', end_pos);
			}

			if (start_pos == str.npos)
				break;

			bool separatorFound = false;
			if(sep_pos != str.npos)
			{
				if(sep_pos <= start_pos || start_pos == str.npos)
				{
					start_pos = sep_pos;
					end_pos = start_pos + 1;

					if (end_pos >= str.size())
						end_pos = str.size() - 1;

					separatorFound = true;
				}
			}
			

			//----------------------------------------------------------------------

			bool hasQuotes = false;

			if (!separatorFound)
			{
				if (str [start_pos] == '\"')
				{
					if (++start_pos >= str.size ())
						break;
					end_pos = str.find_first_of ('\"', start_pos);
					hasQuotes = true;
				}
				else
				{
					if(delimiters)
					{
						end_pos = str.find_first_of(*delimiters, start_pos);
					}
					else
					{
						end_pos = str.find_first_of (' ', start_pos);
					}

					if(sep_pos != str.npos && sep_pos > 0)
					{
						if(sep_pos < end_pos)
						{
							end_pos = sep_pos;
						}
					}
				}
			}
		
			//----------------------------------------------------------------------
						
			if (start_pos == end_pos)
			{
				if (hasQuotes)
				{
					// Empty string "", which is a valid token

					result.push_back (Unicode::String());
					++end_pos;
					continue;
				}

				break;
			}

			if (start_pos <= tokenPos)
			{
				whichToken  = result.size ();
				token_start = start_pos;
				token_end   = end_pos;
			}
			
			if (end_pos == str.npos)
			{
				result.push_back (str.substr (start_pos));
				break;
			}
			else
			{
				result.push_back (str.substr (start_pos, end_pos - start_pos));
				if (hasQuotes)
					++end_pos;
			}

			 ++start_pos;
		}
		
		return true;
	}

	//-----------------------------------------------------------------

	float toFloat (const Unicode::String &str)
	{
		float result = 0.0f;

		if (!str.empty())
		{
			result = static_cast<float>(atof(Unicode::wideToNarrow(str).c_str()));
		}

		return result;
	}

	//-----------------------------------------------------------------

	int toInt (const Unicode::String &str)
	{
		int result = 0;

		if (!str.empty())
		{
			result = atoi(Unicode::wideToNarrow(str).c_str());
		}

		return result;
	}

	//-----------------------------------------------------------------

	bool toBool (const Unicode::String &str)
	{
		bool result = 0;

		if (!str.empty())
		{
			result = (atoi(Unicode::wideToNarrow(str).c_str()) != 0);
		}

		return result;
	}

	//-----------------------------------------------------------------

	Unicode::String intToWide(int value)
	{
		Unicode::String str;
		Unicode::intToWide(value, str);
		return str;
	}

	//-----------------------------------------------------------------

	void intToWide(int value, Unicode::String & str)
	{
		char buffer[16];
		sprintf(buffer, "%d", value);
		str = Unicode::narrowToWide(buffer);
	}

	//-----------------------------------------------------------------

	bool isUnicode (const Unicode::String & theStr)
	{
		for(Unicode::String::const_iterator i = theStr.begin(); i != theStr.end(); ++i)
		{
			if(*i > 255)
			{
				return true;
			}
		}
		return false;
	}

	//-----------------------------------------------------------------

	/**
	 * Return the size of the string if it were stored as UTF-8.  Useful
	 * since we store all Unicode values as UTF-8 in the DB by handling
	 * the conversion ourselves instead of letting Oracle handle it.
	 */
	unsigned int utf8Size(const Unicode::String & str)
	{
		unsigned int count = 0;

		for (String::const_iterator from = str.begin(); from != str.end(); ++from)
		{
			Unicode::unicode_char_t value = (*from);
			count += utf8CharSize(value);
		}
		
		return count;
	}

	//-----------------------------------------------------------------

	/**
	 * Set str to be a valid string after being truncated to size
	 * if str were stored as UTF-8.  This means that we want to
	 * convert str to UTF-8, truncate to size, then throw out any
	 * extra bytes left on the end by a UTF-8 (multi-byte) value
	 * being truncated in the middle of its encoding.  We take
	 * advantage of Unicode::utf8ToWide() guaranteeing that the
	 * extra bytes are thrown out.
	 */
	void truncateToUTF8Size(Unicode::String & str, unsigned int size)
	{
		if (size < utf8Size(str))
			str = Unicode::utf8ToWide(Unicode::wideToUTF8(str).substr(0, size));
	}

}

// ======================================================================
