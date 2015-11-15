//========================================================================
//
// MessageQueueStringList.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueStringList.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueStringList);

//===================================================================

void MessageQueueStringList::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_npcConversationResponses, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueStringList::remove"); 
} 

/**
 * Class destructor.
 */
MessageQueueStringList::~MessageQueueStringList()
{
}
//-----------------------------------------------------------------------


void MessageQueueStringList::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueStringList* const msg = safe_cast<const MessageQueueStringList*> (data);
	
	if (msg)
	{
		int count = msg->getStringCount();
		uint8 temp8 = static_cast<uint8>(count);
		Archive::put(target, temp8);
		for (int i = 0; i < count; ++i)
		{
			const Unicode::String & string = msg->getString(i);
			Archive::put(target, string);
		}
	}
}	
//-----------------------------------------------------------------------


MessageQueue::Data* MessageQueueStringList::unpack(Archive::ReadIterator & source)
{
	MessageQueueStringList * msg = new MessageQueueStringList;
	
	uint8 count;
	Unicode::String string;
	Archive::get(source, count);
	for (int i = 0; i < count; ++i)
	{
		Archive::get(source, string);
		msg->addString(string);
	}
	
	return msg;
}	




//========================================================================
