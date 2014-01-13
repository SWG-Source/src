//======================================================================
//
// MessageQueueSitOnObjectArchive.h
// Copyright (c) 2002 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#ifndef INCLUDED_MessageQueueSitOnObjectArchive_H
#define INCLUDED_MessageQueueSitOnObjectArchive_H

#include "Archive/ByteStream.h"
#include "sharedFoundation/MessageQueue.h"

class MessageQueueSpatialChat;

//======================================================================

class MessageQueueSitOnObjectArchive
{
public:

	static MessageQueue::Data *get(Archive::ReadIterator &source);
	static void                put(const MessageQueue::Data *source, Archive::ByteStream &target);
};

//======================================================================

#endif
