// ======================================================================
//
// SpaceTerrainAppearance.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/SpaceTerrainAppearance.h"

#include "sharedTerrain/SpaceTerrainAppearanceTemplate.h"

// ======================================================================
// PUBLIC SpaceTerrainAppearance
// ======================================================================

SpaceTerrainAppearance::SpaceTerrainAppearance(SpaceTerrainAppearanceTemplate const * const spaceTerrainAppearanceTemplate) :
	TerrainAppearance(spaceTerrainAppearanceTemplate)
{
}

// ----------------------------------------------------------------------

SpaceTerrainAppearance::~SpaceTerrainAppearance()
{
}

// ----------------------------------------------------------------------

float SpaceTerrainAppearance::getMapWidthInMeters() const
{
	SpaceTerrainAppearanceTemplate const * const stat = safe_cast<SpaceTerrainAppearanceTemplate const *>(getAppearanceTemplate());
	return stat->getMapWidthInMeters();
}

// ======================================================================
