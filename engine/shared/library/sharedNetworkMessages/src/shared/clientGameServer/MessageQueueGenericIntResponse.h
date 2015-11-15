//======================================================================
//
// MessageQueueGenericIntResponse.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueGenericIntResponse_H
#define INCLUDED_MessageQueueGenericIntResponse_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

class MessageQueueGenericIntResponse : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueGenericIntResponse (int requestId, int response, uint8 sequenceId);
	virtual ~MessageQueueGenericIntResponse ();

	int               getResponse   () const;
	uint8             getSequenceId () const;
	int               getRequestId  () const;

private:
	MessageQueueGenericIntResponse (const MessageQueueGenericIntResponse &);
	MessageQueueGenericIntResponse & operator= (const MessageQueueGenericIntResponse &);

	int         m_requestId;
	int         m_response;
	uint8       m_sequenceId;
};

//----------------------------------------------------------------------

inline MessageQueueGenericIntResponse::MessageQueueGenericIntResponse (int requestId, int response, uint8 sequenceId) :
m_requestId (requestId),
m_response (response),
m_sequenceId (sequenceId)
{
}

//----------------------------------------------------------------------

inline int MessageQueueGenericIntResponse::getResponse ()       const
{
	return m_response;
}

//----------------------------------------------------------------------

inline uint8 MessageQueueGenericIntResponse::getSequenceId ()    const
{
	return m_sequenceId;
}

//----------------------------------------------------------------------

inline int MessageQueueGenericIntResponse::getRequestId  () const
{
	return m_requestId;
}

//======================================================================

#endif
