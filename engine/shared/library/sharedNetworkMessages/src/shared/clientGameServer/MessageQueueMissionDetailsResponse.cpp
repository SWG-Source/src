// MessageQueueMissionDetailsResponse.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponse.h"

#include "sharedNetworkMessages/MessageQueueMissionDetailsResponseData.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponseDataArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueMissionDetailsResponse, CM_missionDetailsResponse);

//===================================================================


//-----------------------------------------------------------------------

MessageQueueMissionDetailsResponse::MessageQueueMissionDetailsResponse() :
m_data(new MessageQueueMissionDetailsResponseData),
m_sequenceId(0)
{
}

//----------------------------------------------------------------------

MessageQueueMissionDetailsResponse::MessageQueueMissionDetailsResponse(const MessageQueueMissionDetailsResponseData & data, const uint8 sequenceId) :
m_data(new MessageQueueMissionDetailsResponseData (data)),
m_sequenceId(sequenceId)
{
}
//-----------------------------------------------------------------------

MessageQueueMissionDetailsResponse::~MessageQueueMissionDetailsResponse()
{
	delete m_data;
	m_data = 0;
}

//----------------------------------------------------------------------

void MessageQueueMissionDetailsResponse::set(const MessageQueueMissionDetailsResponseData & data, const uint8 sequenceId)
{
	*NON_NULL(m_data) = data;
	m_sequenceId = sequenceId;
}

//-----------------------------------------------------------------------

void MessageQueueMissionDetailsResponse::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueMissionDetailsResponse* const message = safe_cast<const MessageQueueMissionDetailsResponse*> (data);
	if(message)
	{
		Archive::put(target, message->getSequenceId());
		Archive::put(target, message->getData());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueMissionDetailsResponse::unpack(Archive::ReadIterator & source)
{
	uint8 sequenceId;
	MessageQueueMissionDetailsResponseData data;
	
	Archive::get(source, sequenceId);
	Archive::get(source, data);
	
	return new MessageQueueMissionDetailsResponse(data, sequenceId);
}

//-----------------------------------------------------------------------
