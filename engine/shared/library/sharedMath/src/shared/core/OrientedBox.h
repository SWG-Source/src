// ======================================================================
//
// OrientedBox.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_OrientedBox_H
#define INCLUDED_OrientedBox_H

#include "sharedMath/Vector.h"

class Plane3d;
class AxialBox;
class YawedBox;
class Transform;

// ======================================================================

class OrientedBox
{
public:

	OrientedBox ( Vector const & center, 
				  Vector const & i, Vector const & j, Vector const & k, 
				  real extentI, real extentJ, real extentK );
	
	OrientedBox ( AxialBox const & box, Transform const & tform );

	explicit OrientedBox ( AxialBox const & box );
	explicit OrientedBox ( YawedBox const & box );

	// ----------

	Vector const *  getAxes          ( void ) const;
	float const *   getExtents       ( void ) const;

	Vector          getBase          ( void ) const;
	Plane3d         getFacePlane     ( int whichPlane ) const;

	Vector          getCorner        ( int whichCorner ) const;

	// ----------

	Vector const &  getCenter        ( void ) const;

	Vector const &  getAxisX         ( void ) const;
	Vector const &  getAxisY         ( void ) const;
	Vector const &  getAxisZ         ( void ) const;

	float           getExtentX       ( void ) const;
	float           getExtentY       ( void ) const;
	float           getExtentZ       ( void ) const;

	// ----------

	AxialBox        getLocalShape    ( void ) const;

	Transform       getTransform_l2p ( void ) const;
	Transform       getTransform_p2l ( void ) const;

	Vector          transformToLocal ( Vector const & V ) const;
	Vector          transformToWorld ( Vector const & V ) const;

	Vector          rotateToLocal    ( Vector const & V ) const;
	Vector          rotateToWorld    ( Vector const & V ) const;

protected:

	Vector	m_center;

	Vector  m_axisX;
	Vector  m_axisY;
	Vector  m_axisZ;

	float   m_extentX;
	float   m_extentY;
	float   m_extentZ;
};

// ----------------------------------------------------------------------

inline OrientedBox::OrientedBox( Vector const & center, 
								 Vector const & axisX, 
								 Vector const & axisY, 
								 Vector const & axisZ, 
								 real extentX, 
								 real extentY, 
								 real extentZ )
: m_center(center),
  m_axisX(axisX),
  m_axisY(axisY),
  m_axisZ(axisZ),
  m_extentX(extentX),
  m_extentY(extentY),
  m_extentZ(extentZ)
{
}

// ----------------------------------------------------------------------

inline Vector const * OrientedBox::getAxes ( void ) const
{
	return &m_axisX;
}

inline float const * OrientedBox::getExtents ( void ) const
{
	return &m_extentX;
}

inline Vector OrientedBox::getBase ( void ) const
{
	return getCenter() - getAxisY() * getExtentY();
}

inline Vector OrientedBox::getCorner ( int whichCorner ) const
{
	Vector X = m_axisX * m_extentX;
	Vector Y = m_axisY * m_extentY;
	Vector Z = m_axisZ * m_extentZ;

	switch(whichCorner)
	{
	case 0: return m_center - X - Y - Z;
	case 1: return m_center + X - Y - Z;
	case 2: return m_center - X - Y + Z;
	case 3: return m_center + X - Y + Z;
	case 4: return m_center - X + Y - Z;
	case 5: return m_center + X + Y - Z;
	case 6: return m_center - X + Y + Z;
	case 7: return m_center + X + Y + Z;

	default: return Vector(0,0,0);
	}
}

// ----------------------------------------------------------------------

inline Vector const & OrientedBox::getCenter ( void ) const
{
	return m_center;
}

// ----------

inline Vector const & OrientedBox::getAxisX ( void ) const
{
	return m_axisX;
}

inline Vector const & OrientedBox::getAxisY ( void ) const
{
	return m_axisY;
}

inline Vector const & OrientedBox::getAxisZ ( void ) const
{
	return m_axisZ;
}

// ----------

inline float OrientedBox::getExtentX ( void ) const
{
	return m_extentX;
}

inline float OrientedBox::getExtentY ( void ) const
{
	return m_extentY;
}

inline float OrientedBox::getExtentZ ( void ) const
{
	return m_extentZ;
}

// ----------------------------------------------------------------------

#endif

