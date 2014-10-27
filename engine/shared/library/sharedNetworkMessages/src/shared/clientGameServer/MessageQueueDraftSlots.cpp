//========================================================================
//
// MessageQueueDraftSlots.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueDraftSlots.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueCraftIngredients.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsDataArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueDraftSlots, CM_draftSlotsMessage);

//===================================================================

/**
 * Class destructor.
 */
MessageQueueDraftSlots::~MessageQueueDraftSlots()
{
}

//----------------------------------------------------------------------

void MessageQueueDraftSlots::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueDraftSlots* const msg = safe_cast<const MessageQueueDraftSlots*> (data);
	if (msg)
	{
		Archive::put (target, msg->getToolId());
		Archive::put (target, msg->getManfSchemId());
		Archive::put (target, msg->getPrototypeId());
		Archive::put (target, msg->getVolume());
		Archive::put (target, msg->canManufacture());
		
		int slotCount = msg->getSlotCount();
		Archive::put (target, slotCount);

		for (int i = 0; i < slotCount; ++i)
		{
			const MessageQueueDraftSlots::Slot & slot = msg->getSlot(i);
			Archive::put (target, slot);
		}
	}
}

//----------------------------------------------------------------------

MessageQueue::Data* MessageQueueDraftSlots::unpack(Archive::ReadIterator & source)
{
	NetworkId toolId;
	NetworkId manfSchemId;
	NetworkId testProtoId;
	int       volume;
	bool      canManufacture;
	
	Archive::get(source, toolId);
	Archive::get(source, manfSchemId);
	Archive::get(source, testProtoId);
	Archive::get(source, volume);
	Archive::get(source, canManufacture);
	
	MessageQueueDraftSlots * const msg = new MessageQueueDraftSlots(toolId, manfSchemId);

	msg->setPrototypeId(testProtoId);
	msg->setVolume(volume);
	msg->setCanManufacture(canManufacture);
	
	MessageQueueDraftSlots::Slot slot;
	
	int slotCount = 0;
	Archive::get(source, slotCount);
	for (int i = 0; i < slotCount; ++i)
	{
		Archive::get(source, slot);
		msg->addSlot (slot);
		slot.options.clear ();
	}
	
	return msg;
}
//-----------------------------------------------------------------------

