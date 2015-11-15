//========================================================================
//
// MessageQueueCyberneticsChangeRequest.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCyberneticsChangeRequest.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueCyberneticsChangeRequest, CM_cyberneticsChangeRequest);

//===================================================================

/**
 * Class destructor.
 */
MessageQueueCyberneticsChangeRequest::~MessageQueueCyberneticsChangeRequest()
{
}	// MessageQueueCyberneticsChangeRequest::~MessageQueueCyberneticsChangeRequest

//----------------------------------------------------------------------

void MessageQueueCyberneticsChangeRequest::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCyberneticsChangeRequest* const msg = safe_cast<const MessageQueueCyberneticsChangeRequest*> (data);
	if (msg)
	{
		Archive::put(target, msg->getChangeType());
		Archive::put(target, msg->getCyberneticPiece());
		Archive::put(target, msg->getTarget());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCyberneticsChangeRequest::unpack (Archive::ReadIterator & source)
{
	int changeTypeInt = -1;
	NetworkId cyberneticPiece;
	NetworkId target;

	Archive::get(source, changeTypeInt);
	Archive::get(source, cyberneticPiece);
	Archive::get(source, target);

	return new MessageQueueCyberneticsChangeRequest(static_cast<ChangeType>(changeTypeInt), cyberneticPiece, target);
}

//----------------------------------------------------------------------
