// ======================================================================
//
// OrientedCircle.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/OrientedCircle.h"

#include "sharedMath/Circle.h"

OrientedCircle::OrientedCircle ( Vector const & center, Vector const & axis, float radius )
: m_center(center),
  m_axis(axis),
  m_radius(radius)
{
}

OrientedCircle::OrientedCircle ( Circle const & circle )
: m_center( circle.getCenter() ),
  m_axis( Vector(0,1,0) ),
  m_radius( circle.getRadius() )
{
}



