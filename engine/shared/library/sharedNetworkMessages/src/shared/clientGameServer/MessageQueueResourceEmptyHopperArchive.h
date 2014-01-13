//======================================================================
//
// MessageQueueResourceEmptyHopperArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueResourceEmptyHopperArchive_H
#define INCLUDED_MessageQueueResourceEmptyHopperArchive_H

#include "Archive/ByteStream.h"
#include "sharedFoundation/MessageQueue.h"

class MessageQueueResourceEmptyHopper;

//======================================================================

class MessageQueueResourceEmptyHopperArchive
{
public:

	static MessageQueue::Data *     get (Archive::ReadIterator & source);
	static void                     put (const MessageQueue::Data * source, Archive::ByteStream & target);
};

//======================================================================

#endif
