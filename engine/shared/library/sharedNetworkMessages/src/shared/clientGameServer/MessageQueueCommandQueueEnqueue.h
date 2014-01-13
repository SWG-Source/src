// ======================================================================
//
// MessageQueueCommandQueueEnqueue.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueueCommandQueueEnqueue_H
#define INCLUDED_MessageQueueCommandQueueEnqueue_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
// ======================================================================

class MessageQueueCommandQueueEnqueue: public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;


public:

	MessageQueueCommandQueueEnqueue(uint32 sequenceId, uint32 commandHash,
		NetworkId const &targetId, Unicode::String const &params);
	MessageQueueCommandQueueEnqueue(const MessageQueueCommandQueueEnqueue &source);

	uint32 getSequenceId() const;
	uint32 getCommandHash() const;
	NetworkId const &getTargetId() const;
	Unicode::String const &getParams() const;

private:
	MessageQueueCommandQueueEnqueue();
	MessageQueueCommandQueueEnqueue &operator=(const MessageQueueCommandQueueEnqueue &source);

	uint32           m_sequenceId;
	uint32           m_commandHash;
	NetworkId        m_targetId;
	Unicode::String  m_params;
};

// ----------------------------------------------------------------------

// ======================================================================

#endif // INCLUDED_MessageQueueCommandQueueEnqueue_H

