// ======================================================================
//
// PathfindingEnums.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PathfindingEnums_H
#define INCLUDED_PathfindingEnums_H

// ======================================================================

enum PathGraphType
{
	PGT_Cell,
	PGT_Building,
	PGT_City,
	
	PGT_None,
};

// ----------

enum PathNodeType
{
	// node types for cell graphs

	PNT_CellPortal           = 0,
	PNT_CellWaypoint         = 1,
	PNT_CellPOI              = 2,
	
	// node types for building graphs

	PNT_BuildingEntrance     = 3,
	PNT_BuildingCell         = 4,
	PNT_BuildingPortal       = 5,

	// node types for city graphs

	PNT_CityBuildingEntrance = 6,
	PNT_CityWaypoint         = 7,
	PNT_CityPOI              = 8,
	PNT_CityBuilding         = 9,
	PNT_CityEntrance         = 10,   // the city gates, in a sense

	// new node type to represent one contiguous piece of a cell

	PNT_BuildingCellPart     = 11,

	PNT_Invalid              = 12,
};

// ----------

enum PathBuildResult
{
	PBR_OK,
	PBR_Unreachable,
	PBR_Incomplete,
};




// ======================================================================

#endif

