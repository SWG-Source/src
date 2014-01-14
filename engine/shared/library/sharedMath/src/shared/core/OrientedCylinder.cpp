// ======================================================================
//
// OrientedCylinder.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/OrientedCylinder.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Range.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/OrientedCircle.h"
#include "sharedMath/Segment3d.h"


// ----------------------------------------------------------------------

OrientedCylinder::OrientedCylinder ( Cylinder const & s )
: m_base   ( s.getBase() ),
  m_axis   ( Vector::unitY ),
  m_radius ( s.getRadius() ),
  m_height ( s.getHeight() )
{
}

OrientedCylinder::OrientedCylinder ( Cylinder const & s, Transform const & tform )
: m_base   ( tform.rotateTranslate_l2p(s.getBase()) ),
  m_axis   ( tform.rotate_l2p(s.getAxisY()) ),
  m_radius ( s.getRadius() ),
  m_height ( s.getHeight() )
{
}

// ----------------------------------------------------------------------

Segment3d OrientedCylinder::getAxisSegment ( void ) const
{
	return Segment3d( m_base, m_base + m_axis * m_height );
}

OrientedCircle	OrientedCylinder::getBaseCircle ( void ) const
{
	return OrientedCircle(m_base,m_axis,m_radius);
}

OrientedCircle  OrientedCylinder::getTopCircle ( void ) const
{
	return OrientedCircle(m_base + m_axis * m_height,m_axis,m_radius);
}

// ----------------------------------------------------------------------

Cylinder OrientedCylinder::getLocalShape ( void ) const
{
	float extentY = getExtentY();

	return Cylinder( Vector(0,-extentY,0), m_radius, extentY * 2.0f );
}

Transform OrientedCylinder::getTransform_l2p ( void ) const
{
	Transform temp;

	temp.setLocalFrameIJK_p( getAxisX(), getAxisY(), getAxisZ() );
	temp.move_p( getCenter() );

	return temp;
}

Transform OrientedCylinder::getTransform_p2l ( void ) const
{
	Transform temp;

	temp.invert(getTransform_l2p());

	return temp;
}

Vector OrientedCylinder::transformToLocal ( Vector const & V ) const
{
	return rotateToLocal(V - getCenter());
}

Vector OrientedCylinder::transformToWorld ( Vector const & V ) const
{
	return rotateToWorld(V) + getCenter();
}

Vector OrientedCylinder::rotateToLocal ( Vector const & V ) const
{
	Vector temp;

	Vector axisX = getAxisX();
	Vector axisY = getAxisY();
	Vector axisZ = getAxisZ();

	temp.x = V.dot(axisX);
	temp.y = V.dot(axisY);
	temp.z = V.dot(axisZ);

	return temp;
}

Vector OrientedCylinder::rotateToWorld ( Vector const & V ) const
{
    Vector temp = Vector::zero;

	Vector axisX = getAxisX();
	Vector axisY = getAxisY();
	Vector axisZ = getAxisZ();

	temp += axisX * V.x;
	temp += axisY * V.y;
	temp += axisZ * V.z;

	return temp;
}

// ----------------------------------------------------------------------
