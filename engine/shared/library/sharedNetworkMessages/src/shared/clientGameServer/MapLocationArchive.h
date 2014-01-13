// MapLocationArchive.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_MapLocationArchive_H
#define	_INCLUDED_MapLocationArchive_H

//-----------------------------------------------------------------------

#include "Archive/ByteStream.h"
#include "sharedNetworkMessages/MapLocation.h"

//-----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator &source, MapLocation &target);
	void put (ByteStream &target, const MapLocation &source);
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MapLocationArchive_H
