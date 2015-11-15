// ======================================================================
//
// SpaceTerrainAppearance.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SpaceTerrainAppearance_H
#define INCLUDED_SpaceTerrainAppearance_H

// ======================================================================

#include "sharedTerrain/TerrainAppearance.h"

class SpaceTerrainAppearanceTemplate;

// ======================================================================

class SpaceTerrainAppearance : public TerrainAppearance
{
public:

	SpaceTerrainAppearance(SpaceTerrainAppearanceTemplate const * spaceTerrainAppearanceTemplate);
	virtual ~SpaceTerrainAppearance();

	virtual float getMapWidthInMeters() const;

private:

	SpaceTerrainAppearance();
	SpaceTerrainAppearance(SpaceTerrainAppearance const &);
	SpaceTerrainAppearance & operator=(SpaceTerrainAppearance const &);
};

// ======================================================================

#endif
