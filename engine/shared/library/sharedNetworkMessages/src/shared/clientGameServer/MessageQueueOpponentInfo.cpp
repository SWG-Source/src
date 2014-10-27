//========================================================================
//
// MessageQueueOpponentInfo.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueOpponentInfo.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueOpponentInfo, CM_opponentInfo);

//===================================================================



/**
 * Class destructor.
 */
MessageQueueOpponentInfo::~MessageQueueOpponentInfo()
{
}	// MessageQueueOpponentInfo::~MessageQueueOpponentInfo

void MessageQueueOpponentInfo::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueOpponentInfo* const msg = safe_cast<const MessageQueueOpponentInfo*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getOpponent());
		Archive::put(target, msg->getHealth());
		Archive::put(target, msg->getAction());
		Archive::put(target, msg->getMind());
	}
}

//----------------------------------------------------------------------


MessageQueue::Data* MessageQueueOpponentInfo::unpack(Archive::ReadIterator & source)
{
	NetworkId opponent;
	uint16 health;
	uint16 action;
	uint16 mind;
	
	Archive::get(source, opponent);
	Archive::get(source, health);
	Archive::get(source, action);
	Archive::get(source, mind);
	
	MessageQueueOpponentInfo * msg = new MessageQueueOpponentInfo();
	msg->setOpponent(opponent);
	msg->setAttributes(health, action, mind);
	
	return msg;
}

//----------------------------------------------------------------------

