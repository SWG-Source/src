// ======================================================================
//
// GroupMissionCriticalObjectsBuilder.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GroupMissionCriticalObjectsBuilder.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GroupObject.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace GroupMissionCriticalObjectsBuilderNamespace
{
	void buildGroupMissionCriticalObjects(GroupObject const & groupObject, CreatureObject::GroupMissionCriticalObjectSet & groupMissionCriticalObjects);
	void buildGroupMissionCriticalObjects(CreatureObject const & creatureObject, CreatureObject::GroupMissionCriticalObjectSet & groupMissionCriticalObjects);
	void updateGroupMissionCriticalObjects(GroupObject const & groupObject, CreatureObject::GroupMissionCriticalObjectSet const & groupMissionCriticalObjects);
	void updateGroupMissionCriticalObjects(CreatureObject & creatureObject, CreatureObject::GroupMissionCriticalObjectSet const & groupMissionCriticalObjects);
}

using namespace GroupMissionCriticalObjectsBuilderNamespace;

// ======================================================================
// GroupMissionCriticalObjectsBuilderNamespace
// ======================================================================

void GroupMissionCriticalObjectsBuilderNamespace::buildGroupMissionCriticalObjects(GroupObject const & groupObject, CreatureObject::GroupMissionCriticalObjectSet & groupMissionCriticalObjects)
{
	GroupObject::GroupMemberVector const & groupMemberVector = groupObject.getGroupMembers();
	for (GroupObject::GroupMemberVector::const_iterator groupMemberVectorIter = groupMemberVector.begin(); groupMemberVectorIter != groupMemberVector.end(); ++groupMemberVectorIter)
	{
		Object const * const object = NetworkIdManager::getObjectById(groupMemberVectorIter->first);
		if (!object)
			continue;

		ServerObject const * serverObject = object->asServerObject();
		if (!serverObject)
			continue;

		CreatureObject const * const creatureObject = serverObject->asCreatureObject();
		if (!creatureObject)
			continue;

		buildGroupMissionCriticalObjects(*creatureObject, groupMissionCriticalObjects);
	}
}

// ----------------------------------------------------------------------

void GroupMissionCriticalObjectsBuilderNamespace::buildGroupMissionCriticalObjects(CreatureObject const & creatureObject, CreatureObject::GroupMissionCriticalObjectSet & groupMissionCriticalObjects)
{
	CreatureObject::MissionCriticalObjectSet const & missionCriticalObjects = creatureObject.getMissionCriticalObjects();
	for (CreatureObject::MissionCriticalObjectSet::const_iterator missionCriticalObjectsIter = missionCriticalObjects.begin(); missionCriticalObjectsIter != missionCriticalObjects.end(); ++missionCriticalObjectsIter)
		groupMissionCriticalObjects.insert(std::make_pair(creatureObject.getNetworkId(), *missionCriticalObjectsIter));
}

// ----------------------------------------------------------------------

void GroupMissionCriticalObjectsBuilderNamespace::updateGroupMissionCriticalObjects(GroupObject const & groupObject, CreatureObject::GroupMissionCriticalObjectSet const & groupMissionCriticalObjects)
{
	GroupObject::GroupMemberVector const & groupMemberVector = groupObject.getGroupMembers();
	for (GroupObject::GroupMemberVector::const_iterator groupMemberVectorIter = groupMemberVector.begin(); groupMemberVectorIter != groupMemberVector.end(); ++groupMemberVectorIter)
	{
		Object * const object = NetworkIdManager::getObjectById(groupMemberVectorIter->first);
		if (!object)
			continue;

		ServerObject * serverObject = object->asServerObject();
		if (!serverObject)
			continue;

		CreatureObject * const creatureObject = serverObject->asCreatureObject();
		if (!creatureObject)
			continue;

		updateGroupMissionCriticalObjects(*creatureObject, groupMissionCriticalObjects);
	}
}

// ----------------------------------------------------------------------

void GroupMissionCriticalObjectsBuilderNamespace::updateGroupMissionCriticalObjects(CreatureObject & creatureObject, CreatureObject::GroupMissionCriticalObjectSet const & groupMissionCriticalObjects)
{
	if (creatureObject.isAuthoritative())
		creatureObject.setGroupMissionCriticalObjects(groupMissionCriticalObjects);
}

// ======================================================================
// PUBLIC GroupMissionCriticalObjectsBuilder
// ======================================================================

void GroupMissionCriticalObjectsBuilder::updateGroupMissionCriticalObjects(GroupObject const & groupObject)
{
	CreatureObject::GroupMissionCriticalObjectSet groupMissionCriticalObjects;
	buildGroupMissionCriticalObjects(groupObject, groupMissionCriticalObjects);
	GroupMissionCriticalObjectsBuilderNamespace::updateGroupMissionCriticalObjects(groupObject, groupMissionCriticalObjects);
}

// ----------------------------------------------------------------------

void GroupMissionCriticalObjectsBuilder::updateGroupMissionCriticalObjects(CreatureObject & creatureObject, bool const updateGroup)
{
	GroupObject const * const groupObject = creatureObject.getGroup();
	if (groupObject && updateGroup)
	{
		CreatureObject::GroupMissionCriticalObjectSet groupMissionCriticalObjects;
		buildGroupMissionCriticalObjects(*groupObject, groupMissionCriticalObjects);
		GroupMissionCriticalObjectsBuilderNamespace::updateGroupMissionCriticalObjects(*groupObject, groupMissionCriticalObjects);
	}
	else
	{
		CreatureObject::GroupMissionCriticalObjectSet groupMissionCriticalObjects;
		buildGroupMissionCriticalObjects(creatureObject, groupMissionCriticalObjects);
		GroupMissionCriticalObjectsBuilderNamespace::updateGroupMissionCriticalObjects(creatureObject, groupMissionCriticalObjects);
	}
}

// ======================================================================
