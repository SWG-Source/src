// ======================================================================
//
// MessageQueueCommandQueueEnqueue.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueEnqueue.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueCommandQueueEnqueue, CM_commandQueueEnqueue);

//===================================================================




// ----------------------------------------------------------------------

MessageQueueCommandQueueEnqueue::MessageQueueCommandQueueEnqueue(
	uint32 sequenceId, uint32 commandHash, NetworkId const &targetId,
	Unicode::String const &params) :
	MessageQueue::Data(),
	m_sequenceId(sequenceId),
	m_commandHash(commandHash),
	m_targetId(targetId),
	m_params(params)
{
}

// ----------------------------------------------------------------------

MessageQueueCommandQueueEnqueue::MessageQueueCommandQueueEnqueue(
	MessageQueueCommandQueueEnqueue const &source) :
	MessageQueue::Data(),
	m_sequenceId(source.m_sequenceId),
	m_commandHash(source.m_commandHash),
	m_targetId(source.m_targetId),
	m_params(source.m_params)
{
}	

// ----------------------------------------------------------------------

uint32 MessageQueueCommandQueueEnqueue::getSequenceId() const
{
	return m_sequenceId;
}

// ----------------------------------------------------------------------

uint32 MessageQueueCommandQueueEnqueue::getCommandHash() const
{
	return m_commandHash;
}

// ----------------------------------------------------------------------

NetworkId const &MessageQueueCommandQueueEnqueue::getTargetId() const
{
	return m_targetId;
}

// ----------------------------------------------------------------------

Unicode::String const &MessageQueueCommandQueueEnqueue::getParams() const
{
	return m_params;
}
//-----------------------------------------------------------------------

void MessageQueueCommandQueueEnqueue::pack(const MessageQueue::Data* const data, Archive::ByteStream &target)
{
	MessageQueueCommandQueueEnqueue const* const msg = safe_cast<MessageQueueCommandQueueEnqueue const*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getSequenceId());
		Archive::put(target, msg->getCommandHash());
		Archive::put(target, msg->getTargetId());
		Archive::put(target, msg->getParams());
	}
}
//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCommandQueueEnqueue::unpack(Archive::ReadIterator &source)
{
	uint32 sequenceId;
	uint32 commandHash;
	NetworkId targetId;
	Unicode::String params;
	
	Archive::get(source, sequenceId);
	Archive::get(source, commandHash);
	Archive::get(source, targetId);
	Archive::get(source, params);
	
	return new MessageQueueCommandQueueEnqueue(sequenceId, commandHash, targetId, params);
}

//-----------------------------------------------------------------------
