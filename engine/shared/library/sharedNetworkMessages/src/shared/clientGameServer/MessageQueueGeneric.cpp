//======================================================================
//
// MessageQueueGeneric.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueGeneric.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueGeneric);

//===================================================================

void MessageQueueGeneric::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_nextCraftingStage, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_cancelCraftingSession, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_restartCraftingSession, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_createPrototype, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_createManfSchematic, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueGeneric::remove"); 
} 

//======================================================================

MessageQueueGeneric::~MessageQueueGeneric ()
{
}

//----------------------------------------------------------------------

void MessageQueueGeneric::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueGeneric* const msg = safe_cast<const MessageQueueGeneric*> (data);
	if (msg)
	{
		Archive::put (target, msg->getSequenceId());
	}
}

//-----------------------------------------------------------------------


MessageQueue::Data* MessageQueueGeneric::unpack(Archive::ReadIterator & source)
{
	uint8 sequenceId;
	Archive::get(source, sequenceId);
	return new MessageQueueGeneric(sequenceId);
}


//======================================================================
