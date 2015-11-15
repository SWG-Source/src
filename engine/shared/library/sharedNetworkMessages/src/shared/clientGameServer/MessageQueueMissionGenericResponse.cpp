//======================================================================
//
// MessageQueueMissionGenericResponse.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericResponse.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericResponseArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(MessageQueueMissionGenericResponse);

//===================================================================
void MessageQueueMissionGenericResponse::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_missionAcceptResponse, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_missionRemoveResponse, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_missionCreateResponse, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueMissionGenericResponse::remove"); 
} 


//======================================================================

MessageQueueMissionGenericResponse::MessageQueueMissionGenericResponse () :
m_missionObjectId (new NetworkId),
m_success (false),
m_sequenceId (0)
{
}

//----------------------------------------------------------------------

MessageQueueMissionGenericResponse::MessageQueueMissionGenericResponse (const NetworkId & id, bool success, uint8 sequenceId) :
m_missionObjectId (new NetworkId (id)),
m_success (success),
m_sequenceId (sequenceId)
{
}

//----------------------------------------------------------------------

MessageQueueMissionGenericResponse::~MessageQueueMissionGenericResponse ()
{
	delete m_missionObjectId;
	m_missionObjectId = 0;
}

//----------------------------------------------------------------------

void MessageQueueMissionGenericResponse::set (const NetworkId & missionObjectId, bool success, uint8 sequenceId)
{
	setMissionObjectId (missionObjectId);
	setSuccess (success);
	m_sequenceId                = sequenceId;
}

//----------------------------------------------------------------------

void MessageQueueMissionGenericResponse::setSuccess (bool b)
{
	m_success                   = b;
}

//----------------------------------------------------------------------

void MessageQueueMissionGenericResponse::setMissionObjectId (const NetworkId & missionObjectId)
{
	*NON_NULL(m_missionObjectId) = missionObjectId;
}
//----------------------------------------------------------------------


void MessageQueueMissionGenericResponse::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	Archive::put(target, *NON_NULL(safe_cast<const MessageQueueMissionGenericResponse*> (data)));
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueMissionGenericResponse::unpack(Archive::ReadIterator & source)
{
	MessageQueueMissionGenericResponse* const rsp = new MessageQueueMissionGenericResponse;
	Archive::get(source, *rsp);
	return rsp;
}



//======================================================================
