//===================================================================
//
// SpringDynamics.h
// asommers 12-12-2000
//
// a simple dynamics class showing how to use it
//
//===================================================================

#ifndef INCLUDED_SpringDynamics_H
#define INCLUDED_SpringDynamics_H

//===================================================================

#include "sharedObject/StateDynamics.h"
#include "sharedMath/Vector.h"

//===================================================================

class SpringDynamics : public StateDynamics
{
public:

	SpringDynamics (Object* owner, const Vector& offset, const Vector& springDirection, float springMagnitude, float springsPerSecond);
	virtual ~SpringDynamics ();

	void setPosition (const Vector& position);

protected:

	virtual void realAlter (float time);

private:

	SpringDynamics ();
	SpringDynamics (const SpringDynamics&);
	SpringDynamics& operator= (const SpringDynamics&);

private:

	const Object* m_parent;
	const Vector  m_offset;
	const Vector  m_springDirection;
	const float   m_springMagnitude;
	const float   m_springsPerSecond;

	float         m_dy;
	Vector        m_position;
};

//===================================================================

#endif
