// ======================================================================
//
// MessageQueueGrantOfflineXp.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageQueueGrantOfflineXp.h"

#include <string>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueGrantOfflineXp, CM_grantOfflineXp);

//===================================================================


// ----------------------------------------------------------------------

MessageQueueGrantOfflineXp::MessageQueueGrantOfflineXp(const NetworkId & player, 
	const std::string & experienceType, int amount) :
	m_playerId(player),
	m_xpType(experienceType),
	m_xpAmount(amount)
{
}	

// ----------------------------------------------------------------------

MessageQueueGrantOfflineXp::~MessageQueueGrantOfflineXp()
{
	
}

void MessageQueueGrantOfflineXp::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueGrantOfflineXp* const msg = safe_cast<const MessageQueueGrantOfflineXp*> (data);
	if (msg)
	{
		Archive::put(target, msg->m_playerId);
		Archive::put(target, msg->m_xpType);
		Archive::put(target, msg->m_xpAmount);
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueGrantOfflineXp::unpack(Archive::ReadIterator & source)
{
	NetworkId player;
	std::string experienceType;
	int amount;

	Archive::get(source, player);
	Archive::get(source, experienceType);
	Archive::get(source, amount);
	
	return new MessageQueueGrantOfflineXp(player, experienceType, amount);
}


// ======================================================================
