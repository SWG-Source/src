// ======================================================================
//
// AiCreatureStateArchive.h
// copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiCreatureStateArchive_H
#define	INCLUDED_AiCreatureStateArchive_H

class AiCreatureStateMessage;

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, AiCreatureStateMessage & target);
	void put(ByteStream & target, AiCreatureStateMessage const & source);
};

// ======================================================================

#endif // INCLUDED_AiCreatureStateArchive_H
