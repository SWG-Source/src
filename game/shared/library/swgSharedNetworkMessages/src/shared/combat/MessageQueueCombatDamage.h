//========================================================================
//
// MessageQueueCombatDamage.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCombatDamage_H
#define INCLUDED_MessageQueueCombatDamage_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "swgSharedUtility/Attributes.def"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
class MessageQueueCombatDamage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	struct CurrentAttribute
	{
		Attributes::Enumerator attribute;
		Attributes::Value      value;
	};

	typedef std::vector<CurrentAttribute>  CurrentAttributeVector;

public:
	         MessageQueueCombatDamage(void);
	virtual ~MessageQueueCombatDamage();

	MessageQueueCombatDamage&	operator=	(const MessageQueueCombatDamage & source);
	MessageQueueCombatDamage(const MessageQueueCombatDamage & source);

	uint16 getActionId(void) const;
	void setActionId(uint16 id);
	NetworkId getDefender(void) const;
	void setDefender(NetworkId defenderId);
	const CurrentAttributeVector & getDefenderAttributes(void) const;
	void addDefenderAttribute(Attributes::Enumerator attribute, 
		Attributes::Value value);

private:
	uint16                  m_actionId;
	NetworkId               m_defender;
	CurrentAttributeVector *m_defenderAttributes;
};


inline uint16 MessageQueueCombatDamage::getActionId(void) const
{
	return m_actionId;
}	// MessageQueueCombatDamage::getActionId

inline void MessageQueueCombatDamage::setActionId(uint16 id)
{
	m_actionId = id;
}	// MessageQueueCombatDamage::setActionId

inline NetworkId MessageQueueCombatDamage::getDefender(void) const
{
	return m_defender;
}	// MessageQueueCombatDamage::getDefender

inline void MessageQueueCombatDamage::setDefender(NetworkId defenderId)
{
	m_defender = defenderId;
}	// MessageQueueCombatDamage::setDefender

inline const MessageQueueCombatDamage::CurrentAttributeVector & 
	MessageQueueCombatDamage::getDefenderAttributes(void) const
{
	return *m_defenderAttributes;
}	// MessageQueueCombatDamage::getDefenderAttributes

#endif	// INCLUDED_MessageQueueCombatDamage_H
