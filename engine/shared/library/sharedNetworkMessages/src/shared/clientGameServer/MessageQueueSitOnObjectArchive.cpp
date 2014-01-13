//======================================================================
//
// MessageQueueSitOnObjectArchive.cpp
// Copyright (c) 2002 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSitOnObjectArchive.h"

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/MessageQueueSitOnObject.h"


//======================================================================

MessageQueue::Data *MessageQueueSitOnObjectArchive::get(Archive::ReadIterator & source)
{
	NetworkId  chairCellId;
	Vector     chairPosition_p;

	Archive::get(source, chairCellId);
	Archive::get(source, chairPosition_p);
	return new MessageQueueSitOnObject(chairCellId, chairPosition_p);
}

//----------------------------------------------------------------------

void MessageQueueSitOnObjectArchive::put(const MessageQueue::Data *source, Archive::ByteStream &target)
{
	const MessageQueueSitOnObject * const message = dynamic_cast<const MessageQueueSitOnObject*>(source);
	if (!message)
	{
		WARNING_STRICT_FATAL(true, ("attempted to pack SitOnObject message queue message with wrong data type."));
		return;
	}

	NetworkId const &chairCellId      = message->getChairCellId();
	Vector const    &chairPosition_p = message->getChairPosition_p();

	Archive::put(target, chairCellId);
	Archive::put(target, chairPosition_p);
}

//======================================================================
