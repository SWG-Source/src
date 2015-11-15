// ======================================================================
//
// GroupWaypointBuilder.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GroupWaypointBuilder_H
#define INCLUDED_GroupWaypointBuilder_H

// ======================================================================

class GroupObject;
class PlayerObject;

// ======================================================================

class GroupWaypointBuilder
{
public:
	static void updateGroupWaypoints(GroupObject const &groupObject);
	static void updateGroupWaypoints(PlayerObject &playerObject, bool updateGroup);
};

// ======================================================================

#endif // INCLUDED_GroupWaypointBuilder_H

