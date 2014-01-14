// UnicodeArchive.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_UnicodeArchive_H
#define	_INCLUDED_UnicodeArchive_H

//-----------------------------------------------------------------------

#include "Unicode.h"

//----------------------------------------------------------------------

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, Unicode::String & target);
	void put(ByteStream & target,   const Unicode::String & source);

}// namespace Archive

//-----------------------------------------------------------------------

#endif	// _INCLUDED_UnicodeArchive_H
