//======================================================================
//
// UnicodeArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "unicodeArchive/FirstUnicodeArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "Archive/Archive.h"

//======================================================================

namespace Archive
{
	//-----------------------------------------------------------------------
	void get(ReadIterator & source, Unicode::String & target)
	{
		unsigned int size = 0;
		Archive::get (source, size);
		
		const unsigned char * const buf            = source.getBuffer();
		const Unicode::unicode_char_t * const ubuf = reinterpret_cast<const Unicode::unicode_char_t *>(buf);
		
		target.assign (ubuf, ubuf + size);
		
		const unsigned int readSize                = size * sizeof (Unicode::unicode_char_t);
		source.advance(readSize);
	}
	
	//-----------------------------------------------------------------------
	
	void put(ByteStream & target, const Unicode::String & source)
	{
		const unsigned int size = source.size ();
		Archive::put (target, size);
		target.put (source.data(), size * sizeof (Unicode::unicode_char_t));
	}
}
	
//======================================================================
