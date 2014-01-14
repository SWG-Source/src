//===================================================================
//
// SimpleDynamics.h
// copyright 1999, bootprint entertainment
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SimpleDynamics_H
#define INCLUDED_SimpleDynamics_H

//===================================================================

#include "sharedObject/Dynamics.h"
#include "sharedMath/Vector.h"

//===================================================================

class SimpleDynamics : public Dynamics
{
public:

	enum MovementSpace
	{
		WorldSpace,
		ParentSpace,
		ObjectSpace
	};

	explicit SimpleDynamics(Object *newOwner);
	virtual ~SimpleDynamics();

	virtual float alter(float time);

	Vector const &getCurrentVelocity_w() const;
	void          setCurrentVelocity_w(Vector const &newCurrentVelocity_w);
	Vector const &getCurrentVelocity_p() const;
	void          setCurrentVelocity_p(Vector const &newCurrentVelocity_p);
	Vector const &getCurrentVelocity_o() const;
	void          setCurrentVelocity_o(Vector const &newCurrentVelocity_o);

private:

	SimpleDynamics();
	SimpleDynamics(SimpleDynamics const &);
	SimpleDynamics &operator=(SimpleDynamics const &);

private:

	Vector m_currentVelocity;
	MovementSpace m_movementSpace;
};

//===================================================================

#endif


