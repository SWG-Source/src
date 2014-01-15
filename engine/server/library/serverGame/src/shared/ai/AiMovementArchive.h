// ======================================================================
//
// AiMovementArchive.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementArchive_H
#define	INCLUDED_AiMovementArchive_H

class AiMovementMessage;

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, AiMovementMessage & target);
	void put(ByteStream & target, const AiMovementMessage & source);
};


// ======================================================================


#endif

