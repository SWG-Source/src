// ======================================================================
//
// TrackingDynamics.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/TrackingDynamics.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(TrackingDynamics, true, 0, 0, 0);

// ======================================================================

TrackingDynamics::TrackingDynamics(Object *owner, Object const *targetObject, float speed, Vector const &rotationRadiansPerSecond) :
	SimpleDynamics(owner),
	m_targetWatcher(targetObject),
	m_speed(speed),
	m_radiansPerSecond(rotationRadiansPerSecond)
{
}

// ----------------------------------------------------------------------

float TrackingDynamics::alter(float elapsedTime)
{
	Object *const ownerObject = getOwner();
	if (ownerObject)
	{
		Object const *const targetObject = m_targetWatcher.getPointer();
		if (targetObject)
		{
			//-- Get world position of owner and target.
			Vector const ownerPosition_w  = ownerObject->getPosition_w();
			Vector const targetPosition_w = targetObject->getPosition_w();

			//-- Compute world-space direction vector.
			Vector direction_w = targetPosition_w - ownerPosition_w;
			IGNORE_RETURN(direction_w.normalize());

			//-- Set current velocity.
			setCurrentVelocity_w(direction_w * m_speed);
		}

		//-- Handle object rotation.
		ownerObject->yaw_o   (m_radiansPerSecond.y * elapsedTime);
		ownerObject->pitch_o (m_radiansPerSecond.x * elapsedTime);
		ownerObject->roll_o  (m_radiansPerSecond.z * elapsedTime);
	}

	//-- Chain up alter.  This will handle the object translation.
	return SimpleDynamics::alter(elapsedTime);
}

// ----------------------------------------------------------------------

void TrackingDynamics::setTarget(Object const *targetObject)
{
	m_targetWatcher = targetObject;
}

// ----------------------------------------------------------------------

float TrackingDynamics::getDistanceToTargetSquared() const
{
	Object const *const ownerObject = getOwner();
	if (ownerObject)
	{
		Object const *const targetObject = m_targetWatcher.getPointer();
		if (targetObject)
		{
			//-- Get world position of owner and target.
			Vector const ownerPosition_w  = ownerObject->getPosition_w();
			Vector const targetPosition_w = targetObject->getPosition_w();

			return ownerPosition_w.magnitudeBetweenSquared(targetPosition_w);
		}
	}

	return 0.0f;
}

// ======================================================================
