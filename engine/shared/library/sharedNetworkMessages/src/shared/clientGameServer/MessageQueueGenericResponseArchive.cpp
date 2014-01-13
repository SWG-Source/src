//======================================================================
//
// MessageQueueGenericResponseArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueGenericResponseArchive.h"

#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedFoundation/NetworkIdArchive.h"

//======================================================================

MessageQueue::Data * MessageQueueGenericResponseArchive::get (Archive::ReadIterator & source)
{
	int   requestId;
	bool  success;
	uint8 sequenceId;
	Archive::get(source, requestId);
	Archive::get(source, success);
	Archive::get(source, sequenceId);
	return new MessageQueueGenericResponse(requestId, success, sequenceId);
}

//----------------------------------------------------------------------

void MessageQueueGenericResponseArchive::put (const MessageQueue::Data * source, Archive::ByteStream & target)
{
	const MessageQueueGenericResponse * const msg = dynamic_cast<const MessageQueueGenericResponse  *>(source);
	NOT_NULL (msg);

	if (msg)
	{
		Archive::put (target, msg->getRequestId());
		Archive::put (target, msg->getSuccess());
		Archive::put (target, msg->getSequenceId());
	}
}

//======================================================================
