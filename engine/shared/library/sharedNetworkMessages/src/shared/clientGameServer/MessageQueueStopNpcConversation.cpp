//========================================================================
//
// MessageQueueStopNpcConversation.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueStopNpcConversation.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueStopNpcConversation, CM_npcConversationStop);

//===================================================================

/**
 * Class destructor.
 */
MessageQueueStopNpcConversation::~MessageQueueStopNpcConversation()
{
}	

//-----------------------------------------------------------------------

void MessageQueueStopNpcConversation::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueStopNpcConversation* const msg = safe_cast<const MessageQueueStopNpcConversation*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getNpc());
		Archive::put(target, msg->getFinalMessageId());
		Archive::put(target, msg->getFinalMessageProse());
		Archive::put(target, msg->getFinalResponse());
	}
}	

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueStopNpcConversation::unpack(Archive::ReadIterator & source)
{
	MessageQueueStopNpcConversation * msg = new MessageQueueStopNpcConversation;

	// npc
	{
		NetworkId npc;
		Archive::get(source, npc);
		msg->setNpc(npc);
	}

	// message string id
	{
		StringId finalMessageId;
		Archive::get(source, finalMessageId);
		msg->setFinalMessageId(finalMessageId);
	}

	// message prose package
	{
		Unicode::String finalMessageProse;
		Archive::get(source, finalMessageProse);
		msg->setFinalMessageProse(finalMessageProse);
	}

	// message final response
	{
		Unicode::String finalResponse;
		Archive::get(source, finalResponse);
		msg->setFinalResponse(finalResponse);
	}
	
	return msg;
}	

//-----------------------------------------------------------------------
