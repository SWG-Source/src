//======================================================================
//
// MessageQueueResourceEmptyHopperArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueResourceEmptyHopperArchive.h"

#include "sharedNetworkMessages/MessageQueueResourceEmptyHopper.h"
#include "sharedFoundation/NetworkIdArchive.h"

//======================================================================

MessageQueue::Data * MessageQueueResourceEmptyHopperArchive::get (Archive::ReadIterator & source)
{
	NetworkId playerId;
	NetworkId harvesterId;
	NetworkId resourceId;
	int       amount;
	bool      discard;
	uint8     sequenceId;

	Archive::get (source, playerId);
	Archive::get (source, harvesterId);
	Archive::get (source, resourceId);
	Archive::get (source, amount);
	Archive::get (source, discard);
	Archive::get (source, sequenceId);

	return new MessageQueueResourceEmptyHopper (playerId, harvesterId, resourceId, amount, discard, sequenceId);
}

//----------------------------------------------------------------------

void MessageQueueResourceEmptyHopperArchive::put (const MessageQueue::Data * source, Archive::ByteStream & target)
{
	const MessageQueueResourceEmptyHopper * msg = dynamic_cast<const MessageQueueResourceEmptyHopper *>(source);
	NOT_NULL (msg);

	if (msg)
	{
		Archive::put (target, msg->m_playerId);
		Archive::put (target, msg->m_harvesterId);
		Archive::put (target, msg->m_resourceId);
		Archive::put (target, msg->m_amount);
		Archive::put (target, msg->m_discard);
		Archive::put (target, msg->m_sequenceId);
	}
}

//======================================================================
