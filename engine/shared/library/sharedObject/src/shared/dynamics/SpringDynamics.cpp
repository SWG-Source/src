//===================================================================
//
// SpringDynamics.cpp
// asommers 6-19-99
//
// copyright 1999, bootprint entertainment
//
//===================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/SpringDynamics.h"

#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

//===================================================================
// PUBLIC SpringDynamics
//===================================================================

SpringDynamics::SpringDynamics (Object* const owner, const Vector& offset, const Vector& springDirection, float springMagnitude, float springsPerSecond) :
	StateDynamics (owner),
	m_parent (0),
	m_offset (offset),
	m_springDirection (springDirection),
	m_springMagnitude (springMagnitude),
	m_springsPerSecond (springsPerSecond),
	m_dy (Random::randomReal (0.0f, PI_TIMES_2)),
	m_position ()
{
}

//-------------------------------------------------------------------

SpringDynamics::~SpringDynamics ()
{
	// the parent object isn't owned by this instance
	m_parent = 0;
}

//-------------------------------------------------------------------

void SpringDynamics::setPosition (const Vector& position)
{
	m_position = position;
}

//===================================================================
// PROTECTED SpringDynamics
//===================================================================

void SpringDynamics::realAlter (const float time)
{
	StateDynamics::realAlter (time);

	Object* const owner = getOwner ();
	if (!owner)
		return;

	if (m_springsPerSecond > 0.f)
	{
		m_dy += time;

		const Vector springDirection = owner->rotate_o2p (m_springDirection) * sin (m_springsPerSecond * m_dy * 8.0f) * m_springMagnitude;
		owner->setPosition_p (m_position + m_offset + springDirection);
	}
}

//===================================================================

