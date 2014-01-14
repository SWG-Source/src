// ======================================================================
//
// YawedBox.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/YawedBox.h"

#include "sharedMath/Transform.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Range.h"

YawedBox::YawedBox ( AxialBox const & box, float yaw )
: m_base    ( box.getBase() ),
  m_axisX   ( box.getAxisX() ),
  m_axisZ   ( box.getAxisZ() ),
  m_extentX ( box.getExtentX() ),
  m_extentZ ( box.getExtentZ() ),
  m_height  ( box.getHeight() )
{
	float c = cos(yaw);
	float s = sin(yaw);

	m_axisX = Vector(  c, 0.0f, -s );
	m_axisZ = Vector(  s, 0.0f,  c );

}

// ----------------------------------------------------------------------

Range YawedBox::getRangeY ( void ) const
{
	float min = getBase().y;
	float max = min + getHeight();
	
	return Range(min,max);
}

// ----------------------------------------------------------------------

AxialBox YawedBox::getLocalShape ( void ) const
{
	return AxialBox( Vector( -getExtentX(), -getExtentY(), -getExtentZ() ),
		             Vector(  getExtentX(),  getExtentY(),  getExtentZ() ) );
}

Transform YawedBox::getTransform_l2p ( void ) const
{
	Transform temp;

    temp.setLocalFrameIJK_p( m_axisX, Vector::unitY, m_axisZ );
	temp.move_p( m_base );

	return temp;
}

Transform YawedBox::getTransform_p2l ( void ) const
{
	Transform temp;

	temp.invert(getTransform_l2p());

	return temp;
}


Vector YawedBox::transformToLocal ( Vector const & V ) const
{
	return rotateToLocal(V - getCenter());
}

Vector YawedBox::transformToWorld ( Vector const & V ) const
{
	return rotateToWorld(V) + getCenter();
}

Vector YawedBox::rotateToLocal ( Vector const & V ) const
{
	Vector temp;

	temp.x = V.dot(getAxisX());
	temp.y = V.dot(getAxisY());
	temp.z = V.dot(getAxisZ());

	return temp;
}

Vector YawedBox::rotateToWorld ( Vector const & V ) const
{
    Vector temp = Vector::zero;

	temp += getAxisX() * V.x;
	temp += getAxisY() * V.y;
	temp += getAxisZ() * V.z;

	return temp;
}

// ----------------------------------------------------------------------

