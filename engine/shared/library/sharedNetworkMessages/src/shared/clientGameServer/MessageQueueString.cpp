//======================================================================
//
// MessageQueueString.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueString.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueString);

//===================================================================

void MessageQueueString::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_attachScript, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_detachScript, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_animationAction, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueString::remove"); 
} 

//-----------------------------------------------------------------------

MessageQueueString::MessageQueueString (const std::string & str) :
Data (),
m_string (str)
{
}

//-----------------------------------------------------------------------

MessageQueueString::~MessageQueueString()
{
}

//-----------------------------------------------------------------------
void MessageQueueString::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueString* const msg = safe_cast<const MessageQueueString*> (data);
	if (msg)
	{
		Archive::put (target, msg->getString ());
	}
}

//-----------------------------------------------------------------------


MessageQueue::Data* MessageQueueString::unpack(Archive::ReadIterator & source)
{
	std::string str;
	Archive::get(source, str);
	return new MessageQueueString (str);
}

//-----------------------------------------------------------------------
