// ======================================================================
//
// Sphere.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Sphere.h"

#include "sharedMath/Range.h"
#include "sharedMath/Circle.h"

// ======================================================================

/// Sphere centered at (0,0,0) with a radius of 0.
const Sphere Sphere::zero;

/// Sphere centered at (0,0,0) with a radius of 1.
const Sphere Sphere::unit(Vector::zero, CONST_REAL(1));

// ======================================================================

Circle Sphere::getCircle ( void ) const
{
	return Circle(m_center,m_radius);
}

Range Sphere::getRangeX ( void ) const
{
	return Range( m_center.x - m_radius, m_center.x + m_radius );
}

Range Sphere::getRangeY ( void ) const
{
	return Range( m_center.y - m_radius, m_center.y + m_radius );
}

Range Sphere::getRangeZ ( void ) const
{
	return Range( m_center.z - m_radius, m_center.z + m_radius );
}

bool Sphere::intersectsCone(Vector const & coneBase, Vector const & coneNormal, float const coneAngleRadians) const
{
	float const angleSine = sinf(coneAngleRadians);
	float const angleCosine = cosf(coneAngleRadians);
	float const angleInverseSine = angleSine > FLT_MIN ? 1.0f / angleSine : 0.0f;
	float const angleCosineSquared = sqr(angleCosine);
	
	Vector const & vectorToSphere = m_center - coneBase;
	Vector const & intersectPosition = vectorToSphere + (m_radius * angleInverseSine) * coneNormal;
	float magnitudeOfIntersectionSquared = intersectPosition.magnitudeSquared();
	float angleBetweenSourceAndIntersection = intersectPosition.dot(coneNormal);

	bool inCone = false;
	
	if (angleBetweenSourceAndIntersection > FLT_MIN && sqr(angleBetweenSourceAndIntersection) >= magnitudeOfIntersectionSquared * angleCosineSquared)
	{
		float const angleSinSquared = sqr(angleSine);
		magnitudeOfIntersectionSquared = vectorToSphere.magnitudeSquared();
		angleBetweenSourceAndIntersection = -vectorToSphere.dot(coneNormal);
		if (angleBetweenSourceAndIntersection > FLT_MIN && sqr(angleBetweenSourceAndIntersection) >= magnitudeOfIntersectionSquared * angleSinSquared)
		{
			float const rangeSquared = sqr(m_radius);
			inCone = magnitudeOfIntersectionSquared <= rangeSquared;
		}
		else
		{
			inCone = true;
		}
	}

	return inCone;
}

Vector Sphere::closestPointOnSphere(Vector const & point) const
{
	Vector pointOnSurface;
	Vector normalToPoint(point - m_center);
	
	if(normalToPoint.normalize())
	{
		pointOnSurface = m_center + (normalToPoint * m_radius);
	}
	else
	{
		pointOnSurface = m_center;
	}

	return pointOnSurface;
}

Vector Sphere::approximateClosestPointOnSphere(Vector const & point) const
{
	Vector pointOnSurface;
	Vector normalToPoint(point - m_center);
	
	if(normalToPoint.approximateNormalize())
	{
		pointOnSurface = m_center + (normalToPoint * m_radius);
	}
	else
	{
		pointOnSurface = m_center;
	}
	
	return pointOnSurface;
}


// ======================================================================

