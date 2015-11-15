//======================================================================
//
// MessageQueueGenericString.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueGenericString.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueGenericString);

//===================================================================

void MessageQueueGenericString::install() 
{ 
	installMemoryBlockManager();
	ExitChain::add(&remove, "MessageQueueGenericString::remove"); 
} 

//-----------------------------------------------------------------------

MessageQueueGenericString::~MessageQueueGenericString ()
{
}
//-----------------------------------------------------------------------

void MessageQueueGenericString::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueGenericString* const msg = safe_cast<const MessageQueueGenericString*> (data);
	if (msg)
	{
		Archive::put (target, msg->getSequenceId());
		Archive::put (target, msg->getString());
	}
}

//-----------------------------------------------------------------------
MessageQueue::Data* MessageQueueGenericString::unpack (Archive::ReadIterator & source)
{
	uint8 sequenceId;
	Unicode::String string;
	Archive::get(source, sequenceId);
	Archive::get(source, string);
	return new MessageQueueGenericString(string, sequenceId);
}

//======================================================================
