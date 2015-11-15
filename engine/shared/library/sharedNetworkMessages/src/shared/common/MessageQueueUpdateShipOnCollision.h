//======================================================================
//
// MessageQueueUpdateShipOnCollision.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueUpdateShipOnCollision_H
#define INCLUDED_MessageQueueUpdateShipOnCollision_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedMath/Vector.h"

#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

//----------------------------------------------------------------------

class MessageQueueUpdateShipOnCollision : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueUpdateShipOnCollision(Vector const & deltaToMove, Vector const & newVelocity, float const clampedStrengthOfImpact);
	virtual ~MessageQueueUpdateShipOnCollision();

	Vector const & getDeltaToMove() const;
	Vector const & getNewVelocity() const;
	float getClampedStrengthOfImpact() const;
private:
	Vector m_deltaToMove;
	Vector m_newVelocity;
	float m_clampedStrengthOfImpact;
};

//======================================================================

inline Vector const & MessageQueueUpdateShipOnCollision::getDeltaToMove() const
{
	return(m_deltaToMove);
}

inline Vector const & MessageQueueUpdateShipOnCollision::getNewVelocity() const
{
	return(m_newVelocity);
}

inline float MessageQueueUpdateShipOnCollision::getClampedStrengthOfImpact() const
{
	return(m_clampedStrengthOfImpact);
}

#endif
