// ======================================================================
// Unicode.h
// copyright (c) 2001 Sony Online Entertainment
//
// jwatson
//
// Basic Unicode primitives and string handling/manipulating functions
// ======================================================================

#ifndef INCLUDED_Unicode_H
#define INCLUDED_Unicode_H

#if WIN32
#pragma warning (disable:4710)
#pragma warning (disable:4786)
// stl warning func not inlined
#pragma warning (disable:4514)
#endif

#include <string>
#include <unordered_map>

//-----------------------------------------------------------------

namespace Unicode
{

	typedef unsigned short unicode_char_t;

	/**
	* Standard Unicode string is UTF-16
	*/
	typedef std::basic_string <unicode_char_t, std::char_traits<unicode_char_t>, std::allocator<unicode_char_t> > String;

	/**
	* NarrowString is a ascii string.
	*/

	typedef std::string                        NarrowString;

	const unicode_char_t                       whitespace []       = { ' ', '\n', '\r', '\t', 0x3000, 0 };
	const unicode_char_t                       endlines []         = { '\r', '\n', 0 };
	const char                                 ascii_whitespace [] = { ' ', '\n', '\r', '\t', 0 };
	const char                                 ascii_endlines []   = { '\r', '\n', 0 };

	const String                               emptyString;
}

namespace std {

	template<>
	struct hash<Unicode::String>
	{
		std::size_t operator()(const Unicode::String& s) const
		{
			std::size_t hash = 0;
			int offset = 'a' - 1;
			for(Unicode::String::const_iterator it=s.begin(); it!=s.end(); ++it) {
  				hash = hash << 1 | (*it - offset);
			}

			return hash;
		}
	};

}

//-----------------------------------------------------------------

#endif

