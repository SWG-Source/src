// ======================================================================
//
// MessageQueueCommandQueueRemove.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueRemove.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

// ======================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueCommandQueueRemove, CM_commandQueueRemove);

//===================================================================

MessageQueueCommandQueueRemove::MessageQueueCommandQueueRemove(uint32 sequenceId, float waitTime, int status, int statusDetail) :
	MessageQueue::Data(),
	m_sequenceId(sequenceId),
	m_waitTime(waitTime),
	m_status(status),
	m_statusDetail(statusDetail)
{
}

// ----------------------------------------------------------------------

MessageQueueCommandQueueRemove::MessageQueueCommandQueueRemove(MessageQueueCommandQueueRemove const &source) :
	MessageQueue::Data(),
	m_sequenceId(source.m_sequenceId),
	m_waitTime(source.m_waitTime),
	m_status(source.m_status),
	m_statusDetail(source.m_statusDetail)
{
}	

// ----------------------------------------------------------------------

uint32 MessageQueueCommandQueueRemove::getSequenceId() const
{
	return m_sequenceId;
}

// ----------------------------------------------------------------------

float MessageQueueCommandQueueRemove::getWaitTime() const
{
	return m_waitTime;
}

// ----------------------------------------------------------------------

int MessageQueueCommandQueueRemove::getStatus() const
{
	return m_status;
}

// ----------------------------------------------------------------------

int MessageQueueCommandQueueRemove::getStatusDetail() const
{
	return m_statusDetail;
}

// ----------------------------------------------------------------------

void MessageQueueCommandQueueRemove::pack(const MessageQueue::Data* const data, Archive::ByteStream &target)
{
	MessageQueueCommandQueueRemove const* const msg = safe_cast<MessageQueueCommandQueueRemove const*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getSequenceId());
		Archive::put(target, msg->getWaitTime());
		Archive::put(target, msg->getStatus());
		Archive::put(target, msg->getStatusDetail());
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCommandQueueRemove::unpack(Archive::ReadIterator &source)
{
	uint32 sequenceId;
	float waitTime;
	int status, statusDetail;
	
	Archive::get(source, sequenceId);
	Archive::get(source, waitTime);
	Archive::get(source, status);
	Archive::get(source, statusDetail);
	
	return new MessageQueueCommandQueueRemove(sequenceId, waitTime, status, statusDetail);
}

// ======================================================================

