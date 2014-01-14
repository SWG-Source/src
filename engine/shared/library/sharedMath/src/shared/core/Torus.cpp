// ======================================================================
//
// Torus.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Torus.h"


Vector Torus::transformToLocal( Vector const & V ) const
{
	return rotateToLocal( V - m_center );
}

Vector Torus::transformToWorld ( Vector const & V ) const
{
	return rotateToWorld(V) + m_center;
}

Vector Torus::rotateToLocal ( Vector const & V ) const
{
	Vector temp;

	temp.x = V.dot(m_tangent);
	temp.y = V.dot(m_axis);
	temp.z = V.dot(m_binormal);

	return temp;
}

Vector Torus::rotateToWorld ( Vector const & V ) const
{
    Vector temp = Vector::zero;

	temp += m_tangent * V.x;
	temp += m_axis * V.y;
	temp += m_binormal * V.z;

	return temp;
}
