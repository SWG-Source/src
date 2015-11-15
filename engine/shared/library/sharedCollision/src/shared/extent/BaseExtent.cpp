// ======================================================================
//
// BaseExtent.cpp
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/BaseExtent.h"

#include "sharedCollision/Containment3d.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Range.h"
#include "sharedMath/Segment3d.h"


// ----------------------------------------------------------------------

BaseExtent::BaseExtent(void)
: m_extentType(ET_Null)
{
}

BaseExtent::BaseExtent( ExtentType extentType )
: m_extentType(extentType)
{
}

BaseExtent::~BaseExtent(void)
{
}

// ----------------------------------------------------------------------

bool BaseExtent::intersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time) const
{
	return(realIntersect(begin, end, surfaceNormal, time));
}

bool BaseExtent::intersect(Vector const & begin, Vector const & end, float * time ) const
{
	Vector * surfaceNormal = 0;
	return(realIntersect(begin, end, surfaceNormal, time));
}

bool BaseExtent::intersect(Vector const & begin, Vector const & end) const
{
	Vector * surfaceNormal = 0;
	float * time = 0;
	return(realIntersect(begin, end, surfaceNormal, time));
}

// ----------------------------------------------------------------------

Vector BaseExtent::getCenter ( void ) const
{
	return getBoundingSphere().getCenter();
}

// ----------

float BaseExtent::getRadius ( void ) const
{
	return getBoundingSphere().getRadius();
}

// ----------------------------------------------------------------------

bool BaseExtent::validate ( void ) const
{
	return true;
}

// ----------------------------------------------------------------------
