//========================================================================
//
// MoveSimController.h
//
// copyright 2003 Sony Online Entertainment
//
//========================================================================

#ifndef	INCLUDED_MoveSimController_H
#define	INCLUDED_MoveSimController_H

#include "serverGame/CreatureController.h"

class Vector;

//-----------------------------------------------------------------------

class MoveSimController : public CreatureController
{
public:

	MoveSimController (CreatureObject * newOwner, const Vector & centerPoint, float roamRadius);
	virtual float realAlter(real time);

private:

	MoveSimController (const MoveSimController& rhs);
	MoveSimController & operator= (const MoveSimController & rhs);

	void               reverse (float towardsCenterFactor);

	Vector m_centerPoint;
	float  m_roamRadius;
	Vector m_lastPoint;
	float  m_lastUnstuckTimer;
};

//-----------------------------------------------------------------------
#endif
