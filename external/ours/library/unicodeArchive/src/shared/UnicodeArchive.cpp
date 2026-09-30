//======================================================================
//
// UnicodeArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "unicodeArchive/FirstUnicodeArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "Archive/Archive.h"
#include "Archive/ArchiveCount.h"

//======================================================================

namespace Archive
{
	//-----------------------------------------------------------------------
	void get(ReadIterator & source, Unicode::String & target)
	{
		unsigned int size = 0;
		Archive::get (source, size);
		
		if (size > source.getSize() / sizeof(Unicode::unicode_char_t))
			throw ReadException("Archive::get(Unicode::String) - payload exceeds remaining buffer");

		// Own aligned, contiguous code units instead of dereferencing an unaligned packet.
		Unicode::String decoded(size, Unicode::unicode_char_t(0));
		if (size)
		{
			unsigned int const readSize = size * static_cast<unsigned int>(sizeof(Unicode::unicode_char_t));
			source.get(&decoded[0], readSize);
		}
		target.swap(decoded);
	}
	
	//-----------------------------------------------------------------------
	
	void put(ByteStream & target, const Unicode::String & source)
	{
		const unsigned int size = ArchiveCount::fromSize<unsigned int>(source.size ());
		Archive::put (target, size);
		target.put (source.data(), size * sizeof (Unicode::unicode_char_t));
	}
}
	
//======================================================================
