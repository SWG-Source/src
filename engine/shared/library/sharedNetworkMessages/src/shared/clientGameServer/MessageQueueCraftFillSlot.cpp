//========================================================================
//
// MessageQueueCraftFillSlot.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCraftFillSlot.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueCraftFillSlot, CM_fillSchematicSlotMessage);

//===================================================================



/**
 * Class destructor. 
 */
MessageQueueCraftFillSlot::~MessageQueueCraftFillSlot()
{
}	// MessageQueueCraftFillSlot::~MessageQueueCraftFillSlot

//-----------------------------------------------------------------------

void MessageQueueCraftFillSlot::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCraftFillSlot* const msg = safe_cast<const MessageQueueCraftFillSlot*> (data);
	if (msg)
	{
		Archive::put(target, msg->getIngredient());
		Archive::put(target, msg->getSlot());
		Archive::put(target, msg->getOption());
		Archive::put(target, msg->getSequenceId());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCraftFillSlot::unpack(Archive::ReadIterator & source)
{
	NetworkId ingredient;
	int slot, option;
	uint8 sequenceId;
	
	Archive::get(source, ingredient);
	Archive::get(source, slot);
	Archive::get(source, option);
	Archive::get(source, sequenceId);
	
	return new MessageQueueCraftFillSlot(ingredient, slot, option, sequenceId);
}

//-----------------------------------------------------------------------

