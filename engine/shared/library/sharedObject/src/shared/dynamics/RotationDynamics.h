//===================================================================
//
// RotationDynamics.h
// asommers 12-12-2000
//
// a simple dynamics class showing how to use it
//
//===================================================================

#ifndef INCLUDED_RotationDynamics_H
#define INCLUDED_RotationDynamics_H

//===================================================================

#include "sharedMath/Vector.h"
#include "sharedObject/StateDynamics.h"

//===================================================================

class RotationDynamics : public StateDynamics
{
public:

	RotationDynamics (Object* owner, const Vector& rotationsPerSecond);
	virtual ~RotationDynamics ();

	void setRotateAroundAppearanceCenter(bool rotateAroundAppearanceCenter);

protected:

	virtual void realAlter (float elapsedTime);

private:

	RotationDynamics ();
	RotationDynamics (const RotationDynamics&);
	RotationDynamics& operator= (const RotationDynamics&);

private:

	const Vector  m_rotationsPerSecond;
	bool m_rotateAroundAppearanceCenter;
};

//===================================================================

#endif
