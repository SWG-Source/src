// ======================================================================
//
// GroupWaypointBuilder.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GroupWaypointBuilder.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GroupObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "sharedUtility/Location.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace GroupWaypointBuilderNamespace
{
	void buildGroupWaypoints(GroupObject const &groupObject, PlayerObject::WaypointMap &groupWaypoints);
	void buildGroupWaypoints(PlayerObject const &playerObject, PlayerObject::WaypointMap &groupWaypoints);
	void updateGroupWaypoints(GroupObject const &groupObject, PlayerObject::WaypointMap const &groupWaypoints);
	void updateGroupWaypoints(PlayerObject &playerObject, PlayerObject::WaypointMap const &groupWaypoints);
}

using namespace GroupWaypointBuilderNamespace;

// ======================================================================
// GroupWaypointBuilderNamespace
// ======================================================================

void GroupWaypointBuilderNamespace::buildGroupWaypoints(GroupObject const &groupObject, PlayerObject::WaypointMap &groupWaypoints)
{
	GroupObject::GroupMemberVector const & groupMemberVector = groupObject.getGroupMembers();
	for (GroupObject::GroupMemberVector::const_iterator groupMemberVectorIter = groupMemberVector.begin(); groupMemberVectorIter != groupMemberVector.end(); ++groupMemberVectorIter)
	{
		Object const * const object = NetworkIdManager::getObjectById(groupMemberVectorIter->first);
		if (!object)
			continue;

		ServerObject const * serverObject = object->asServerObject();
		if (!serverObject || !serverObject->isPlayerControlled())
			continue;

		CreatureObject const * const creatureObject = serverObject->asCreatureObject();
		if (!creatureObject)
			continue;

		PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
		if (!playerObject)
			continue;

		buildGroupWaypoints(*playerObject, groupWaypoints);
	}
}

// ----------------------------------------------------------------------

void GroupWaypointBuilderNamespace::buildGroupWaypoints(PlayerObject const &playerObject, PlayerObject::WaypointMap &groupWaypoints)
{
	PlayerObject::WaypointMap const &waypoints = playerObject.getWaypoints();
	if (!waypoints.empty())
	{
		for (PlayerObject::WaypointMap::const_iterator i = waypoints.begin(); i != waypoints.end(); ++i)
			if ((*i).second.isActive() && !strcmp((*i).second.getLocation().getSceneId(), ConfigServerGame::getSceneID()))
				groupWaypoints[(*i).first] = (*i).second;
	}
}

// ----------------------------------------------------------------------

void GroupWaypointBuilderNamespace::updateGroupWaypoints(GroupObject const &groupObject, PlayerObject::WaypointMap const &groupWaypoints)
{
	GroupObject::GroupMemberVector const &groupMemberVector = groupObject.getGroupMembers();
	for (GroupObject::GroupMemberVector::const_iterator groupMemberVectorIter = groupMemberVector.begin(); groupMemberVectorIter != groupMemberVector.end(); ++groupMemberVectorIter)
	{
		Object * const object = NetworkIdManager::getObjectById(groupMemberVectorIter->first);
		if (!object)
			continue;

		ServerObject * const serverObject = object->asServerObject();
		if (!serverObject || !serverObject->isPlayerControlled())
			continue;

		CreatureObject * const creatureObject = serverObject->asCreatureObject();
		if (!creatureObject)
			continue;

		PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
		if (!playerObject)
			continue;

		updateGroupWaypoints(*playerObject, groupWaypoints);
	}
}

// ----------------------------------------------------------------------

void GroupWaypointBuilderNamespace::updateGroupWaypoints(PlayerObject &playerObject, PlayerObject::WaypointMap const &groupWaypoints)
{
	if (playerObject.isAuthoritative())
		playerObject.setGroupWaypoints(groupWaypoints);
}

// ======================================================================
// PUBLIC GroupWaypointBuilder
// ======================================================================

void GroupWaypointBuilder::updateGroupWaypoints(GroupObject const &groupObject)
{
#if 0
	PlayerObject::WaypointMap groupWaypoints;
	buildGroupWaypoints(groupObject, groupWaypoints);
	GroupWaypointBuilderNamespace::updateGroupWaypoints(groupObject, groupWaypoints);
#endif
}

// ----------------------------------------------------------------------

void GroupWaypointBuilder::updateGroupWaypoints(PlayerObject &playerObject, bool const updateGroup)
{
#if 0
	CreatureObject const * const creatureObject = playerObject.getCreatureObject();
	if (creatureObject)
	{
		GroupObject const * const groupObject = creatureObject->getGroup();
		if (groupObject && updateGroup)
		{
			PlayerObject::WaypointMap groupWaypoints;
			buildGroupWaypoints(*groupObject, groupWaypoints);
			GroupWaypointBuilderNamespace::updateGroupWaypoints(*groupObject, groupWaypoints);
		}
		else
		{
			PlayerObject::WaypointMap groupWaypoints;
			buildGroupWaypoints(playerObject, groupWaypoints);
			GroupWaypointBuilderNamespace::updateGroupWaypoints(playerObject, groupWaypoints);
		}
	}
#endif
}

// ======================================================================

