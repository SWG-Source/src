//========================================================================
//
// MessageQueueDirectDamage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueDirectDamage_H
#define INCLUDED_MessageQueueDirectDamage_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

/**
 */
class MessageQueueDirectDamage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueDirectDamage(int damageType, int hitLocation, int damageDone);
	virtual ~MessageQueueDirectDamage();
	
	MessageQueueDirectDamage&	operator=	(const MessageQueueDirectDamage & source);
	MessageQueueDirectDamage(const MessageQueueDirectDamage & source);

	int getDamageType() const;
	int getHitLocation() const;
	int getDamageDone() const;
	
private:
	int m_damageType;
	int m_hitLocation;
	int m_damageDone;
};


// ======================================================================

inline int MessageQueueDirectDamage::getDamageType() const
{
	return m_damageType;
}

inline int MessageQueueDirectDamage::getHitLocation() const
{
	return m_hitLocation;
}

inline int MessageQueueDirectDamage::getDamageDone() const
{
	return m_damageDone;
}


// ======================================================================


#endif	// INCLUDED_MessageQueueDirectDamage_H
