//======================================================================
//
// MessageQueueShowFlyTextArchive.h
// Copyright (c) 2002 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#ifndef INCLUDED_MessageQueueShowFlyTextArchive_H
#define INCLUDED_MessageQueueShowFlyTextArchive_H

#include "sharedFoundation/MessageQueue.h"

//======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//----------------------------------------------------------------------

class MessageQueueShowFlyTextArchive
{
public:

	static MessageQueue::Data *get(Archive::ReadIterator &source);
	static void                put(const MessageQueue::Data *source, Archive::ByteStream &target);
};

//======================================================================

#endif
