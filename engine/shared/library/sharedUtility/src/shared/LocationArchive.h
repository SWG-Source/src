// LocationArchive.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_LocationArchive_H
#define	_INCLUDED_LocationArchive_H

//-----------------------------------------------------------------------

class Location;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (Archive::ReadIterator & source, Location & target);
	void put (ByteStream & target, const Location & source);
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_LocationArchive_H
