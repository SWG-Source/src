// ======================================================================
//
// Region3d.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Region3d_H
#define INCLUDED_Region3d_H

// ======================================================================

#include "sharedMath/Sphere.h"

// ======================================================================

class ScriptParams;
class SpatialSubdivisionHandle;

// ======================================================================

class Region3d
{
public:
	virtual ~Region3d();

	virtual bool containsPoint(Vector const &point) const = 0;
	virtual float getVolume() const = 0;

	Sphere const &getContainingSphere() const;
	std::string const &getName() const;
	ScriptParams const *getRegionScriptData() const;

protected:
	Region3d(std::string const &name, Sphere const &containingSphere, ScriptParams *regionScriptData);

	SpatialSubdivisionHandle *getSpatialSubdivisionHandle();
	void setSpatialSubdivisionHandle(SpatialSubdivisionHandle *newHandle);

private:
	Region3d();
	Region3d(Region3d const &);
	Region3d &operator=(Region3d const &);

private:
	std::string m_name;
	Sphere m_containingSphere;
	ScriptParams *m_regionScriptData;
	SpatialSubdivisionHandle *m_spatialSubdivisionHandle;

friend class Region3dMaster;
};

// ======================================================================

#endif // INCLUDED_Region3d_H

