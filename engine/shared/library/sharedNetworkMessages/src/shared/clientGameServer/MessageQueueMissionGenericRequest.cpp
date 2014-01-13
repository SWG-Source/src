//======================================================================
//
// MessageQueueMissionGenericRequest.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericRequest.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericRequestArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueMissionGenericRequest);

//===================================================================
void MessageQueueMissionGenericRequest::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_missionAcceptRequest, pack, unpack, true); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_missionRemoveRequest, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueMissionGenericRequest::remove"); 
} 

//======================================================================

MessageQueueMissionGenericRequest::MessageQueueMissionGenericRequest () :
m_missionObjectId (new NetworkId),
m_terminalId (new NetworkId),
m_sequenceId (0)
{
}

//----------------------------------------------------------------------

MessageQueueMissionGenericRequest::MessageQueueMissionGenericRequest (const NetworkId & missionObjectId, const NetworkId & terminalId, uint8 sequenceId) :
m_missionObjectId (new NetworkId (missionObjectId)),
m_terminalId (new NetworkId (terminalId)),
m_sequenceId (sequenceId)
{
}

//----------------------------------------------------------------------

MessageQueueMissionGenericRequest::~MessageQueueMissionGenericRequest ()
{
	delete m_missionObjectId;
	m_missionObjectId = 0;
	delete m_terminalId;
	m_terminalId = 0;
}

//----------------------------------------------------------------------

void MessageQueueMissionGenericRequest::set (const NetworkId & missionObjectId, const NetworkId & terminalId, uint8 sequenceId)
{
	*NON_NULL (m_missionObjectId) = missionObjectId;
	*NON_NULL (m_terminalId)    = terminalId;
	m_sequenceId                = sequenceId;
}
// ----------------------------------------------------------------------


void MessageQueueMissionGenericRequest::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	Archive::put(target, *NON_NULL(safe_cast<const MessageQueueMissionGenericRequest*> (data)));
}

//----------------------------------------------------------------------


MessageQueue::Data* MessageQueueMissionGenericRequest::unpack(Archive::ReadIterator & source)
{
	MessageQueueMissionGenericRequest* const req = new MessageQueueMissionGenericRequest;
	Archive::get(source, *req);
	return req;
}

//======================================================================
