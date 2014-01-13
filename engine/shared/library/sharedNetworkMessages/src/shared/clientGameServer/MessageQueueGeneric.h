//======================================================================
//
// MessageQueueGeneric.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueGeneric_H
#define INCLUDED_MessageQueueGeneric_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

class MessageQueueGeneric : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueGeneric (uint8 sequenceId);
	virtual ~MessageQueueGeneric ();

	uint8             getSequenceId ()    const;

private:
	MessageQueueGeneric (const MessageQueueGeneric &);
	MessageQueueGeneric & operator= (const MessageQueueGeneric &);

	uint8       m_sequenceId;
};


inline MessageQueueGeneric::MessageQueueGeneric (uint8 sequenceId) :
m_sequenceId (sequenceId)
{
}

//----------------------------------------------------------------------

inline uint8 MessageQueueGeneric::getSequenceId ()    const
{
	return m_sequenceId;
}

//======================================================================

#endif
