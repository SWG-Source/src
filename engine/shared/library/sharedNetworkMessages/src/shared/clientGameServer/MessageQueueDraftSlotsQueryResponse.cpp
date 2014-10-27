//======================================================================
//
// MessageQueueDraftSlotsQueryResponse.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsQueryResponse.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsDataArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueDraftSlotsQueryResponse, CM_draftSlotsQueryResponse);


//======================================================================

MessageQueueDraftSlotsQueryResponse::MessageQueueDraftSlotsQueryResponse (const std::pair<uint32, uint32> & combinedCrc) :
m_crc            (combinedCrc),
m_slots          (),
m_complexity     (0),
m_volume         (1),
m_canManufacture (true)
{
}

//----------------------------------------------------------------------

MessageQueueDraftSlotsQueryResponse::~MessageQueueDraftSlotsQueryResponse ()
{
}

//----------------------------------------------------------------------

void MessageQueueDraftSlotsQueryResponse::setSlots (const SlotVector & sv)
{
	m_slots = sv;
}

//----------------------------------------------------------------------


void MessageQueueDraftSlotsQueryResponse::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueDraftSlotsQueryResponse * const msg = safe_cast<const MessageQueueDraftSlotsQueryResponse*> (data);
	if (msg)
	{
		Archive::put (target, msg->m_crc);
		Archive::put (target, msg->getComplexity ());
		Archive::put (target, msg->getVolume ());
		Archive::put (target, msg->canManufacture ());
		
		const SlotVector & slots = msg->m_slots;
		
		const int slotCount = static_cast<int>(slots.size ());
		Archive::put (target, slotCount);
		
		for (SlotVector::const_iterator it = slots.begin (); it != slots.end (); ++it)
		{
			const MessageQueueDraftSlotsQueryResponse::Slot & slot = *it;
			Archive::put (target, slot);
		}
	}
}

//----------------------------------------------------------------------

MessageQueue::Data* MessageQueueDraftSlotsQueryResponse::unpack(Archive::ReadIterator & source)
{
	std::pair<uint32, uint32> crc;
	int complexity, volume;
	bool canManufacture;
	
	Archive::get(source, crc);
	Archive::get(source, complexity);
	Archive::get(source, volume);
	Archive::get(source, canManufacture);
	
	MessageQueueDraftSlotsQueryResponse * const msg = new MessageQueueDraftSlotsQueryResponse (crc);
	msg->setComplexity(complexity);
	msg->setVolume(volume);
	msg->setCanManufacture(canManufacture);
	
	MessageQueueDraftSlotsQueryResponse::Slot slot;
	MessageQueueDraftSlotsQueryResponse::Option option;
	
	int slotCount = 0;
	Archive::get(source, slotCount);

	SlotVector slots;
	slots.reserve (slotCount);

	for (int i = 0; i < slotCount; ++i)
	{
		Archive::get (source, slot);
		slots.push_back (slot);
		slot.options.clear();
	}

	msg->setSlots (slots);
	
	return msg;
}

//======================================================================
