// MessageQueueMissionDetailsRequest.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsRequest.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueMissionDetailsRequest, CM_missionDetailsRequest);

//===================================================================


//-----------------------------------------------------------------------

MessageQueueMissionDetailsRequest::MessageQueueMissionDetailsRequest(const NetworkId & m, const NetworkId & t, const unsigned char s) :
MessageQueue::Data(),
missionData(m),
terminalId(t),
sequenceId(s)
{
}

//-----------------------------------------------------------------------

MessageQueueMissionDetailsRequest::~MessageQueueMissionDetailsRequest()
{
}

//-----------------------------------------------------------------------


void MessageQueueMissionDetailsRequest::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueMissionDetailsRequest* const message = safe_cast<const MessageQueueMissionDetailsRequest*> (data);
	if(message)
	{
		Archive::put(target, message->getMissionData());
		Archive::put(target, message->getTerminalId());
		Archive::put(target, message->getSequenceId());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueMissionDetailsRequest::unpack(Archive::ReadIterator & source)
{
	NetworkId missionData;
	NetworkId terminalId;
	unsigned char sequenceId;
	
	Archive::get(source, missionData);
	Archive::get(source, terminalId);
	Archive::get(source, sequenceId);
	
	return new MessageQueueMissionDetailsRequest(missionData, terminalId, sequenceId);
}

//-----------------------------------------------------------------------
