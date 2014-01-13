//======================================================================
//
// MessageQueueSocialArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueSocialArchive_H
#define INCLUDED_MessageQueueSocialArchive_H

#include "sharedFoundation/MessageQueue.h"

class MessageQueueSocial;

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//======================================================================

class MessageQueueSocialArchive
{
public:

	static MessageQueue::Data *     get (Archive::ReadIterator & source);
	static void                     put (const MessageQueue::Data * source, Archive::ByteStream & target);
};

//======================================================================

#endif
