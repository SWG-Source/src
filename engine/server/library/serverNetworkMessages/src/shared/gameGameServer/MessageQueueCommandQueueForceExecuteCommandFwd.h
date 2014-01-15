// ======================================================================
//
// MessageQueueCommandQueueForceExecuteCommandFwd.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueueCommandQueueForceExecuteCommandFwd_H
#define INCLUDED_MessageQueueCommandQueueForceExecuteCommandFwd_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
// ======================================================================

class MessageQueueCommandQueueForceExecuteCommandFwd: public MessageQueue::Data
{

	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueCommandQueueForceExecuteCommandFwd(uint32 commandHash, NetworkId const &targetId, Unicode::String const &params, int status, bool commandIsFromCommandQueue);
	MessageQueueCommandQueueForceExecuteCommandFwd(const MessageQueueCommandQueueForceExecuteCommandFwd &source);

	uint32 getCommandHash() const;
	NetworkId const &getTargetId() const;
	Unicode::String const &getParams() const;
	int getStatus() const;
	bool getCommandIsFromCommandQueue() const;

private:
	MessageQueueCommandQueueForceExecuteCommandFwd();
	MessageQueueCommandQueueForceExecuteCommandFwd &operator=(const MessageQueueCommandQueueForceExecuteCommandFwd &source);

	uint32           m_commandHash;
	NetworkId        m_targetId;
	Unicode::String  m_params;
	int              m_status;
	bool             m_commandIsFromCommandQueue;
};


// ======================================================================

#endif // INCLUDED_MessageQueueCommandQueueForceExecuteCommandFwd_H

