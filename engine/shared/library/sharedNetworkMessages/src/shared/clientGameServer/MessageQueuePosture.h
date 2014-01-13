//======================================================================
//
// MessageQueuePosture.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueuePosture_H
#define INCLUDED_MessageQueuePosture_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

class MessageQueuePosture : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueuePosture (uint8 posture, bool newIsClientImmediate);
	virtual ~MessageQueuePosture ();

	uint8             getPosture ()    const;
	bool              isClientImmediate () const;

private:
	MessageQueuePosture (const MessageQueuePosture &);
	MessageQueuePosture & operator= (const MessageQueuePosture &);

	uint8       m_posture;
	bool        m_isClientImmediate;
};

//----------------------------------------------------------------------

inline MessageQueuePosture::MessageQueuePosture (uint8 posture, bool newIsClientImmediate):
	m_posture (posture),
	m_isClientImmediate (newIsClientImmediate)
{
}

//----------------------------------------------------------------------

inline uint8 MessageQueuePosture::getPosture ()    const
{
	return m_posture;
}

//----------------------------------------------------------------------

inline bool MessageQueuePosture::isClientImmediate () const
{
	return m_isClientImmediate;
}

//======================================================================

#endif
