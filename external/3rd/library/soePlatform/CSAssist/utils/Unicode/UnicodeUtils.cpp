// ======================================================================
//
// Unicode.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstUnicode.h"
#include "UnicodeUtils.h"

#include <algorithm>

#if defined (linux)
#include <ctype.h>
#endif

// ======================================================================

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

namespace Plat_Unicode 
{

	//-----------------------------------------------------------------

	/**
	* Return by value a lowered version of nstr
	*/

	NarrowString    toLower (const NarrowString & nstr)
	{
		NarrowString retval (nstr.c_str());
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

	/**
	* Return by value a lowered version of str.
	* This currently only works properly with ASCII.
	* @todo: implement this for all unicode blocks.
	*/

	String    toLower (const String & nstr)
	{
		String retval (nstr);
		std::transform (retval.begin (), retval.end (), retval.begin (), tolower);
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
		std::transform (retval.begin (), retval.end (), retval.begin (), toupper);
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

	/**
	* Find the first token in str, starting at pos.  The value of the token is placed in the token field.  endpos is updated to point past the token.
	*/

	bool getFirstToken (const String & str, size_t pos, size_t & endpos, String & token, const unicode_char_t * sepChars)
	{
		const size_t first_nonspace = str.find_first_not_of ( sepChars, pos );

		if (first_nonspace != std::string::npos)
		{
			const size_t first_space    = str.find_first_of   (sepChars, first_nonspace );

			if (first_space != std::string::npos)
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

	bool getFirstToken (const NarrowString & str, size_t pos, size_t & endpos, NarrowString & token, const char * sepChars)
	{
		const size_t first_nonspace = str.find_first_not_of ( sepChars, pos );

		if (first_nonspace != std::string::npos)
		{
			const size_t first_space    = str.find_first_of   (sepChars, first_nonspace );
			if (first_space != std::string::npos)
				token = str.substr (first_nonspace, first_space - first_nonspace);
			else
				token = str.substr (first_nonspace);

			endpos = first_space;
			return true;
		}
		else
			return false;
	}

	//-----------------------------------------------------------------

	/**
	* Find the nth token in str, starting at pos.  The value of the token is placed in the token field.  endpos is updated to point past the token.
	* pos is updated to point to the first character of token in str.
	*/

	bool getNthToken (const String & str, const size_t n, size_t & pos, size_t & endpos, String & token, const unicode_char_t * sepChars)
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
				if (space == std::string::npos)
					return false;
				//now find the next non-whitespace character after it
				desired_first_nonspace = str.find_first_not_of( sepChars, space );
				if (desired_first_nonspace == std::string::npos)
					return false;
				pos = desired_first_nonspace;
			}
		}
		//find the end of the token
		const size_t first_space    = str.find_first_of   (sepChars, desired_first_nonspace );
		//now get that token
		if (first_space != std::string::npos)
			token = str.substr (desired_first_nonspace, first_space - desired_first_nonspace);
		else
			token = str.substr (desired_first_nonspace);

		endpos = desired_first_nonspace;

		return true;
	}

	//-----------------------------------------------------------------

	/**
	* Find the nth token in str, starting at pos.  The value of the token is placed in the token field.  endpos is updated to point past the token.
	* pos is updated to point to the first character of token in str.
	*/
	
	bool getNthToken (const NarrowString & str, const size_t n, size_t & pos, size_t & endpos, NarrowString & token, const char * sepChars)
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
				if (space == std::string::npos)
					return false;
				//now find the next non-whitespace character after it
				desired_first_nonspace = str.find_first_not_of( sepChars, space );
				if (desired_first_nonspace == std::string::npos)
					return false;
				pos = desired_first_nonspace;
			}
		}
		//find the end of the token
		const size_t first_space    = str.find_first_of   (sepChars, desired_first_nonspace );
		//now get that token
		if (first_space != std::string::npos)
			token = str.substr (desired_first_nonspace, first_space - desired_first_nonspace);
		else
			token = str.substr (desired_first_nonspace);

		endpos = desired_first_nonspace;

		return true;
	}
};
// ======================================================================
// Extensions to Base/Archive for String
// ======================================================================

namespace Base
{
void get(Base::ByteStream::ReadIterator& source, Plat_Unicode::String& target)
{
        unsigned int size = 0;
        get (source, size);

        const unsigned char * const buf            = source.getBuffer();
        const Plat_Unicode::unicode_char_t * const ubuf = reinterpret_cast<const Plat_Unicode::unicode_char_t *>(buf);

        target.assign (ubuf, ubuf + size);

        const unsigned int readSize                = size * sizeof (Plat_Unicode::unicode_char_t);
        source.advance(readSize);
//		printf("%d\n", target.size());
}

void put(Base::ByteStream& target, const Plat_Unicode::String& source)
{
  const unsigned int size = source.size ();
  put (target, size);
  target.put (source.data(), size * sizeof (Plat_Unicode::unicode_char_t));
}

};


#ifdef EXTERNAL_DISTRO
};
#endif 
