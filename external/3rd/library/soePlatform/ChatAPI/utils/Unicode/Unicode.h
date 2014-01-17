// ======================================================================
// Unicode.h
// copyright (c) 2001 Sony Online Entertainment
//
// jwatson
//
// Basic Unicode primitives and string handling/manipulating functions
// ======================================================================

#ifndef INCLUDED_PlatUnicode_H
#define INCLUDED_PlatUnicode_H

#if WIN32
#pragma warning (disable:4710)
#pragma warning (disable:4786)
// stl warning func not inlined
#pragma warning (disable:4514)
#endif

#include <string>

//-----------------------------------------------------------------

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

namespace Plat_Unicode 
{

	typedef unsigned short unicode_char_t;

	/**
	* Standard Unicode string is UTF-16
	*/
	typedef std::basic_string<unicode_char_t> String;

	/**
	* NarrowString is a ascii string.
	*/

	typedef std::string                        NarrowString;

	const unicode_char_t                       whitespace []       = { ' ', '\n', '\r', '\t', 0 };
	const unicode_char_t                       endlines []         = { '\r', '\n', 0 };
	const char                                 ascii_whitespace [] = { ' ', '\n', '\r', '\t', 0 };
	const char                                 ascii_endlines []   = { '\r', '\n', 0 };

	const unicode_char_t						cyrillic_lower_first	= 0x0410;
	const unicode_char_t						cyrillic_lower_last		= 0x042F;
	const unicode_char_t						cyrillic_upper_first	= 0x0430;
	const unicode_char_t						cyrillic_upper_last		= 0x044F;
}; 
#ifdef EXTERNAL_DISTRO
};
#endif 

//-----------------------------------------------------------------

#endif

