// ======================================================================
//
// Region3d.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/Region3d.h"

#include "serverGame/Region3dMaster.h"
#include "serverScript/ScriptParameters.h"

// ======================================================================

Region3d::Region3d(std::string const &name, Sphere const &containingSphere, ScriptParams *regionScriptData) :
	m_name(name),
	m_containingSphere(containingSphere),
	m_regionScriptData(regionScriptData),
	m_spatialSubdivisionHandle(0)
{
	Region3dMaster::addRegion(*this);
}

// ----------------------------------------------------------------------

Region3d::~Region3d()
{
	Region3dMaster::removeRegion(*this);
	delete m_regionScriptData;
	m_regionScriptData = 0;
	m_spatialSubdivisionHandle = 0;
}

// ----------------------------------------------------------------------

Sphere const &Region3d::getContainingSphere() const
{
	return m_containingSphere;
}

// ----------------------------------------------------------------------

std::string const &Region3d::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

ScriptParams const *Region3d::getRegionScriptData() const
{
	return m_regionScriptData;
}

// ----------------------------------------------------------------------

SpatialSubdivisionHandle *Region3d::getSpatialSubdivisionHandle()
{
	return m_spatialSubdivisionHandle;
}

// ----------------------------------------------------------------------

void Region3d::setSpatialSubdivisionHandle(SpatialSubdivisionHandle *newHandle)
{
	m_spatialSubdivisionHandle = newHandle;
}

// ======================================================================

