//===================================================================
//
// SimpleDynamics.cpp
// copyright 1999, bootprint entertainment
// copyright 2002, sony online entertainment
//
//===================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/SimpleDynamics.h"

#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

//===================================================================

SimpleDynamics::SimpleDynamics (Object* const owner) : 
	Dynamics (owner),
	m_currentVelocity (),
	m_movementSpace (WorldSpace)
{
}

//-------------------------------------------------------------------

SimpleDynamics::~SimpleDynamics ()
{
}

//-------------------------------------------------------------------

Vector const &SimpleDynamics::getCurrentVelocity_w() const
{
	FATAL(m_movementSpace != WorldSpace && m_currentVelocity != Vector::zero, ("SimpleDynamics::getCurrentVelocity_w called when velocity is not in world space"));
	return m_currentVelocity;
}

//-------------------------------------------------------------------

void SimpleDynamics::setCurrentVelocity_w(Vector const &newCurrentVelocity_w)
{
	m_movementSpace = WorldSpace;
	m_currentVelocity = newCurrentVelocity_w;
}

//-------------------------------------------------------------------

Vector const &SimpleDynamics::getCurrentVelocity_p() const
{
	FATAL(m_movementSpace != ParentSpace && m_currentVelocity != Vector::zero, ("SimpleDynamics::getCurrentVelocity_p called when velocity is not in parent space"));
	return m_currentVelocity;
}

//-------------------------------------------------------------------

void SimpleDynamics::setCurrentVelocity_p(Vector const &newCurrentVelocity_p)
{
	m_movementSpace = ParentSpace;
	m_currentVelocity = newCurrentVelocity_p;
}

//-------------------------------------------------------------------

Vector const &SimpleDynamics::getCurrentVelocity_o() const
{
	FATAL(m_movementSpace != ObjectSpace && m_currentVelocity != Vector::zero, ("SimpleDynamics::getCurrentVelocity_o called when velocity is not in object space"));
	return m_currentVelocity;
}

//-------------------------------------------------------------------

void SimpleDynamics::setCurrentVelocity_o(Vector const &newCurrentVelocity_o)
{
	m_movementSpace = ObjectSpace;
	m_currentVelocity = newCurrentVelocity_o;
}

//-------------------------------------------------------------------

float SimpleDynamics::alter (const float time)
{
	float const baseAlterResult = Dynamics::alter (time);
	UNREF (baseAlterResult);

	if (getOwner () && m_currentVelocity != Vector::zero)
	{
		switch (m_movementSpace)
		{
		case WorldSpace:
			getOwner ()->move_o (getOwner ()->rotate_w2o (m_currentVelocity * time));
			break;
		case ParentSpace:
			getOwner ()->move_o (getOwner ()->rotate_p2o (m_currentVelocity * time));
			break;
		case ObjectSpace:
			getOwner ()->move_o (m_currentVelocity * time);
			break;
		}
	}

	// @todo if worthwhile, figure out a way to have an inactive SimpleDynamics
	//       return that it doesn't need an alter next frame.  Then a mechanism
	//       will be needed to cause an AlterScheduler::submitForAlter() if
	//       something causes the dynamics to require an alter.
	//
	//       For now, all simple dynamics get an alter every frame.
	return AlterResult::cms_alterNextFrame;
}

//===================================================================
