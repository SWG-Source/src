//======================================================================
//
// MessageQueueGenericIntResponseArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueGenericIntResponseArchive.h"

#include "sharedNetworkMessages/MessageQueueGenericIntResponse.h"
#include "sharedFoundation/NetworkIdArchive.h"

//======================================================================

MessageQueue::Data * MessageQueueGenericIntResponseArchive::get (Archive::ReadIterator & source)
{
	int   requestId;
	int   response;
	uint8 sequenceId;
	Archive::get(source, requestId);
	Archive::get(source, response);
	Archive::get(source, sequenceId);
	return new MessageQueueGenericIntResponse(requestId, response, sequenceId);
}

//----------------------------------------------------------------------

void MessageQueueGenericIntResponseArchive::put (const MessageQueue::Data * source, Archive::ByteStream & target)
{
	const MessageQueueGenericIntResponse * const msg = dynamic_cast<const MessageQueueGenericIntResponse  *>(source);
	NOT_NULL (msg);

	if (msg)
	{
		Archive::put (target, msg->getRequestId());
		Archive::put (target, msg->getResponse());
		Archive::put (target, msg->getSequenceId());
	}
}

//======================================================================
