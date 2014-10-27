// ======================================================================
//
// MessageQueueCombatDamage.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "MessageQueueCombatDamage.h"

#include <vector>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueCombatDamage, CM_combatDamage);

//===================================================================


// ======================================================================

/**
 * Class constructor.
 */
MessageQueueCombatDamage::MessageQueueCombatDamage(void)
:	m_actionId(0),
	m_defender(),
	m_defenderAttributes(new CurrentAttributeVector)
{
}

// ----------------------------------------------------------------------

/**
 * Class destructor.
 */
MessageQueueCombatDamage::~MessageQueueCombatDamage()
{
	delete m_defenderAttributes;
}

// ----------------------------------------------------------------------

void MessageQueueCombatDamage::addDefenderAttribute(
	Attributes::Enumerator attribute, Attributes::Value value)
{
	CurrentAttribute currentAttribute;
	currentAttribute.attribute = attribute;
	currentAttribute.value = value;
	m_defenderAttributes->push_back(currentAttribute);
}


void MessageQueueCombatDamage::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCombatDamage* const msg = safe_cast<const MessageQueueCombatDamage*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getActionId());
		Archive::put(target, msg->getDefender());
		
		const std::vector<MessageQueueCombatDamage::CurrentAttribute> & attribs = 
			msg->getDefenderAttributes();
		uint16 count = static_cast<uint16>(attribs.size());
		Archive::put(target, count);
		for (std::vector<MessageQueueCombatDamage::CurrentAttribute>::const_iterator
			iter = attribs.begin(); iter != attribs.end(); ++iter)
		{
			Archive::put(target, (*iter).attribute);
			Archive::put(target, (*iter).value);
		}
	}
}	

//-----------------------------------------------------------------------


MessageQueue::Data* MessageQueueCombatDamage::unpack(Archive::ReadIterator & source)
{
	uint16 actionId;
	NetworkId defenderId;
	Attributes::Enumerator attribute;
	Attributes::Value value;
	
	MessageQueueCombatDamage * msg = new MessageQueueCombatDamage;	
	
	Archive::get(source, actionId);
	msg->setActionId(actionId);
	
	Archive::get(source, defenderId);
	msg->setDefender(defenderId);
	
	uint16 count;
	Archive::get(source, count);
	for (uint16 i = 0; i < count; ++i)
	{
		Archive::get(source, attribute);
		Archive::get(source, value);
		msg->addDefenderAttribute(attribute, value);
	}
	
	return msg;
}	
//-----------------------------------------------------------------------
// ======================================================================
