//======================================================================
//
// StartingLocationDataArchive.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_StartingLocationDataArchive_H
#define INCLUDED_StartingLocationDataArchive_H

//======================================================================

class StartingLocationData;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (Archive::ReadIterator & source, StartingLocationData & target);
	void put (ByteStream & target, const StartingLocationData & source);
};

//======================================================================

#endif
