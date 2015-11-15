// ======================================================================
//
// RegionSphere.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/RegionSphere.h"

// ======================================================================

const float FOUR_THIRDS_PI = (4.0f*PI)/3.0f;

// ======================================================================

RegionSphere::RegionSphere(std::string const &name, Vector const &center, float radius, ScriptParams *regionScriptData) :
	Region3d(name, Sphere(center, radius), regionScriptData)
{
}

// ----------------------------------------------------------------------

bool RegionSphere::containsPoint(Vector const &point) const
{
	return getContainingSphere().contains(point);
}

// ----------------------------------------------------------------------

float RegionSphere::getVolume() const
{
	float radius = getContainingSphere().getRadius();
	return FOUR_THIRDS_PI*radius*radius*radius;
}

// ======================================================================

