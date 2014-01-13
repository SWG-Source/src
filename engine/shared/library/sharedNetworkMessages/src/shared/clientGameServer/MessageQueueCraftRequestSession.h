//========================================================================
//
// MessageQueueCraftRequestSession.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCraftRequestSession_H
#define INCLUDED_MessageQueueCraftRequestSession_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueCraftRequestSession : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueCraftRequestSession(const NetworkId & stationId, uint8 sequenceId);
	virtual ~MessageQueueCraftRequestSession();

	const NetworkId & getStationId(void) const;
	uint8             getSequenceId(void) const;

private:
	NetworkId         m_stationId;
	uint8             m_sequenceId;

private:
	MessageQueueCraftRequestSession (const MessageQueueCraftRequestSession &);
	MessageQueueCraftRequestSession & operator= (const MessageQueueCraftRequestSession &);
};


inline MessageQueueCraftRequestSession::MessageQueueCraftRequestSession(const NetworkId & stationId, uint8 sequenceId) :
	m_stationId(stationId),
	m_sequenceId(sequenceId)
{
}	// MessageQueueCraftRequestSession::MessageQueueCraftRequestSession

inline const NetworkId & MessageQueueCraftRequestSession::getStationId(void) const
{
	return m_stationId;
}	// MessageQueueCraftRequestSession::getStationId

inline uint8 MessageQueueCraftRequestSession::getSequenceId(void) const
{
	return m_sequenceId;
}	// MessageQueueCraftRequestSession::getSequenceId


#endif	// INCLUDED_MessageQueueCraftRequestSession_H
