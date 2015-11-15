//======================================================================
//
// AuctionTokenArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AuctionTokenArchive_H
#define INCLUDED_AuctionTokenArchive_H

//======================================================================

class AuctionToken;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (Archive::ReadIterator & source, AuctionToken & target);
	void put (ByteStream & target, const AuctionToken & source);
};

//======================================================================

#endif
