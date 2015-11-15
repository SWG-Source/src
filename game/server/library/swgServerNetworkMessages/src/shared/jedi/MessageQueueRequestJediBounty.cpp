// ======================================================================
//
// MessageQueueRequestJediBounty.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "swgServerNetworkMessages/MessageQueueRequestJediBounty.h"

#include <string>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueRequestJediBounty, CM_requestJediBounty);

//===================================================================


// ----------------------------------------------------------------------

MessageQueueRequestJediBounty::MessageQueueRequestJediBounty(const NetworkId & targetId, 
	const NetworkId & hunterId, const std::string & successCallback, 
	const std::string & failCallback, const NetworkId & callbackObjectId) :
	m_targetId(targetId),
	m_hunterId(hunterId),
	m_successCallback(successCallback),
	m_failCallback(failCallback),
	m_callbackObjectId(callbackObjectId)
{
}	

// ----------------------------------------------------------------------

MessageQueueRequestJediBounty::~MessageQueueRequestJediBounty()
{
	
}

void MessageQueueRequestJediBounty::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueRequestJediBounty* const msg = safe_cast<const MessageQueueRequestJediBounty*> (data);
	if (msg)
	{
		Archive::put(target, msg->m_targetId);
		Archive::put(target, msg->m_hunterId);
		Archive::put(target, msg->m_successCallback);
		Archive::put(target, msg->m_failCallback);
		Archive::put(target, msg->m_callbackObjectId);
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueRequestJediBounty::unpack(Archive::ReadIterator & source)
{
	NetworkId targetId;
	NetworkId hunterId;
	std::string successCallback;
	std::string failCallback;
	NetworkId callbackObjectId;

	Archive::get(source, targetId);
	Archive::get(source, hunterId);
	Archive::get(source, successCallback);
	Archive::get(source, failCallback);
	Archive::get(source, callbackObjectId);
	
	return new MessageQueueRequestJediBounty(targetId, hunterId, successCallback, 
		failCallback, callbackObjectId);
}


// ======================================================================
