//======================================================================
//
// MessageQueueMissionCreateRequest.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionCreateRequest.h"

#include "sharedNetworkMessages/MessageQueueMissionCreateRequestData.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueMissionCreateRequestDataArchive.h"


//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueMissionCreateRequest, CM_missionCreateRequest);

//======================================================================

MessageQueueMissionCreateRequest::MessageQueueMissionCreateRequest (const MessageQueueMissionCreateRequestData & data, uint8 sequenceId) :
m_data (new MessageQueueMissionCreateRequestData (data)),
m_sequenceId (sequenceId)
{

}

//----------------------------------------------------------------------

MessageQueueMissionCreateRequest::~MessageQueueMissionCreateRequest ()
{
	delete m_data;
	m_data = 0;
}
//----------------------------------------------------------------------


void MessageQueueMissionCreateRequest::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueMissionCreateRequest* const msg = NON_NULL(safe_cast<const MessageQueueMissionCreateRequest*> (data));
	Archive::put(target, msg->getSequenceId ());
	Archive::put(target, msg->getData ());
}

//----------------------------------------------------------------------

MessageQueue::Data* MessageQueueMissionCreateRequest::unpack(Archive::ReadIterator & source)
{
	uint8 sequenceId = 0;
	MessageQueueMissionCreateRequestData data;
	Archive::get(source, sequenceId);
	Archive::get(source, data);
	return new MessageQueueMissionCreateRequest (data, sequenceId);
}

//----------------------------------------------------------------------
//======================================================================
