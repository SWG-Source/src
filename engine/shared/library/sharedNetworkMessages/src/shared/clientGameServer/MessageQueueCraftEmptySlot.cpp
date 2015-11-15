//========================================================================
//
// MessageQueueCraftEmptySlot.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCraftEmptySlot.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueCraftEmptySlot, CM_emptySchematicSlotMessage);

//----------------------------------------------------------------------

MessageQueueCraftEmptySlot::MessageQueueCraftEmptySlot (int slot, const NetworkId & targetContainer, uint8 sequenceId) :
m_slot            (slot),
m_targetContainer (targetContainer),
m_sequenceId      (sequenceId)
{
}

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
MessageQueueCraftEmptySlot::~MessageQueueCraftEmptySlot ()
{
}

//----------------------------------------------------------------------

void MessageQueueCraftEmptySlot::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCraftEmptySlot* const msg = safe_cast<const MessageQueueCraftEmptySlot*> (data);
	if (msg)
	{
		Archive::put(target, msg->getSlot ());
		Archive::put(target, msg->getTargetContainer ());
		Archive::put(target, msg->getSequenceId ());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCraftEmptySlot::unpack (Archive::ReadIterator & source)
{
	int slot = 0;
	uint8 sequenceId = 0;
	NetworkId targetContainer;
	
	Archive::get(source, slot);
	Archive::get(source, targetContainer);
	Archive::get(source, sequenceId);
	
	return new MessageQueueCraftEmptySlot(slot, targetContainer, sequenceId);
}

//----------------------------------------------------------------------

