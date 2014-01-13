// MessageQueueMissionListRequest.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionListRequest.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueMissionListRequest, CM_missionListRequest);

//===================================================================


//-----------------------------------------------------------------------

MessageQueueMissionListRequest::MessageQueueMissionListRequest(const NetworkId & n, const uint8 f, const uint8 s) :
MessageQueue::Data(),
m_flags(f),
m_sequenceId(s),
m_terminalId(n)
{
}

//-----------------------------------------------------------------------

MessageQueueMissionListRequest::~MessageQueueMissionListRequest()
{
}

//-----------------------------------------------------------------------


void MessageQueueMissionListRequest::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueMissionListRequest* const message = safe_cast<const MessageQueueMissionListRequest*> (data);
	if(message)
	{
		Archive::put(target, message->getFlags());
		Archive::put(target, message->getSequenceId());
		Archive::put(target, message->getTerminalId());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueMissionListRequest::unpack(Archive::ReadIterator & source)
{
	unsigned char flags;
	unsigned char sequenceId;
	NetworkId terminalId;
	
	Archive::get(source, flags);
	Archive::get(source, sequenceId);
	Archive::get(source, terminalId);
	
	return new MessageQueueMissionListRequest(terminalId, flags, sequenceId);
}

//----------------------------------------------------------------------
