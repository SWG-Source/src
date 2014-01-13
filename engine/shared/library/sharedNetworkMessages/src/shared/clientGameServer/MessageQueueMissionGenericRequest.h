//======================================================================
//
// MessageQueueMissionGenericRequest.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionGenericRequest_H
#define INCLUDED_MessageQueueMissionGenericRequest_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

class MessageQueueMissionGenericRequest : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueMissionGenericRequest ();
	MessageQueueMissionGenericRequest (const NetworkId & missionObjectId, const NetworkId & terminalId, uint8 sequenceId);
	~MessageQueueMissionGenericRequest ();

	const NetworkId & getMissionObjectId () const;
	const NetworkId & getTerminalId    () const;
	uint8             getSequenceId ()    const;

	void              set (const NetworkId & missionObjectId, const NetworkId & terminalId, uint8 sequenceId);

private:
	MessageQueueMissionGenericRequest (const MessageQueueMissionGenericRequest &);
	MessageQueueMissionGenericRequest & operator= (const MessageQueueMissionGenericRequest &);

	NetworkId * m_missionObjectId;
	NetworkId * m_terminalId;
	uint8       m_sequenceId;
};

//----------------------------------------------------------------------

inline const NetworkId & MessageQueueMissionGenericRequest::getMissionObjectId () const
{
	return *NON_NULL(m_missionObjectId);
}

//----------------------------------------------------------------------

inline const NetworkId & MessageQueueMissionGenericRequest::getTerminalId () const
{
	return *NON_NULL(m_terminalId);
}

//----------------------------------------------------------------------

inline uint8 MessageQueueMissionGenericRequest::getSequenceId ()    const
{
	return m_sequenceId;
}

//======================================================================

#endif
