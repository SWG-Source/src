//======================================================================
//
// MessageQueueMissionCreateRequest.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionCreateRequest_H
#define INCLUDED_MessageQueueMissionCreateRequest_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
class MessageQueueMissionCreateRequestData;

//-----------------------------------------------------------------------

/**
* The proper response to a MessageQueueMissionCreateRequest is a MessageQueueMissionGenericResponse
*/

class MessageQueueMissionCreateRequest : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueMissionCreateRequest (const MessageQueueMissionCreateRequestData & data, uint8 sequenceId);
	~MessageQueueMissionCreateRequest ();

	const MessageQueueMissionCreateRequestData & getData () const;
	uint8             getSequenceId ()    const;

private:
	MessageQueueMissionCreateRequest (const MessageQueueMissionCreateRequest &);
	MessageQueueMissionCreateRequest & operator= (const MessageQueueMissionCreateRequest &);

	MessageQueueMissionCreateRequestData * m_data;
	uint8                                  m_sequenceId;
};

//----------------------------------------------------------------------

inline const MessageQueueMissionCreateRequestData & MessageQueueMissionCreateRequest::getData () const
{
	return *NON_NULL (m_data);
}

//----------------------------------------------------------------------

inline uint8 MessageQueueMissionCreateRequest::getSequenceId () const
{
	return m_sequenceId;
}

//======================================================================

#endif
