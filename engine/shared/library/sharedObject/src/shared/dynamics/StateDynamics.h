//===================================================================
//
// StateDynamics.h
// asommers 12-12-2000
//
// a simple dynamics class showing how to use it
//
//===================================================================

#ifndef INCLUDED_StateDynamics_H
#define INCLUDED_StateDynamics_H

//===================================================================

#include "sharedFoundation/Timer.h"
#include "sharedObject/Dynamics.h"

//===================================================================

class StateDynamics : public Dynamics
{
public:

	StateDynamics (Object* owner);
	virtual ~StateDynamics ();

	virtual float alter (float elapsedTime);

	void setState (bool on);

protected:

	virtual void realAlter (float elapsedTime);

private:

	StateDynamics ();
	StateDynamics (const StateDynamics&);
	StateDynamics& operator= (const StateDynamics&);

private:

	enum State
	{
		S_off,
		S_offToOn,
		S_on,
		S_onToOff
	};

private:

	State m_state;
	Timer m_stateTimer;
};

//===================================================================

#endif
