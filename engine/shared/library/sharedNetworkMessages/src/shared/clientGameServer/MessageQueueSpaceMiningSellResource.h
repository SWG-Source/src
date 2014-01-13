//======================================================================
//
// MessageQueueSpaceMiningSellResource.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueSpaceMiningSellResource_H
#define INCLUDED_MessageQueueSpaceMiningSellResource_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

class MessageQueueSpaceMiningSellResource : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueSpaceMiningSellResource(NetworkId const & shipId, NetworkId const & spaceStationId, NetworkId const & resourceId, int amount);

private:

	MessageQueueSpaceMiningSellResource(MessageQueueSpaceMiningSellResource const &);
	MessageQueueSpaceMiningSellResource();
	MessageQueueSpaceMiningSellResource& operator=(MessageQueueSpaceMiningSellResource const &);

public:

	NetworkId m_shipId;
	NetworkId m_spaceStationId;
	NetworkId m_resourceId;
	int m_amount;
};

//======================================================================

#endif
