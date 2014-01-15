// ======================================================================
//
// RegionSphere.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RegionSphere_H
#define INCLUDED_RegionSphere_H

// ======================================================================

#include "Region3d.h"

// ======================================================================

class RegionSphere: public Region3d
{
public:
	RegionSphere(std::string const &name, Vector const &center, float radius, ScriptParams *regionScriptData);

	virtual bool containsPoint(Vector const &point) const;
	virtual float getVolume() const;

private:
	RegionSphere();
	RegionSphere(RegionSphere const &);
	RegionSphere &operator=(RegionSphere const &);
};

// ======================================================================

#endif // INCLUDED_RegionSphere_H

