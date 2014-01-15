// ======================================================================
//
// MessageQueueDirectDamage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "swgServerNetworkMessages/MessageQueueDirectDamage.h"

#include <string>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueDirectDamage, CM_directDamage);

//===================================================================


// ----------------------------------------------------------------------

MessageQueueDirectDamage::MessageQueueDirectDamage(int damageType, int hitLocation, int damageDone) :
	m_damageType(damageType), 
	m_hitLocation(hitLocation),
	m_damageDone(damageDone)
{
}	

// ----------------------------------------------------------------------

MessageQueueDirectDamage::~MessageQueueDirectDamage()
{
	
}

void MessageQueueDirectDamage::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueDirectDamage* const msg = safe_cast<const MessageQueueDirectDamage*> (data);
	if (msg)
	{
		Archive::put(target, msg->m_damageType);
		Archive::put(target, msg->m_hitLocation);
		Archive::put(target, msg->m_damageDone);
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueDirectDamage::unpack(Archive::ReadIterator & source)
{
	int damageType, hitLocation, damageDone;
	
	Archive::get(source, damageType);
	Archive::get(source, hitLocation);
	Archive::get(source, damageDone);
	
	return new MessageQueueDirectDamage(damageType, hitLocation, damageDone);
}


// ======================================================================
