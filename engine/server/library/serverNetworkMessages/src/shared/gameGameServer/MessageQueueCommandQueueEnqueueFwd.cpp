// ======================================================================
//
// MessageQueueCommandQueueEnqueueFwd.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageQueueCommandQueueEnqueueFwd.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueCommandQueueEnqueueFwd, CM_commandQueueEnqueueFwd);

//===================================================================



MessageQueueCommandQueueEnqueueFwd::MessageQueueCommandQueueEnqueueFwd(
	uint32 sequenceId, uint32 commandHash, NetworkId const &targetId,
	Unicode::String const &params, bool clearable, int priority, bool fromServer) :
	MessageQueue::Data(),
	m_sequenceId(sequenceId),
	m_commandHash(commandHash),
	m_targetId(targetId),
	m_params(params),
	m_clearable(clearable),
	m_priority(priority),
	m_fromServer(fromServer)
{
}

// ----------------------------------------------------------------------

MessageQueueCommandQueueEnqueueFwd::MessageQueueCommandQueueEnqueueFwd(
	MessageQueueCommandQueueEnqueueFwd const &source) :
	MessageQueue::Data(),
	m_sequenceId(source.m_sequenceId),
	m_commandHash(source.m_commandHash),
	m_targetId(source.m_targetId),
	m_params(source.m_params),
	m_clearable(source.m_clearable),
	m_priority(source.m_priority),
	m_fromServer(source.m_fromServer)
{
}	

// ----------------------------------------------------------------------

uint32 MessageQueueCommandQueueEnqueueFwd::getSequenceId() const
{
	return m_sequenceId;
}

// ----------------------------------------------------------------------

uint32 MessageQueueCommandQueueEnqueueFwd::getCommandHash() const
{
	return m_commandHash;
}

// ----------------------------------------------------------------------

NetworkId const &MessageQueueCommandQueueEnqueueFwd::getTargetId() const
{
	return m_targetId;
}

// ----------------------------------------------------------------------

Unicode::String const &MessageQueueCommandQueueEnqueueFwd::getParams() const
{
	return m_params;
}

// ----------------------------------------------------------------------

bool MessageQueueCommandQueueEnqueueFwd::getClearable() const
{
	return m_clearable;
}

// ----------------------------------------------------------------------

int MessageQueueCommandQueueEnqueueFwd::getPriority() const
{
	return m_priority;
}

// ----------------------------------------------------------------------

bool MessageQueueCommandQueueEnqueueFwd::getFromServer() const
{
	return m_fromServer;
}

//-----------------------------------------------------------------------

void MessageQueueCommandQueueEnqueueFwd::pack(const MessageQueue::Data* const data, Archive::ByteStream &target)
{
	MessageQueueCommandQueueEnqueueFwd const* const msg = safe_cast<MessageQueueCommandQueueEnqueueFwd const*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getSequenceId());
		Archive::put(target, msg->getCommandHash());
		Archive::put(target, msg->getTargetId());
		Archive::put(target, msg->getParams());
		Archive::put(target, msg->getClearable());
		Archive::put(target, msg->getPriority());
		Archive::put(target, msg->getFromServer());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCommandQueueEnqueueFwd::unpack(Archive::ReadIterator &source)
{
	uint32 sequenceId;
	uint32 commandHash;
	NetworkId targetId;
	Unicode::String params;
	bool clearable;
	int priority;
	bool fromServer;
	
	Archive::get(source, sequenceId);
	Archive::get(source, commandHash);
	Archive::get(source, targetId);
	Archive::get(source, params);
	Archive::get(source, clearable);
	Archive::get(source, priority);
	Archive::get(source, fromServer);
	
	return new MessageQueueCommandQueueEnqueueFwd(sequenceId, commandHash, targetId, params, clearable, priority, fromServer);
}

//-----------------------------------------------------------------------
