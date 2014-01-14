// StringIdArchive.h
// copyright 2001 Verant Interactive

#ifndef	_INCLUDED_StringIdArchive_H
#define	_INCLUDED_StringIdArchive_H

//-----------------------------------------------------------------------

class StringId;

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//-----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, StringId & target);

	void put(ByteStream & target, const StringId & source);
}

//-----------------------------------------------------------------------

#endif
