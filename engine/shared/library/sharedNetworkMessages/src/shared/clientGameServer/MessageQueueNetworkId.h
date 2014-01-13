//======================================================================
//
// MessageQueueNetworkId.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueNetworkId_H
#define INCLUDED_MessageQueueNetworkId_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

/**
* A generic message queue data that contains a simple payload of one NetworkId.
* The meaning of the NetworkId is context and message dependant.
*
*/

class MessageQueueNetworkId : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	explicit                MessageQueueNetworkId (const NetworkId & id);
	virtual                ~MessageQueueNetworkId();

	const NetworkId &       getNetworkId(void) const;

private:

	MessageQueueNetworkId&  operator= (const MessageQueueNetworkId & source);
	                        MessageQueueNetworkId(const MessageQueueNetworkId & source);

	NetworkId m_id;
};

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueNetworkId::getNetworkId(void) const
{
	return m_id;
}

//-----------------------------------------------------------------------

#endif	// _MessageQueueNetworkId_H

