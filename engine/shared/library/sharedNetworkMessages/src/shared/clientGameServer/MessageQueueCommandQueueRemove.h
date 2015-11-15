// ======================================================================
//
// MessageQueueCommandQueueRemove.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueueCommandQueueRemove_H
#define INCLUDED_MessageQueueCommandQueueRemove_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

// ======================================================================

class MemoryBlockManager;

// ======================================================================

class MessageQueueCommandQueueRemove: public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueCommandQueueRemove(uint32 sequenceId, float waitTime, int status, int statusDetail);
	MessageQueueCommandQueueRemove(MessageQueueCommandQueueRemove const &source);

	uint32 getSequenceId() const;
	float getWaitTime() const;
	int getStatus() const;
	int getStatusDetail() const;

private:
	MessageQueueCommandQueueRemove();
	MessageQueueCommandQueueRemove &operator=(MessageQueueCommandQueueRemove const &source);

	uint32 m_sequenceId;
	float m_waitTime;
	int m_status;
	int m_statusDetail;
};

// ======================================================================

#endif // INCLUDED_MessageQueueCommandQueueRemove_H

