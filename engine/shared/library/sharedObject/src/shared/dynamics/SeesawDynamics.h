//===================================================================
//
// SeesawDynamics.h
// asommers 12-12-2000
//
// a simple dynamics class showing how to use it
//
//===================================================================

#ifndef INCLUDED_SeesawDynamics_H
#define INCLUDED_SeesawDynamics_H

//===================================================================

#include "sharedObject/StateDynamics.h"

//===================================================================

class SeesawDynamics : public StateDynamics
{
public:

	enum Type
	{
		T_yaw,
		T_pitch,
		T_roll
	};

public:

	SeesawDynamics (Object* owner, Type type, float seesawsPerSecond, float seesawMagnitude);
	virtual ~SeesawDynamics ();

protected:

	virtual void realAlter (float elapsedTime);

private:

	SeesawDynamics ();
	SeesawDynamics (const SeesawDynamics&);
	SeesawDynamics& operator= (const SeesawDynamics&);

private:

	const Type    m_type;
	const float   m_seesawsPerSecond;
	const float   m_seesawMagnitude;

	float         m_dy;
};

//===================================================================

#endif
