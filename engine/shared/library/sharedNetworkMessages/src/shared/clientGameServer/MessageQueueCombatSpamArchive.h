//======================================================================
//
// MessageQueueCombatSpamArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueCombatSpamArchive_H
#define INCLUDED_MessageQueueCombatSpamArchive_H

#include "Archive/ByteStream.h"
#include "sharedFoundation/MessageQueue.h"

class MessageQueueCombatSpam;

//======================================================================

class MessageQueueCombatSpamArchive
{
public:

	static MessageQueue::Data *     get (Archive::ReadIterator & source);
	static void                     put (const MessageQueue::Data * source, Archive::ByteStream & target);
};

//======================================================================

#endif
