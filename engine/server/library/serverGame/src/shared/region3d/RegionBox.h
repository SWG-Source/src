// ======================================================================
//
// RegionBox.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RegionBox_H
#define INCLUDED_RegionBox_H

// ======================================================================

#include "Region3d.h"

// ======================================================================

class RegionBox: public Region3d
{
public:
	RegionBox(std::string const &name, Vector const &corner1, Vector const &corner2, ScriptParams *regionScriptData);

	virtual bool containsPoint(Vector const &point) const;
	virtual float getVolume() const;

private:
	RegionBox();
	RegionBox(RegionBox const &);
	RegionBox &operator=(RegionBox const &);

private:
	Vector m_corner1;
	Vector m_corner2;
};

// ======================================================================

#endif // INCLUDED_RegionBox_H

