//========================================================================
//
// MessageQueuePushCreature.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueuePushCreature_H
#define INCLUDED_MessageQueuePushCreature_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//----------------------------------------------------------------------

class MessageQueuePushCreature : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueuePushCreature(const NetworkId & attacker, const NetworkId & defender, const Vector & attackerPos, const Vector & defenderPos, float distance);
	virtual ~MessageQueuePushCreature();

	const NetworkId & getAttacker() const;
	const NetworkId & getDefender() const;
	const Vector & getAttackerPos() const;
	const Vector & getDefenderPos() const;
	float getDistance() const;

private:
	NetworkId m_attacker; 
	NetworkId m_defender; 
	Vector    m_attackerPos; 
	Vector    m_defenderPos;
	float     m_distance;
};


//----------------------------------------------------------------------

inline const NetworkId & MessageQueuePushCreature::getAttacker() const
{
	return m_attacker;
}

//----------------------------------------------------------------------

inline const NetworkId & MessageQueuePushCreature::getDefender() const
{
	return m_defender;
}

//----------------------------------------------------------------------

inline const Vector & MessageQueuePushCreature::getAttackerPos() const
{
	return m_attackerPos;
}

//----------------------------------------------------------------------

inline const Vector & MessageQueuePushCreature::getDefenderPos() const
{
	return m_defenderPos;
}

//----------------------------------------------------------------------

inline float MessageQueuePushCreature::getDistance() const
{
	return m_distance;
}

//----------------------------------------------------------------------

#endif	// INCLUDED_MessageQueuePushCreature_H
