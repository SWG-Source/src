// ======================================================================
//
// PulseDynamics.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/PulseDynamics.h"

#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

//======================================================================

PulseDynamics::PulseDynamics(Object *owner, float minimumSize, float maximumSize, float speed)
: SimpleDynamics(owner)
, m_minimumSize(minimumSize)
, m_maximumSize(maximumSize)
, m_speed(speed)
, m_baseScale(owner->getScale())
, m_currentSize(1.0f)
, m_pulseGrowDirection(1.0f)
{
}

// ----------------------------------------------------------------------

PulseDynamics::~PulseDynamics()
{
	Object *const ownerObject = getOwner();
	if (ownerObject != 0)
	{
		ownerObject->setRecursiveScale(m_baseScale);
	}
}

// ----------------------------------------------------------------------

float PulseDynamics::alter(float elapsedTime)
{
	Object *const ownerObject = getOwner();
	if (ownerObject != 0)
	{
		m_currentSize += (elapsedTime * m_speed * m_pulseGrowDirection);

		if (!WithinRangeInclusiveInclusive(m_minimumSize, m_currentSize, m_maximumSize))
		{
			m_currentSize = clamp(m_minimumSize, m_currentSize, m_maximumSize);
			m_pulseGrowDirection *= -1.0f;	
		}

		Vector scale = m_baseScale * m_currentSize;
		ownerObject->setRecursiveScale(scale);
	}

	return SimpleDynamics::alter(elapsedTime);
}

// ======================================================================
