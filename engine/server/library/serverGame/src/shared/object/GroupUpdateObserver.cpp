// ======================================================================
//
// GroupUpdateObserver.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GroupUpdateObserver.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/Client.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GroupMissionCriticalObjectsBuilder.h"
#include "serverGame/GroupWaypointBuilder.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GroupStringId.h"
#include "serverGame/ObserveTracker.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedGame/ProsePackage.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

static std::set<GroupObject const *> s_activeUpdaters;

// ======================================================================

inline CreatureObject *findCreatureById(NetworkId const &networkId)
{
	Object *obj = NetworkIdManager::getObjectById(networkId);
	if (obj)
	{
		ServerObject *serverObj = obj->asServerObject();
		if (serverObj)
			return serverObj->asCreatureObject();
	}
	return 0;
}

// ======================================================================

GroupUpdateObserver::GroupUpdateObserver(GroupObject *group, Archive::AutoDeltaObserverOp) :
	m_profilerBlock("GroupUpdateObserver"),
	m_group(0),
	m_members(),
	m_leaderId(NetworkId::cms_invalid),
	m_lootMasterId()
{
	if (!group || s_activeUpdaters.find(group) != s_activeUpdaters.end())
		return;
	m_group = group;
	s_activeUpdaters.insert(group);
	group->getGroupMemberIds(m_members);
	m_leaderId = group->getGroupLeaderId();
	m_lootMasterId = group->getLootMasterId();
}

// ----------------------------------------------------------------------

GroupUpdateObserver::~GroupUpdateObserver()
{
	if (!m_group)
		return;

	s_activeUpdaters.erase(m_group);

	std::vector<NetworkId> newMembers;
	m_group->getGroupMemberIds(newMembers);

	NetworkId const &newLeaderId = m_group->getGroupLeaderId();

	// leader change notifications
	{
		if (newLeaderId != m_leaderId)
		{
			// script triggers on old and new leaders
			if (m_leaderId != NetworkId::cms_invalid)
			{
				GroupObject::removeFromLeaderMap(m_leaderId, m_group->getNetworkId());
				CreatureObject *oldLeaderCreature = findCreatureById(m_leaderId);
				if (oldLeaderCreature && oldLeaderCreature->isAuthoritative() && oldLeaderCreature->getScriptObject())
				{
					ScriptParams scriptParams;
					scriptParams.addParam(m_group->getNetworkId());
					scriptParams.addParam(newLeaderId);
					scriptParams.addParam(m_leaderId);
					IGNORE_RETURN(oldLeaderCreature->getScriptObject()->trigAllScripts(Scripting::TRIG_GROUP_LEADER_CHANGED, scriptParams));
				}
			}
			if (newLeaderId != NetworkId::cms_invalid)
			{
				GroupObject::addToLeaderMap(newLeaderId, m_group->getNetworkId());
				CreatureObject *newLeaderCreature = findCreatureById(newLeaderId);
				if (newLeaderCreature && newLeaderCreature->isAuthoritative() && newLeaderCreature->getScriptObject())
				{
					ScriptParams scriptParams;
					scriptParams.addParam(m_group->getNetworkId());
					scriptParams.addParam(newLeaderId);
					scriptParams.addParam(m_leaderId);
					IGNORE_RETURN(newLeaderCreature->getScriptObject()->trigAllScripts(Scripting::TRIG_GROUP_LEADER_CHANGED, scriptParams));
				}
			}

			// messages to group members if there was a leader before
			if (m_leaderId != NetworkId::cms_invalid)
			{
				ProsePackage pp;
				pp.stringId = GroupStringId::SID_GROUP_NEW_LEADER;
				pp.actor.str = Unicode::narrowToWide(m_group->getGroupLeaderName());
				for (std::vector<NetworkId>::iterator i = newMembers.begin(); i != newMembers.end(); ++i)
				{
					CreatureObject *creature = findCreatureById(*i);
					if (creature && creature->isAuthoritative())
						Chat::sendSystemMessage(*creature, pp);
				}
			}
		}

		NetworkId const & newLootMasterId = m_group->getLootMasterId();

		if ((m_lootMasterId != newLootMasterId) && (newLootMasterId.isValid()))
		{
			ProsePackage pp;
			pp.stringId = GroupStringId::SID_GROUP_NEW_MASTER_LOOTER;
			pp.actor.str = Unicode::narrowToWide(m_group->getMemberName(newLootMasterId));
			for (std::vector<NetworkId>::iterator i = newMembers.begin(); i != newMembers.end(); ++i)
			{
				CreatureObject * const creature = findCreatureById(*i);
				if (creature && creature->isAuthoritative())
					Chat::sendSystemMessage(*creature, pp);
			}
		}
	}

	// gather any people removed from the group in delMembers
	// and pare down newMembers to only those added to the group
	std::vector<NetworkId> delMembers;
	{
		for (std::vector<NetworkId>::iterator i = m_members.begin(); i != m_members.end(); ++i)
		{
			bool found = false;
			for (std::vector<NetworkId>::iterator j = newMembers.begin(); j != newMembers.end(); ++j)
			{
				if (*i == *j)
				{
					found = true;
					newMembers.erase(j);
					break;
				}
			}
			if (!found)
				delMembers.push_back(*i);
		}
	}

	// If the group member list has changed, notify all the current and former group members
	if (!delMembers.empty() || !newMembers.empty())
	{
		// Grab the list of current members
		std::vector<NetworkId> currentMembers;
		m_group->getGroupMemberIds(currentMembers);

		// Create a combined list of members that includes both the current members and the deleted members
		std::vector<NetworkId> currentAndDeletedGroupMembers(currentMembers.begin(), currentMembers.end());
		currentAndDeletedGroupMembers.insert(currentAndDeletedGroupMembers.end(), delMembers.begin(), delMembers.end());

		// Tell all the group members (both past and present) about the group member change
		std::vector<NetworkId>::const_iterator i;
		for (i = currentAndDeletedGroupMembers.begin(); i != currentAndDeletedGroupMembers.end(); ++i)
		{
			CreatureObject *creature = findCreatureById(*i);
			if (creature && creature->isAuthoritative() && creature->getScriptObject())
			{
				ScriptParams scriptParams;
				scriptParams.addParam(m_group->getNetworkId());
				scriptParams.addParam(currentMembers);   // current group members
				scriptParams.addParam(newMembers);       // the added group members
				scriptParams.addParam(delMembers);       // the removed group members
				IGNORE_RETURN(creature->getScriptObject()->trigAllScripts(Scripting::TRIG_GROUP_MEMBERS_CHANGED, scriptParams));
			}
		}
	}

	// have removed members with clients unobserve the group object, and get remove or disband messages
	{
		bool disbanding = newLeaderId == NetworkId::cms_invalid;
		for (std::vector<NetworkId>::iterator i = delMembers.begin(); i != delMembers.end(); ++i)
			m_group->onGroupMemberRemoved(*i, disbanding);
	}
	// have new members with clients observe the group object, and get join or formed messages
	{
		for (std::vector<NetworkId>::iterator i = newMembers.begin(); i != newMembers.end(); ++i)
		{
			CreatureObject *creature = findCreatureById(*i);
			if (creature && creature->isAuthoritative())
			{
				creature->setGroup(m_group, true);
				if (creature->getScriptObject())
				{
					ScriptParams scriptParams;
					scriptParams.addParam(m_group->getNetworkId());
					IGNORE_RETURN(creature->getScriptObject()->trigAllScripts(Scripting::TRIG_ADDED_TO_GROUP, scriptParams));
					if ((*i) == newLeaderId)
					{
						IGNORE_RETURN(creature->getScriptObject()->trigAllScripts(Scripting::TRIG_GROUP_FORMED, scriptParams));
					}
				}

				Client *client = creature->getClient();
				if (client)
				{
					ObserveTracker::onClientJoinedGroup(*client, *m_group);
					ProsePackage pp;
					pp.stringId = (*i) == newLeaderId ? GroupStringId::SID_GROUP_FORMED : GroupStringId::SID_GROUP_JOIN;
					Chat::sendSystemMessage(*creature, pp);
				}
			}
		}
	}

	GroupMissionCriticalObjectsBuilder::updateGroupMissionCriticalObjects(*m_group);
	GroupWaypointBuilder::updateGroupWaypoints(*m_group);
}

// ======================================================================

