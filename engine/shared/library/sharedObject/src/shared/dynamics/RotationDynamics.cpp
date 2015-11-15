//===================================================================
//
// RotationDynamics.cpp
// asommers 6-19-99
//
// copyright 1999, bootprint entertainment
//
//===================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/RotationDynamics.h"

#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

//===================================================================
// PUBLIC RotationDynamics
//===================================================================

RotationDynamics::RotationDynamics (Object* const owner, const Vector& rotationsPerSecond) :
	StateDynamics (owner),
	m_rotationsPerSecond (rotationsPerSecond),
	m_rotateAroundAppearanceCenter(false)
{
}

//-------------------------------------------------------------------

RotationDynamics::~RotationDynamics ()
{
}

//----------------------------------------------------------------------

void RotationDynamics::setRotateAroundAppearanceCenter(bool rotateAroundAppearanceCenter)
{
	m_rotateAroundAppearanceCenter = rotateAroundAppearanceCenter;
}

//===================================================================
// PROTECTED RotationDynamics
//===================================================================

void RotationDynamics::realAlter (const float elapsedTime)
{
	StateDynamics::realAlter (elapsedTime);

	Object* const owner = getOwner ();
	if (!owner)
		return;

	if (m_rotateAroundAppearanceCenter)
	{
		Transform t = owner->getTransform_o2p();
		t.move_l(owner->getAppearanceSphereCenter());
		t.yaw_l(m_rotationsPerSecond.x * elapsedTime);
		t.pitch_l(m_rotationsPerSecond.y * elapsedTime);
		t.roll_l(m_rotationsPerSecond.z * elapsedTime);
		t.move_l(-owner->getAppearanceSphereCenter());

		owner->setTransform_o2p(t);
	}
	else
	{
		owner->yaw_o   (m_rotationsPerSecond.x * elapsedTime);
		owner->pitch_o (m_rotationsPerSecond.y * elapsedTime);
		owner->roll_o  (m_rotationsPerSecond.z * elapsedTime);
	}
}

//===================================================================
