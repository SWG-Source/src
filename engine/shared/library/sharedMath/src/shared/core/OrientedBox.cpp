// ======================================================================
//
// OrientedBox.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/OrientedBox.h"

#include "sharedMath/Plane3d.h"
#include "sharedMath/Transform.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"

// ----------------------------------------------------------------------

OrientedBox::OrientedBox ( AxialBox const & s, Transform const & tform )
: m_center  ( tform.rotateTranslate_l2p(s.getCenter()) ),
  m_axisX   ( tform.rotate_l2p(s.getAxisX()) ),
  m_axisY   ( tform.rotate_l2p(s.getAxisY()) ),
  m_axisZ   ( tform.rotate_l2p(s.getAxisZ()) ),
  m_extentX ( s.getExtentX() ),
  m_extentY ( s.getExtentY() ),
  m_extentZ ( s.getExtentZ() )
{
}

// ----------

OrientedBox::OrientedBox ( AxialBox const & s )
: m_center  ( s.getCenter() ),
  m_axisX   ( s.getAxisX() ),
  m_axisY   ( s.getAxisY() ),
  m_axisZ   ( s.getAxisZ() ),
  m_extentX ( s.getExtentX() ),
  m_extentY ( s.getExtentY() ),
  m_extentZ ( s.getExtentZ() )
{
}

// ----------

OrientedBox::OrientedBox ( YawedBox const & s )
: m_center  ( s.getCenter() ),
  m_axisX   ( s.getAxisX() ),
  m_axisY   ( s.getAxisY() ),
  m_axisZ   ( s.getAxisZ() ),
  m_extentX ( s.getExtentX() ),
  m_extentY ( s.getExtentY() ),
  m_extentZ ( s.getExtentZ() )
{
}

// ----------------------------------------------------------------------

Plane3d OrientedBox::getFacePlane ( int whichFace ) const
{
	switch(whichFace)
	{
	case 0: return Plane3d( m_center + m_axisX * m_extentX,  m_axisX );
	case 1: return Plane3d( m_center + m_axisY * m_extentY,  m_axisY );
	case 2: return Plane3d( m_center + m_axisZ * m_extentZ,  m_axisZ );
	case 3: return Plane3d( m_center - m_axisX * m_extentX, -m_axisX );
	case 4: return Plane3d( m_center - m_axisY * m_extentY, -m_axisY );
	case 5: return Plane3d( m_center - m_axisZ * m_extentZ, -m_axisZ );

	default:
		DEBUG_FATAL(true,("OrientedBox::getFacePlane - invalid face\n"));
		return Plane3d(Vector::zero,Vector::zero);
	}
}

// ----------------------------------------------------------------------

AxialBox OrientedBox::getLocalShape ( void ) const
{
	return AxialBox( Vector(-m_extentX,-m_extentY,-m_extentZ),
	                 Vector( m_extentX, m_extentY, m_extentZ) );
}

Transform OrientedBox::getTransform_l2p ( void ) const
{
	Transform temp;

	temp.setLocalFrameIJK_p( getAxisX(), getAxisY(), getAxisZ() );
	temp.move_p( m_center );

	return temp;
}

Transform OrientedBox::getTransform_p2l ( void ) const
{
	Transform temp;

	temp.invert(getTransform_l2p());

	return temp;
}

Vector OrientedBox::transformToLocal( Vector const & V ) const
{
	return rotateToLocal( V - m_center );
}

Vector OrientedBox::transformToWorld ( Vector const & V ) const
{
	return rotateToWorld(V) + m_center;
}

Vector OrientedBox::rotateToLocal ( Vector const & V ) const
{
	Vector temp;

	temp.x = V.dot(getAxisX());
	temp.y = V.dot(getAxisY());
	temp.z = V.dot(getAxisZ());

	return temp;
}

Vector OrientedBox::rotateToWorld ( Vector const & V ) const
{
    Vector temp = Vector::zero;

	temp += getAxisX() * V.x;
	temp += getAxisY() * V.y;
	temp += getAxisZ() * V.z;

	return temp;
}

// ----------------------------------------------------------------------
