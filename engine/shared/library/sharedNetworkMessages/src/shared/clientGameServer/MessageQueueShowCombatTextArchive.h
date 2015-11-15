//======================================================================
//
// MessageQueueShowCombatTextArchive.h
// Copyright (c) 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#ifndef INCLUDED_MessageQueueShowCombatTextArchive_H
#define INCLUDED_MessageQueueShowCombatTextArchive_H

#include "sharedFoundation/MessageQueue.h"

//======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//----------------------------------------------------------------------

class MessageQueueShowCombatTextArchive
{
public:

	static MessageQueue::Data *get(Archive::ReadIterator &source);
	static void                put(const MessageQueue::Data *source, Archive::ByteStream &target);
};

//======================================================================

#endif
