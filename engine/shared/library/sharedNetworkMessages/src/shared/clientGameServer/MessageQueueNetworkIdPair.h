//======================================================================
//
// MessageQueueNetworkIdPair.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueNetworkIdPair_H
#define INCLUDED_MessageQueueNetworkIdPair_H

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

class MessageQueueNetworkIdPair : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	explicit                MessageQueueNetworkIdPair (const NetworkId & first, const NetworkId & second);
	virtual                ~MessageQueueNetworkIdPair();

	const NetworkId &       getFirstNetworkId(void) const;
	const NetworkId &       getSecondNetworkId(void) const;

private:

	MessageQueueNetworkIdPair&  operator= (const MessageQueueNetworkIdPair & source);
	                        MessageQueueNetworkIdPair(const MessageQueueNetworkIdPair & source);

	NetworkId m_firstId;
	NetworkId m_secondId;
};

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueNetworkIdPair::getFirstNetworkId(void) const
{
	return m_firstId;
}

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueNetworkIdPair::getSecondNetworkId(void) const
{
	return m_secondId;
}

//-----------------------------------------------------------------------

#endif	// _MessageQueueNetworkIdPair_H

