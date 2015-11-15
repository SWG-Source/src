// ======================================================================
//
// ServerSpaceTerrainAppearance.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ServerSpaceTerrainAppearance.h"

#include "sharedTerrain/ServerSpaceTerrainAppearanceTemplate.h"

// ======================================================================
// PUBLIC ServerSpaceTerrainAppearance
// ======================================================================

ServerSpaceTerrainAppearance::ServerSpaceTerrainAppearance(ServerSpaceTerrainAppearanceTemplate const * const serverSpaceTerrainAppearanceTemplate) :
	SpaceTerrainAppearance(serverSpaceTerrainAppearanceTemplate)
{
}

// ----------------------------------------------------------------------

ServerSpaceTerrainAppearance::~ServerSpaceTerrainAppearance()
{
}

// ======================================================================
