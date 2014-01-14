// ======================================================================
//
// ServerSpaceTerrainAppearance.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerSpaceTerrainAppearance_H
#define INCLUDED_ServerSpaceTerrainAppearance_H

// ======================================================================

#include "sharedTerrain/SpaceTerrainAppearance.h"

class ServerSpaceTerrainAppearanceTemplate;

// ======================================================================

class ServerSpaceTerrainAppearance : public SpaceTerrainAppearance
{
public:

	explicit ServerSpaceTerrainAppearance(ServerSpaceTerrainAppearanceTemplate const * serverSpaceTerrainAppearanceTemplate);
	virtual ~ServerSpaceTerrainAppearance();

private:

	ServerSpaceTerrainAppearance();
	ServerSpaceTerrainAppearance(ServerSpaceTerrainAppearance const &);
	ServerSpaceTerrainAppearance & operator=(ServerSpaceTerrainAppearance const &);
};

// ======================================================================

#endif
