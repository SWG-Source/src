//===================================================================
//
// SeesawDynamics.cpp
// asommers 6-19-99
//
// copyright 1999, bootprint entertainment
//
//===================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/SeesawDynamics.h"

#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

//===================================================================
// PUBLIC SeesawDynamics
//===================================================================

SeesawDynamics::SeesawDynamics (Object* const owner, const Type type, const float seesawsPerSecond, const float seesawMagnitude) :
	StateDynamics (owner),
	m_type (type),
	m_seesawsPerSecond (seesawsPerSecond),
	m_seesawMagnitude (seesawMagnitude),
	m_dy (Random::randomReal (0.0f, PI_TIMES_2))
{
}

//-------------------------------------------------------------------

SeesawDynamics::~SeesawDynamics ()
{
}

//-------------------------------------------------------------------

void SeesawDynamics::realAlter (const float elapsedTime)
{
	StateDynamics::realAlter (elapsedTime);

	Object* const owner = getOwner ();
	if (!owner)
		return;

	//-- rotation
	m_dy += elapsedTime;

	const float t     = (sin (m_seesawsPerSecond * m_dy) + 1.f) * 0.5f;
	const float angle = linearInterpolate (-m_seesawMagnitude, m_seesawMagnitude, t);

	owner->resetRotate_o2p ();

	switch (m_type)
	{
	case T_yaw:    owner->yaw_o (angle);    break;
	case T_pitch:  owner->pitch_o (angle);  break;
	case T_roll:   owner->roll_o (angle);   break;
	}
}

//===================================================================
