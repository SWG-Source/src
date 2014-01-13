// MessageQueueMissionDetailsResponse.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MessageQueueMissionDetailsResponse_H
#define	_INCLUDED_MessageQueueMissionDetailsResponse_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
struct MessageQueueMissionDetailsResponseData;

//-----------------------------------------------------------------------

class MessageQueueMissionDetailsResponse : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueMissionDetailsResponse();
	MessageQueueMissionDetailsResponse(const MessageQueueMissionDetailsResponseData & data, const uint8 sequenceId);
	~MessageQueueMissionDetailsResponse();

	const MessageQueueMissionDetailsResponseData & getData () const;
	const uint8                                    getSequenceId() const;

	void set(const MessageQueueMissionDetailsResponseData & data, const uint8 sequenceId);

private:
	MessageQueueMissionDetailsResponse & operator = (const MessageQueueMissionDetailsResponse & rhs);
	MessageQueueMissionDetailsResponse(const MessageQueueMissionDetailsResponse & source);

	MessageQueueMissionDetailsResponseData * m_data;
	uint8                                    m_sequenceId;
};

//-----------------------------------------------------------------------

inline const MessageQueueMissionDetailsResponseData & MessageQueueMissionDetailsResponse::getData() const
{
	return *NON_NULL(m_data);
}

//-----------------------------------------------------------------------

inline const uint8 MessageQueueMissionDetailsResponse::getSequenceId() const
{
	return m_sequenceId;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageQueueMissionDetailsResponse_H
