//======================================================================
//
// MessageQueueSpatialChat.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSpaceMiningSellResource.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//----------------------------------------------------------------------

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueSpaceMiningSellResource, CM_spaceMiningSaleSellResource);

//----------------------------------------------------------------------


MessageQueueSpaceMiningSellResource::MessageQueueSpaceMiningSellResource(NetworkId const & shipId, NetworkId const & spaceStationId, NetworkId const & resourceId, int amount) :
MessageQueue::Data(),
m_shipId(shipId),
m_spaceStationId(spaceStationId),
m_resourceId(resourceId),
m_amount(amount)
{
}

//----------------------------------------------------------------------

void MessageQueueSpaceMiningSellResource::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	MessageQueueSpaceMiningSellResource const * const msg = safe_cast<const MessageQueueSpaceMiningSellResource*> (data);

	if (!msg)
		return;

	Archive::put(target, msg->m_shipId);
	Archive::put(target, msg->m_spaceStationId);
	Archive::put(target, msg->m_resourceId);
	Archive::put(target, msg->m_amount);
}	

//-----------------------------------------------------------------------


MessageQueue::Data* MessageQueueSpaceMiningSellResource::unpack(Archive::ReadIterator & source)
{
	NetworkId shipId;
	NetworkId spaceStationId;
	NetworkId resourceId;
	int amount = 0;

	Archive::get(source, shipId);
	Archive::get(source, spaceStationId);
	Archive::get(source, resourceId);
	Archive::get(source, amount);

	return new MessageQueueSpaceMiningSellResource(shipId, spaceStationId, resourceId, amount);
}

//----------------------------------------------------------------------

