//======================================================================
//
// MessageQueueMissionGenericResponse.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionGenericResponse_H
#define INCLUDED_MessageQueueMissionGenericResponse_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

class MessageQueueMissionGenericResponse : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueMissionGenericResponse ();
	MessageQueueMissionGenericResponse (const NetworkId & missionObjectId, bool success, uint8 sequenceId);
	~MessageQueueMissionGenericResponse ();

	const NetworkId & getMissionObjectId () const;
	bool              getSuccess ()       const;
	uint8             getSequenceId ()    const;

	void              set (const NetworkId & missionObjectId, bool success, uint8 sequenceId);

	void              setSuccess (bool b);
	void              setMissionObjectId (const NetworkId & missionObjectId);

private:
	MessageQueueMissionGenericResponse (const MessageQueueMissionGenericResponse &);
	MessageQueueMissionGenericResponse & operator= (const MessageQueueMissionGenericResponse &);

	NetworkId * m_missionObjectId;
	bool        m_success;
	uint8       m_sequenceId;
};

//----------------------------------------------------------------------

inline const NetworkId & MessageQueueMissionGenericResponse::getMissionObjectId () const
{
	return *NON_NULL(m_missionObjectId);
}

//----------------------------------------------------------------------

inline bool MessageQueueMissionGenericResponse::getSuccess ()       const
{
	return m_success;
}

//----------------------------------------------------------------------

inline uint8 MessageQueueMissionGenericResponse::getSequenceId ()    const
{
	return m_sequenceId;
}

//======================================================================

#endif
