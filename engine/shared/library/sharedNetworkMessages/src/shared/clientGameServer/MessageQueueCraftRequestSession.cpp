//========================================================================
//
// MessageQueueCraftRequestSession.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCraftRequestSession.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueCraftRequestSession, CM_requestCraftingSession);

//===================================================================



/**
 * Class destructor.
 */
MessageQueueCraftRequestSession::~MessageQueueCraftRequestSession()
{
}	// MessageQueueCraftRequestSession::~MessageQueueCraftRequestSession

//----------------------------------------------------------------------

void MessageQueueCraftRequestSession::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCraftRequestSession* const msg = safe_cast<const MessageQueueCraftRequestSession*> (data);
	if (msg)
	{
		Archive::put(target, msg->getStationId());
		Archive::put(target, msg->getSequenceId());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCraftRequestSession::unpack (Archive::ReadIterator & source)
{
	NetworkId stationId;
	uint8 sequenceId;
	
	Archive::get(source, stationId);
	Archive::get(source, sequenceId);
	
	return new MessageQueueCraftRequestSession(stationId, sequenceId);
}

//----------------------------------------------------------------------
