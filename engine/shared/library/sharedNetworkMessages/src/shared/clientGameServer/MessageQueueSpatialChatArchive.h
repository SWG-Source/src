//======================================================================
//
// MessageQueueSpatialChatArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueSpatialChatArchive_H
#define INCLUDED_MessageQueueSpatialChatArchive_H

#include "sharedFoundation/MessageQueue.h"

class MessageQueueSpatialChat;
class MessageQueueData;

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//======================================================================

class MessageQueueSpatialChatArchive
{
public:

	static MessageQueue::Data *     get (Archive::ReadIterator & source);
	static void                     put (const MessageQueue::Data * source, Archive::ByteStream & target);
};

//======================================================================

#endif
