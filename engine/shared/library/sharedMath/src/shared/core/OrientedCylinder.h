// ======================================================================
//
// OrientedCylinder.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_OrientedCylinder_H
#define INCLUDED_OrientedCylinder_H


#include "sharedMath/Vector.h"

class Transform;
class Range;
class Cylinder;
class OrientedCircle;
class Segment3d;

// ======================================================================

class OrientedCylinder
{
public:

	OrientedCylinder();
	OrientedCylinder ( Vector const & base, Vector const & axis, float radius, float height );
	
	explicit OrientedCylinder ( Cylinder const & cyl );
	OrientedCylinder( Cylinder const & s, Transform const & tform );

	// ----------

	Vector const & getBase ( void ) const;
	Vector const & getAxis ( void ) const;

	float getRadius ( void ) const;
	float getHeight ( void ) const;

	// ----------
	// Sub-shapes

	Segment3d		getAxisSegment	( void ) const;
	OrientedCircle	getBaseCircle	( void ) const;
	OrientedCircle  getTopCircle	( void ) const;

	// ----------

	Vector getCenter ( void ) const;

	Vector getAxisX ( void ) const;
	Vector getAxisY ( void ) const;
	Vector getAxisZ ( void ) const;

	float getExtentX ( void ) const;
	float getExtentY ( void ) const;
	float getExtentZ ( void ) const;

	// ----------

	Transform getTransform_l2p ( void ) const;
	Transform getTransform_p2l ( void ) const;

	Vector transformToLocal ( Vector const & V ) const;
	Vector transformToWorld ( Vector const & V ) const;

	Vector rotateToLocal ( Vector const & V ) const;
	Vector rotateToWorld ( Vector const & V ) const;

	Cylinder getLocalShape ( void ) const;

protected:

	Vector m_base;
	Vector m_axis;

	float  m_radius;
	float  m_height;
};

// ----------------------------------------------------------------------

inline OrientedCylinder::OrientedCylinder ()
: m_base( Vector::zero ),
  m_axis( Vector(0.0f,1.0f,0.0f) ),
  m_radius( 1.0f ),
  m_height( 1.0f )
{
}

inline OrientedCylinder::OrientedCylinder ( Vector const & base, Vector const & axis, float radius, float height )
: m_base( base ),
  m_axis( axis ),
  m_radius( radius ),
  m_height( height )
{
	IGNORE_RETURN(m_axis.normalize());
}

// ----------

inline Vector const & OrientedCylinder::getBase ( void ) const
{
	return m_base;
}

inline Vector const & OrientedCylinder::getAxis ( void ) const
{
	return m_axis;
}

inline float OrientedCylinder::getRadius ( void ) const
{
	return m_radius;
}

inline float OrientedCylinder::getHeight ( void ) const
{
	return m_height;
}

// ----------------------------------------------------------------------

inline Vector OrientedCylinder::getCenter ( void ) const
{
	return m_base + m_axis * (m_height / 2.0f);
}

inline Vector OrientedCylinder::getAxisX ( void ) const
{
	Vector projected(-m_axis.z,0.0f,m_axis.x);

	if(projected.normalize())
	{
		return projected;
	}
	else
	{
		return Vector::unitX;
	}
}

inline Vector OrientedCylinder::getAxisY ( void ) const
{
	return m_axis;
}

inline Vector OrientedCylinder::getAxisZ ( void ) const
{
	Vector temp = getAxisX().cross(m_axis);

	temp.normalize();

	return temp;
}

inline float OrientedCylinder::getExtentX ( void ) const
{
	return m_radius;
}

inline float OrientedCylinder::getExtentY ( void ) const
{
	return m_height / 2.0f;
}

inline float OrientedCylinder::getExtentZ ( void ) const
{
	return m_radius;
}

// ======================================================================

#endif

