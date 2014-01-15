// ======================================================================
//
// GroupMissionCriticalObjectsBuilder.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_GroupMissionCriticalObjectsBuilder_H
#define INCLUDED_GroupMissionCriticalObjectsBuilder_H

// ======================================================================

class CreatureObject;
class GroupObject;
class NetworkId;

// ======================================================================

class GroupMissionCriticalObjectsBuilder
{
public:

	static void updateGroupMissionCriticalObjects(GroupObject const & groupObject);
	static void updateGroupMissionCriticalObjects(CreatureObject & creatureObject, bool updateGroup);
};

// ======================================================================

#endif
