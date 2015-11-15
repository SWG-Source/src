//======================================================================
//
// MessageQueueGenericResponse.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueGenericResponse_H
#define INCLUDED_MessageQueueGenericResponse_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

class MessageQueueGenericResponse : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueGenericResponse (int requestId, bool success, uint8 sequenceId);
	virtual ~MessageQueueGenericResponse ();

	bool              getSuccess    () const;
	uint8             getSequenceId () const;
	int               getRequestId  () const;

private:
	MessageQueueGenericResponse (const MessageQueueGenericResponse &);
	MessageQueueGenericResponse & operator= (const MessageQueueGenericResponse &);

	int         m_requestId;
	bool        m_success;
	uint8       m_sequenceId;
};

//----------------------------------------------------------------------

inline MessageQueueGenericResponse::MessageQueueGenericResponse (int requestId, bool success, uint8 sequenceId) :
m_requestId (requestId),
m_success (success),
m_sequenceId (sequenceId)
{
}

//----------------------------------------------------------------------

inline bool MessageQueueGenericResponse::getSuccess ()       const
{
	return m_success;
}

//----------------------------------------------------------------------

inline uint8 MessageQueueGenericResponse::getSequenceId ()    const
{
	return m_sequenceId;
}

//----------------------------------------------------------------------

inline int MessageQueueGenericResponse::getRequestId  () const
{
	return m_requestId;
}

//======================================================================

#endif
