// ======================================================================
//
// NetworkIdArchive.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_NetworkIdArchive_H
#define	INCLUDED_NetworkIdArchive_H

// ======================================================================

class NetworkId;

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, uint64 & target);
	void put(ByteStream & target, const uint64 & source);
	void get(ReadIterator & source, int64 & target);
	void put(ByteStream & target, const int64 & source);
	void get(ReadIterator & source, NetworkId & target);
	void put(ByteStream & target, const NetworkId & source);
}

// ======================================================================

#endif	
