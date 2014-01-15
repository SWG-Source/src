// ======================================================================
//
// RegionBox.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/RegionBox.h"

// ======================================================================

RegionBox::RegionBox(std::string const &name, Vector const &corner1, Vector const &corner2, ScriptParams *regionScriptData) :
	Region3d(name, Sphere((corner1+corner2)/2.0f, corner1.magnitudeBetween(corner2)/2.0f), regionScriptData),
	m_corner1(corner1),
	m_corner2(corner2)
{
}

// ----------------------------------------------------------------------

bool RegionBox::containsPoint(Vector const &point) const
{
	return    point.x >= m_corner1.x
	       && point.y >= m_corner1.y
	       && point.z >= m_corner1.z
	       && point.x < m_corner2.x
	       && point.y < m_corner2.y
	       && point.z < m_corner2.z;
}

// ----------------------------------------------------------------------

float RegionBox::getVolume() const
{
	return   (m_corner2.x-m_corner1.x)
	       * (m_corner2.y-m_corner1.y)
	       * (m_corner2.z-m_corner1.z);
}

// ======================================================================

