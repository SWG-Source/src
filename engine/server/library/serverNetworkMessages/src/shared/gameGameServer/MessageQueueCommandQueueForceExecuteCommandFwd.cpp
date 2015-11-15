// ======================================================================
//
// MessageQueueCommandQueueForceExecuteCommandFwd.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageQueueCommandQueueForceExecuteCommandFwd.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueCommandQueueForceExecuteCommandFwd, CM_commandQueueForceExecuteCommandFwd);

//===================================================================

// ----------------------------------------------------------------------

MessageQueueCommandQueueForceExecuteCommandFwd::MessageQueueCommandQueueForceExecuteCommandFwd(uint32 commandHash, NetworkId const &targetId, Unicode::String const &params, int status, bool commandIsFromCommandQueue) :
	MessageQueue::Data(),
	m_commandHash(commandHash),
	m_targetId(targetId),
	m_params(params),
	m_status(status),
	m_commandIsFromCommandQueue(commandIsFromCommandQueue)
{
}

// ----------------------------------------------------------------------

MessageQueueCommandQueueForceExecuteCommandFwd::MessageQueueCommandQueueForceExecuteCommandFwd(
	MessageQueueCommandQueueForceExecuteCommandFwd const &source) :
	MessageQueue::Data(),
	m_commandHash(source.m_commandHash),
	m_targetId(source.m_targetId),
	m_params(source.m_params),
	m_status(source.m_status),
	m_commandIsFromCommandQueue(source.m_commandIsFromCommandQueue)
{
}	

// ----------------------------------------------------------------------

uint32 MessageQueueCommandQueueForceExecuteCommandFwd::getCommandHash() const
{
	return m_commandHash;
}

// ----------------------------------------------------------------------

NetworkId const &MessageQueueCommandQueueForceExecuteCommandFwd::getTargetId() const
{
	return m_targetId;
}

// ----------------------------------------------------------------------

Unicode::String const &MessageQueueCommandQueueForceExecuteCommandFwd::getParams() const
{
	return m_params;
}

// ----------------------------------------------------------------------

int MessageQueueCommandQueueForceExecuteCommandFwd::getStatus() const
{
	return m_status;
}

// ----------------------------------------------------------------------

bool MessageQueueCommandQueueForceExecuteCommandFwd::getCommandIsFromCommandQueue() const
{
	return m_commandIsFromCommandQueue;
}

//-----------------------------------------------------------------------

void MessageQueueCommandQueueForceExecuteCommandFwd::pack(const MessageQueue::Data* const data, Archive::ByteStream &target)
{
	MessageQueueCommandQueueForceExecuteCommandFwd const* const msg = safe_cast<MessageQueueCommandQueueForceExecuteCommandFwd const*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getCommandHash());
		Archive::put(target, msg->getTargetId());
		Archive::put(target, msg->getParams());
		Archive::put(target, msg->getStatus());
		Archive::put(target, msg->getCommandIsFromCommandQueue());
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCommandQueueForceExecuteCommandFwd::unpack(Archive::ReadIterator &source)
{
	uint32 commandHash;
	NetworkId targetId;
	Unicode::String params;
	int status;
	bool commandIsFromCommandQueue;
	
	Archive::get(source, commandHash);
	Archive::get(source, targetId);
	Archive::get(source, params);
	Archive::get(source, status);
	Archive::get(source, commandIsFromCommandQueue);
	
	return new MessageQueueCommandQueueForceExecuteCommandFwd(commandHash, targetId, params, status, commandIsFromCommandQueue);
}

//-----------------------------------------------------------------------
