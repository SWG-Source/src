//===================================================================
//
// StateDynamics.cpp
// asommers 6-19-99
//
// copyright 1999, bootprint entertainment
//
//===================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/StateDynamics.h"

#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

//===================================================================
// PUBLIC StateDynamics
//===================================================================

StateDynamics::StateDynamics (Object* const owner) :
	Dynamics (owner),
	m_state (S_off),
	m_stateTimer (5.f)
{
}

//-------------------------------------------------------------------

StateDynamics::~StateDynamics ()
{
}

//-------------------------------------------------------------------

float StateDynamics::alter (const float elapsedTime)
{
	float const baseAlterResult = Dynamics::alter (elapsedTime);
	UNREF (baseAlterResult);

	Object* const owner = getOwner ();
	if (!owner)
	{
		// @todo should we request a kill in this condition?
		return AlterResult::cms_alterNextFrame;
	}

	switch (m_state)
	{
	case S_off:
		break;

	case S_offToOn:
		{
			if (m_stateTimer.updateZero (elapsedTime))
			{
				m_state = S_on;
				realAlter (elapsedTime);
			}
			else
				realAlter (elapsedTime * m_stateTimer.getElapsedRatio ());
		}
		break;

	case S_on:
		realAlter (elapsedTime);
		break;

	case S_onToOff:
		{
			if (m_stateTimer.updateZero (elapsedTime))
				m_state = S_off;
			else
				realAlter (elapsedTime * m_stateTimer.getRemainingRatio ());
		}
		break;
	}

	// @todo if worthwhile, come up with a way to have this only alter when
	//       necessary.
	//
	//       For now, alter every frame.
	return AlterResult::cms_alterNextFrame;
}

//-------------------------------------------------------------------

void StateDynamics::setState (const bool on)
{
	if (on)
		m_state = S_offToOn;
	else
		m_state = S_onToOff;

	m_stateTimer.reset ();
}

//===================================================================
// PROTECTED StateDynamics
//===================================================================

void StateDynamics::realAlter (const float /*elapsedTime*/)
{
}

//===================================================================
