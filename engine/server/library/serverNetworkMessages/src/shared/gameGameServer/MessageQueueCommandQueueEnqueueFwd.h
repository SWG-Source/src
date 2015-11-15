// ======================================================================
//
// MessageQueueCommandQueueEnqueueFwd.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueueCommandQueueEnqueueFwd_H
#define INCLUDED_MessageQueueCommandQueueEnqueueFwd_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

// ======================================================================

class MemoryBlockManager;

// ======================================================================

class MessageQueueCommandQueueEnqueueFwd: public MessageQueue::Data
{

	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueCommandQueueEnqueueFwd(uint32 sequenceId, uint32 commandHash,
		NetworkId const &targetId, Unicode::String const &params, bool clearable,
		int priority, bool fromServer=false);
	MessageQueueCommandQueueEnqueueFwd(const MessageQueueCommandQueueEnqueueFwd &source);

	uint32 getSequenceId() const;
	uint32 getCommandHash() const;
	NetworkId const &getTargetId() const;
	Unicode::String const &getParams() const;
	bool getClearable() const;
	int getPriority() const;
	bool getFromServer() const;

private:
	MessageQueueCommandQueueEnqueueFwd();
	MessageQueueCommandQueueEnqueueFwd &operator=(const MessageQueueCommandQueueEnqueueFwd &source);

	uint32           m_sequenceId;
	uint32           m_commandHash;
	NetworkId        m_targetId;
	Unicode::String  m_params;
	bool             m_clearable;
	int              m_priority;
	bool			 m_fromServer;
};

// ======================================================================

#endif // INCLUDED_MessageQueueCommandQueueEnqueueFwd_H

