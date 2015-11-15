// ======================================================================
//
// ServerPathfindingConstants.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/ServerPathfindingConstants.h"

#include <string>

const int CITY_REGION_TYPE = 20;
const int PATHFINDING_REGION_TYPE = 22;

std::string const SERVER_PATHFINDING_WAYPOINT_TEMPLATE("object/path_waypoint/path_waypoint.iff");
std::string const SERVER_PATHFINDING_WAYPOINT_TEMPLATE_BASE("object/path_waypoint/base/path_waypoint_base.iff");
std::string const SERVER_PATHFINDING_WAYPOINT_TEMPLATE_AUTO_SPAWN("object/path_waypoint/path_waypoint_auto_spawn.iff");

std::string const OBJVAR_PATHFINDING("pathfinding");

std::string const OBJVAR_PATHFINDING_WAYPOINT_CREATOR("pathfinding.waypoint.creator");
std::string const OBJVAR_PATHFINDING_WAYPOINT_EDGES("pathfinding.waypoint.edges");
std::string const OBJVAR_PATHFINDING_WAYPOINT_KEY("pathfinding.waypoint.key");
std::string const OBJVAR_PATHFINDING_WAYPOINT_NAME("pathfinding.waypoint.name");
std::string const OBJVAR_PATHFINDING_WAYPOINT_TYPE("pathfinding.waypoint.type");

std::string const OBJVAR_PATHFINDING_BUILDING_PROCESSED("pathfinding.building.processed");
std::string const OBJVAR_PATHFINDING_BUILDING_WAYPOINTS("pathfinding.building.waypoints");

