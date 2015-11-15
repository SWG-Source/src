// MessageQueueMissionListResponse.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponseArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueMissionListResponse, CM_missionListResponse);

//===================================================================


//----------------------------------------------------------------------

MessageQueueMissionListResponse::MessageQueueMissionListResponse() :
m_response(),
m_sequenceId(0),
m_bountyTerminal(false)
{
}

//-----------------------------------------------------------------------

MessageQueueMissionListResponse::MessageQueueMissionListResponse(const DataVector & v, const uint8 s, const bool b) :
m_response(v), 
m_sequenceId(s),
m_bountyTerminal(b)
{
}

//-----------------------------------------------------------------------

MessageQueueMissionListResponse::~MessageQueueMissionListResponse()
{
}

//----------------------------------------------------------------------

void MessageQueueMissionListResponse::set(const DataVector & response, const uint8 sequenceId, const bool b)
{
	m_response = response;
	m_sequenceId = sequenceId;
	m_bountyTerminal = b;
}

//-----------------------------------------------------------------------

void MessageQueueMissionListResponse::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueMissionListResponse* const message = safe_cast<const MessageQueueMissionListResponse*> (data);
	if(message)
		Archive::put(target, *message);
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueMissionListResponse::unpack(Archive::ReadIterator & source)
{
	MessageQueueMissionListResponse* const rsp = new MessageQueueMissionListResponse;
	Archive::get(source, *rsp);
	return rsp;
}

//----------------------------------------------------------------------
