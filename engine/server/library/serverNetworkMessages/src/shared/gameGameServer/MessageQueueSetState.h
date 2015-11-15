// ======================================================================
//
// MessageQueueSetState.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueueSetState_H
#define INCLUDED_MessageQueueSetState_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
// ======================================================================

class MessageQueueSetState: public MessageQueue::Data
{

	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueSetState(int state, bool value);
	MessageQueueSetState(const MessageQueueSetState &source);

	int getState() const;
	bool getValue() const;

private:
	MessageQueueSetState();
	MessageQueueSetState &operator=(const MessageQueueSetState &source);

	int   m_state;
	bool  m_value;
};

// ----------------------------------------------------------------------

// ======================================================================

#endif // INCLUDED_MessageQueueSetState_H

