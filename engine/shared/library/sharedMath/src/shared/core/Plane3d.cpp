// ======================================================================
//
// Plane3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Plane3d.h"

#include "sharedMath/Plane.h"

Plane3d::Plane3d ( Plane const & plane )
: m_point( plane.getNormal() * -plane.getD() ),
  m_normal( plane.getNormal() )
{
}
