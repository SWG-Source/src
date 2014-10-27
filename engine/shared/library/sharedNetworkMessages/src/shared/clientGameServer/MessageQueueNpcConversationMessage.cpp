//========================================================================
//
// MessageQueueNpcConversationMessage.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueNpcConversationMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueNpcConversationMessage, CM_npcConversationMessage);

//===================================================================



/**
 * Class destructor.
 */
MessageQueueNpcConversationMessage::~MessageQueueNpcConversationMessage()
{
}	// MessageQueueNpcConversationMessage::~MessageQueueNpcConversationMessage


void MessageQueueNpcConversationMessage::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueNpcConversationMessage* const msg = safe_cast<const MessageQueueNpcConversationMessage*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getNpcMessage());
	}
}	
//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueNpcConversationMessage::unpack(Archive::ReadIterator & source)
{
	MessageQueueNpcConversationMessage * msg = new MessageQueueNpcConversationMessage;
	
	Unicode::String response;
	Archive::get(source, response);
	msg->setNpcMessage(response);
	
	return msg;
}	

//-----------------------------------------------------------------------

