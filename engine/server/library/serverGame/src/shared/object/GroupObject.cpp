// ======================================================================
//
// GroupObject.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GroupObject.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupController.h"
#include "serverGame/GroupMissionCriticalObjectsBuilder.h"
#include "serverGame/GroupWaypointBuilder.h"
#include "serverGame/GroupStringId.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/GroupUpdateObserver.h"
#include "serverGame/ServerGroupObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ShipObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverNetworkMessages/GroupMemberParam.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedLog/Log.h"
#include "sharedObject/Controller.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

// ======================================================================

SharedObjectTemplate const *GroupObject::m_defaultSharedTemplate = 0;

// ======================================================================

namespace GroupObjectNamespace
{

	// ----------------------------------------------------------------------

	unsigned int const cs_maximumNumberInGroup = 8;
	char const * const DEFAULT_GROUP_TEMPLATE = "object/group/group.iff";
	std::map<NetworkId, NetworkId> s_leaderMap;
	static const std::string cs_emptyString;

	// ----------------------------------------------------------------------

	std::string const &getChatRoomPrefix()
	{
		static std::string prefix = std::string("SWG.") + GameServer::getInstance().getClusterName() + ".group.";
		return prefix;
	}

	// ----------------------------------------------------------------------

	std::string const &getChatRoomSuffix()
	{
		static std::string suffix(".GroupChat");
		return suffix;
	}

	// ----------------------------------------------------------------------


}
using namespace GroupObjectNamespace;

// ======================================================================

GroupObject::GroupObject(ServerGroupObjectTemplate const *newTemplate)
: UniverseObject(newTemplate),
m_groupName(),
m_groupMembers(),
m_groupShipFormationMembers(),
m_groupPOBShipAndOwners(),
m_groupLevel(),
m_groupMemberLevels(),
m_groupMemberProfessions(),
m_formationNameCrc(Crc::crcNull),
m_allMembers(),
m_nonPCMembers(),
m_lootMaster(),
m_lootRule(0),
m_groupPickupTimer(std::make_pair(0, 0)),
m_groupPickupLocation(std::make_pair("", Vector()))
{
	m_groupMembers.setSourceObject(this);
	m_groupShipFormationMembers.setSourceObject(this);
	m_groupPOBShipAndOwners.setSourceObject(this);
	m_lootMaster.setSourceObject(this);
	addMembersToPackages();
	ObjectTracker::addGroupObject();
}

// ----------------------------------------------------------------------

GroupObject::~GroupObject()
{
	if (!GameServer::getInstance().getDone())
	{
		for (GroupMemberVector::const_iterator i = m_groupMembers.begin(); i != m_groupMembers.end(); ++i)
			onGroupMemberRemoved((*i).first, true);
	}

	if (isInWorld())
		removeFromWorld();
	ObjectTracker::removeGroupObject();
}

// ----------------------------------------------------------------------

Controller* GroupObject::createDefaultController()
{
	Controller* controller = new GroupController(this);

	setController(controller);
	return controller;
}

// ----------------------------------------------------------------------

bool GroupObject::isVisibleOnClient(const Client &) const
{
	// Group objects are visible to all clients which are observing them, just which clients observe them
	// are strictly managed to assure this.  See GroupUpdateObserver for how this is done.
	return true;
}

// ----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * GroupObject::getDefaultSharedTemplate() const
{
	static const ConstCharCrcLowerString templateName("object/group/base/shared_group_default.iff");

	if (!m_defaultSharedTemplate)
	{
		m_defaultSharedTemplate = safe_cast<SharedObjectTemplate const *>(ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(!m_defaultSharedTemplate, ("Cannot create default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate)
			ExitChain::add(removeDefaultTemplate, "GroupObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}

// ----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void GroupObject::removeDefaultTemplate()
{
	if (!m_defaultSharedTemplate)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void GroupObject::addToLeaderMap(NetworkId const &leaderId, NetworkId const &groupId) // static
{
	s_leaderMap[leaderId] = groupId;
}

// ----------------------------------------------------------------------

void GroupObject::removeFromLeaderMap(NetworkId const &leaderId, NetworkId const &groupId) // static
{
	std::map<NetworkId, NetworkId>::iterator i = s_leaderMap.find(leaderId);
	if (i != s_leaderMap.end() && (*i).second == groupId)
		s_leaderMap.erase(i);
}

// ----------------------------------------------------------------------

NetworkId GroupObject::getGroupIdForLeader(NetworkId const &leaderId) // static
{
	NetworkId groupId = NetworkId::cms_invalid;

	std::map<NetworkId, NetworkId>::iterator i = s_leaderMap.find(leaderId);
	if (i != s_leaderMap.end())
		groupId = (*i).second;

	return groupId;
}

// ----------------------------------------------------------------------

void GroupObject::createAllGroupChatRooms() // static
{
	if (ServerUniverse::getInstance().isAuthoritative())
	{
		for (std::map<NetworkId, NetworkId>::const_iterator i = s_leaderMap.begin(); i != s_leaderMap.end(); ++i)
		{
			ServerObject const * const so = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById((*i).second));
			if (so)
			{
				GroupObject const * const groupObject = so->asGroupObject();
				if (groupObject)
					groupObject->createGroupChatRoom();
			}
		}
	}
}

// ----------------------------------------------------------------------

int GroupObject::maximumMembersInGroup()
{
	return cs_maximumNumberInGroup;
}

// ----------------------------------------------------------------------

void GroupObject::createGroup(GroupMemberParam const & leader, GroupMemberParamVector const & members)
{
	// If the leader already is leading a group, treat this as the specified member joining instead.
	GroupObject *group = 0;

	{
		std::map<NetworkId, NetworkId>::const_iterator i = s_leaderMap.find(leader.m_memberId);
		if (i != s_leaderMap.end())
		{
			Object *groupObj = NetworkIdManager::getObjectById((*i).second);
			if (groupObj)
			{
				ServerObject *groupServerObj = groupObj->asServerObject();
				if (groupServerObj)
					group = groupServerObj->asGroupObject();
			}
		}
	}

	if (!group)
	{
		group = safe_cast<GroupObject *>(ServerWorld::createNewObject(DEFAULT_GROUP_TEMPLATE, Transform::identity, 0, false));
		group->addToWorld();
		group->addGroupMember(leader);
		group->makeLootMaster(leader.m_memberId);
	}

	int const numberOfMembers = static_cast<int>(members.size());

	for (int i = 0; i < numberOfMembers; ++i)
	{
		group->addGroupMember(members[i]);
	}
}

// ----------------------------------------------------------------------

void GroupObject::setupUniverse()
{
}

// ----------------------------------------------------------------------

void GroupObject::getGroupMemberIds(std::vector<NetworkId> &members) const
{
	for (GroupMemberVector::const_iterator i = m_groupMembers.begin(); i != m_groupMembers.end(); ++i)
		members.push_back((*i).first);
}

// ----------------------------------------------------------------------

NetworkId const &GroupObject::getGroupLeaderId() const
{
	if (m_groupMembers.size())
		return (*m_groupMembers.begin()).first;
	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

std::string const &GroupObject::getGroupLeaderName() const
{
	static std::string nullStr;
	if (m_groupMembers.size())
		return (*m_groupMembers.begin()).second;
	return nullStr;
}

// ----------------------------------------------------------------------

bool GroupObject::isGroupFull() const
{
	return m_groupMembers.size() == cs_maximumNumberInGroup;
}

// ----------------------------------------------------------------------

int GroupObject::getPCMemberCount() const
{
	if (m_groupMembers.size() > m_nonPCMembers.size())
		return static_cast<int>(m_groupMembers.size() - m_nonPCMembers.size());

	return 0;
}

// ----------------------------------------------------------------------

bool GroupObject::isMemberPC(NetworkId const & memberId) const
{
	return !m_nonPCMembers.contains(memberId);
}

// ----------------------------------------------------------------------

bool GroupObject::doesGroupHaveRoomFor(int additionalMembers) const
{
	additionalMembers = std::max(0, additionalMembers);
	return (m_groupMembers.size() + additionalMembers) <= cs_maximumNumberInGroup;
}

// ----------------------------------------------------------------------

void GroupObject::makeLeader(NetworkId const &newLeaderId)
{
	if (!isMemberPC(newLeaderId))
	{
		return;
	}

	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_groupMakeLeader,
				0.0f,
				new MessageQueueGenericValueType<NetworkId>(newLeaderId),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		// swap the specified person with the first person in the group
		for (unsigned int pos = 1; pos < m_groupMembers.size(); ++pos)
		{
			if (m_groupMembers.get(pos).first == newLeaderId)
			{
				GroupUpdateObserver updater(this, Archive::ADOO_generic);
				GroupMember tempMember = m_groupMembers.get(pos);
				m_groupMembers.set(pos, m_groupMembers.get(0));
				m_groupMembers.set(0, tempMember);

				int16 tempMemberLevel = m_groupMemberLevels.get(pos);
				m_groupMemberLevels.set(pos, m_groupMemberLevels.get(0));
				m_groupMemberLevels.set(0, tempMemberLevel);

				uint8 tempMemberProfession = m_groupMemberProfessions.get(pos);
				m_groupMemberProfessions.set(pos, m_groupMemberProfessions.get(0));
				m_groupMemberProfessions.set(0, tempMemberProfession);

				GroupShipFormationMember tempMemberShip = m_groupShipFormationMembers.get(pos);
				m_groupShipFormationMembers.set(pos, m_groupShipFormationMembers.get(0));
				m_groupShipFormationMembers.set(0, tempMemberShip);

				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::setGroupName(std::string const &groupName)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_groupSetName,
				0.0f,
				new MessageQueueGenericValueType<std::string>(groupName),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		m_groupName = groupName;
	}
}

// ----------------------------------------------------------------------

void GroupObject::addGroupMember(GroupMemberParam const & member)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_groupAddMember,
				0.0f,
				new MessageQueueGenericValueType<GroupMemberParam>(member),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		if (isGroupFull())
		{
			Chat::sendSystemMessage(member.m_memberName, GroupStringId::SID_GROUP_JOIN_FULL, Unicode::String());
			return;
		}

		// Don't try to add someone if they are already a member
		if (isGroupMember(member.m_memberId))
		{
			return;
		}

		GroupUpdateObserver updater(this, Archive::ADOO_generic);

		m_groupMembers.push_back(std::make_pair(member.m_memberId, member.m_memberName));
		
		m_allMembers.insert(member.m_memberId);
		if (!member.m_memberIsPC)
			m_nonPCMembers.insert(member.m_memberId);

		m_groupShipFormationMembers.push_back(std::make_pair(member.m_memberShipId, m_groupShipFormationMembers.size()));

		if ((member.m_memberShipId.isValid()) && (member.m_memberShipIsPOB) && (member.m_memberOwnsPOB))
		{
			bool POBShipIsAdded = false;
			int const numberOfGroupPOBShipAndOwners = m_groupPOBShipAndOwners.size();

			for (int i = 0; i < numberOfGroupPOBShipAndOwners; ++i)
			{
				if (m_groupPOBShipAndOwners[i].first == member.m_memberShipId)
				{
					POBShipIsAdded = true;
				}
			}
			
			if (!POBShipIsAdded)
			{
				m_groupPOBShipAndOwners.push_back(std::make_pair(member.m_memberShipId, member.m_memberId));
			}
		}

		m_groupMemberLevels.push_back(static_cast<int16>(member.m_memberDifficulty));
		m_groupMemberProfessions.push_back(static_cast<uint8>(member.m_memberProfession));

		calcGroupLevel();

		if (m_groupMembers.size() == 1)
			createGroupChatRoom();
		addToGroupChatRoom(member.m_memberName);
		addToGroupVoiceChatRoom(member.m_memberId, member.m_memberName);
	}
}

// ----------------------------------------------------------------------

GroupMemberParam const GroupObject::createGroupMemberParamForMember(NetworkId const & memberId) const
{
	std::string const & memberName = getMemberName(memberId);
	int const memberLevel = getMemberLevel(memberId);
	LfgCharacterData::Profession const memberProfession = static_cast<LfgCharacterData::Profession>(getMemberProfession(memberId));
	bool const memberIsPC = !m_nonPCMembers.contains(memberId);
	NetworkId const & memberShipId = getMemberShip(memberId);
	bool const memberShipIsPOB = memberShipId.isValid() && getShipIsPOB(memberShipId);
	bool const memberOwnsPOB = memberShipId.isValid() && memberShipId == getPOBShipOwnedByMember(memberId);

	return GroupMemberParam(memberId, memberName, memberLevel, memberProfession, memberIsPC, memberShipId, memberShipIsPOB, memberOwnsPOB);
}

// ----------------------------------------------------------------------

void GroupObject::removeGroupMember(NetworkId const &memberId)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_groupRemoveMember,
				0.0f,
				new MessageQueueGenericValueType<NetworkId>(memberId),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		GroupUpdateObserver updater(this, Archive::ADOO_generic);
		for (unsigned int i = 0; i < m_groupMembers.size(); ++i)
		{
			GroupMember const & member = m_groupMembers.get(i);
			if (member.first == memberId)
			{
				removeFromGroupChatRoom(member.second);
				removeFromGroupVoiceChatRoom(memberId, member.second);

				m_groupMembers.erase(i);
				m_allMembers.erase(memberId);
				m_nonPCMembers.erase(memberId);
				m_groupShipFormationMembers.erase(i);
				m_groupMemberLevels.erase(i);
				m_groupMemberProfessions.erase(i);
				calcGroupLevel();

				for (unsigned int j = 0; j < m_groupPOBShipAndOwners.size(); ++j)
				{
					if (m_groupPOBShipAndOwners.get(j).second == memberId)
					{
						m_groupPOBShipAndOwners.erase(j);
						break;
					}
				}
				
				break;
			}
		}

		if (m_groupMembers.size() < 2)
		{
			disbandGroup();
		}
		else
		{
			NetworkId firstNonPet = getGroupLeaderId();

			if (!isMemberPC(firstNonPet))
			{
				unsigned int const numberOfGroupMembers = m_groupMembers.size();

				for (unsigned int i = 1; i < numberOfGroupMembers; ++i)
				{
					if (isMemberPC(m_groupMembers.get(i).first))
					{
						firstNonPet = m_groupMembers.get(i).first;
						break;
					}
				}

				if (isMemberPC(firstNonPet))
				{
					makeLeader(firstNonPet);
				}
			}

			if (getLootMasterId() == memberId)
			{
				m_lootMaster = getGroupLeaderId();
			}
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::disbandGroup()
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_groupDisband,
				0.0f,
				0,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (!getKill())
	{
		for (unsigned int i = 0; i < m_groupMembers.size(); ++i)
		{
			GroupMember const & member = m_groupMembers.get(i);
			removeFromGroupVoiceChatRoom(member.first, member.second);
		}

		permanentlyDestroy(DeleteReasons::Player);
		m_groupMembers.clear();
		m_allMembers.clear();
		m_nonPCMembers.clear();
		m_groupShipFormationMembers.clear();
		m_groupPOBShipAndOwners.clear();
		destroyGroupChatRoom();
	}
}

// ----------------------------------------------------------------------

std::string GroupObject::getChatRoomPath() const
{
	return getChatRoomPrefix() + getNetworkId().getValueString() + getChatRoomSuffix();
}

// ----------------------------------------------------------------------

std::string GroupObject::getVoiceChatDisplayName() const
{
	static std::string displayName("GroupChat");
	return displayName;
}

// ----------------------------------------------------------------------

std::string GroupObject::getChatName(std::string const &memberName) const
{
	std::string firstName;
	size_t pos = 0;
	Unicode::getFirstToken(memberName, 0, pos, firstName);
	return firstName;
}

// ----------------------------------------------------------------------

void GroupObject::createGroupChatRoom() const
{
	if (ConfigServerGame::getDebugGroupChat())
		LOG("GroupChat", ("Creating group chat room %s (group id %s)", getChatRoomPath().c_str(), getNetworkId().getValueString().c_str()));

	Chat::createRoom("System", false, getChatRoomPath(), getNetworkId().getValueString());
	
	//TODO: temporary separation of voice and text
	Chat::requestGetChannel(getChatRoomPath(), getVoiceChatDisplayName());
}

// ----------------------------------------------------------------------

void GroupObject::destroyGroupChatRoom() const
{
	if (ConfigServerGame::getDebugGroupChat())
		LOG("GroupChat", ("Destroying group chat room %s (group id %s)", getChatRoomPath().c_str(), getNetworkId().getValueString().c_str()));

	Chat::destroyRoom(getChatRoomPath());

	Chat::requestDestroyChannel(getChatRoomPath());
}

// ----------------------------------------------------------------------

bool GroupObject::chatRoomCreated() const
{
	return hasAttributeAttained(1);
}

// ----------------------------------------------------------------------

void GroupObject::setChatRoomCreated()
{
	setAttributeAttained(1);
}

// ----------------------------------------------------------------------

void GroupObject::addToGroupChatRoom(std::string const &memberName) const
{
	// note: if the room has not already been created, everyone will be added when the notification of its creation arrives.
	if (chatRoomCreated() && memberName.find("(") == std::string::npos)
	{
		if (ConfigServerGame::getDebugGroupChat())
			LOG("GroupChat", ("Invite and enter: member %s (group id %s), chat room %s", getChatName(memberName).c_str(), getNetworkId().getValueString().c_str(), getChatRoomPath().c_str()));
		Chat::invite(getChatName(memberName), getChatRoomPath());
		Chat::enterRoom(getChatName(memberName), getChatRoomPath(), false, false);
	}
}

// ----------------------------------------------------------------------

void GroupObject::addToGroupVoiceChatRoom(NetworkId const & memberId, std::string const & memberName) const
{
	//TODO: temporary separation of voice and text
	Chat::requestAddClientToChannel(memberId, getChatName(memberName), getChatRoomPath(), true);
}

// ----------------------------------------------------------------------

void GroupObject::removeFromGroupChatRoom(std::string const &memberName) const
{
	if (memberName.find("(") == std::string::npos)
	{
		if (ConfigServerGame::getDebugGroupChat())
			LOG("GroupChat", ("Exit and uninvite: member %s (group id %s), chat room %s", getChatName(memberName).c_str(), getNetworkId().getValueString().c_str(), getChatRoomPath().c_str()));
		Chat::exitRoom(getChatName(memberName), getChatRoomPath());
		Chat::uninvite(getChatName(memberName), getChatRoomPath());
	}
}

// ----------------------------------------------------------------------

void GroupObject::removeFromGroupVoiceChatRoom(NetworkId const & memberId, std::string const & memberName) const
{
	//TODO: temporary separation of voice and text
	Chat::requestRemoveClientFromChannel(memberId, memberName, getChatRoomPath());
}

// ----------------------------------------------------------------------

void GroupObject::sendGroupChat(NetworkId const &memberId, Unicode::String const &text) const
{
	bool sent = false;
	for (unsigned int i = 0; i < m_groupMembers.size(); ++i)
	{
		if (m_groupMembers.get(i).first == memberId)
		{
			sent = true;
			if (ConfigServerGame::getDebugGroupChat())
				LOG("GroupChat", ("sendGroupChat: member %s (group id %s), chat room %s, text %s", getChatName(m_groupMembers.get(i).second).c_str(), getNetworkId().getValueString().c_str(), getChatRoomPath().c_str(), Unicode::wideToNarrow(text).c_str()));
			Chat::sendToRoom(getChatName(m_groupMembers.get(i).second), getChatRoomPath(), text, Unicode::String());
		}
	}

	if (!sent && ConfigServerGame::getDebugGroupChat())
		LOG("GroupChat", ("sendGroupChat could not find member id %s (group id %s)", memberId.getValueString().c_str(), getNetworkId().getValueString().c_str()));
}

// ----------------------------------------------------------------------

void GroupObject::onGroupChatRoomCreated()
{
	if (ConfigServerGame::getDebugGroupChat())
		LOG("GroupChat", ("group chat room created (group id %s)", getNetworkId().getValueString().c_str()));

	setChatRoomCreated();
	for (unsigned int i = 0; i < m_groupMembers.size(); ++i)
		addToGroupChatRoom(m_groupMembers.get(i).second);
}

// ----------------------------------------------------------------------

void GroupObject::onGroupVoiceChatRoomCreated()
{
	if (ConfigServerGame::getDebugGroupChat())
		LOG("GroupChat", ("group voice chat room created (group id %s)", getNetworkId().getValueString().c_str()));

	for (unsigned int i = 0; i < m_groupMembers.size(); ++i)
	{
		GroupMember const & member = m_groupMembers.get(i);
		addToGroupVoiceChatRoom(member.first, member.second);
	}
}

// ----------------------------------------------------------------------

void GroupObject::onChatRoomCreate(std::string const &path) // static
{
	if (ConfigServerGame::getDebugGroupChat())
		LOG("GroupChat", ("onChatRoomCreate (%s)", path.c_str()));

	std::string const &prefix = getChatRoomPrefix();
	// is it a group channel?
	if (!path.compare(0, prefix.length(), prefix))
	{
		// it's a group channel, so extract the networkId
		NetworkId id(path.substr(prefix.length(), path.length()-prefix.length()-getChatRoomSuffix().length()));
		Object *obj = NetworkIdManager::getObjectById(id);
		if (obj)
		{
			ServerObject *serverObject = obj->asServerObject();
			if (serverObject)
			{
				GroupObject *groupObject = serverObject->asGroupObject();
				if (groupObject && groupObject->isAuthoritative())
					groupObject->onGroupChatRoomCreated();
			}
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::onVoiceChatRoomCreate(std::string const &name) // static
{
	if (ConfigServerGame::getDebugGroupChat())
		LOG("GroupChat", ("onVoiceChatRoomCreate (%s)", name.c_str()));

	std::string const &prefix = getChatRoomPrefix();
	// is it a group channel?
	if (!name.compare(0, prefix.length(), prefix))
	{
		// it's a group channel, so extract the networkId
		std::string strid = name.substr(prefix.length());
		NetworkId id(strid);
		Object *obj = NetworkIdManager::getObjectById(id);
		if (obj)
		{
			ServerObject *serverObject = obj->asServerObject();
			if (serverObject)
			{
				GroupObject *groupObject = serverObject->asGroupObject();
				if (groupObject && groupObject->isAuthoritative())
					groupObject->onGroupVoiceChatRoomCreated();
			}
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::getAttributes(stdvector<std::pair<std::string, Unicode::String> >::fwd &data) const
{
	UniverseObject::getAttributes(data);
}

// ----------------------------------------------------------------------

void GroupObject::calcGroupLevel()
{
	FATAL(!isAuthoritative(), ("calcGroupLevel called on nonauth group"));

	int maxPlayerLevel = 0;
	int groupLevel = 0;

	const unsigned int numMembers = m_groupMembers.size();

	if (numMembers > 0)
	{
		for (unsigned int i = 0; i < numMembers; ++i)
		{
			if (isMemberPC(m_groupMembers.get(i).first))
			{
				const int memberLevel = m_groupMemberLevels.get(i);
				maxPlayerLevel = (maxPlayerLevel < memberLevel) ? memberLevel : maxPlayerLevel;
			}
		}

		groupLevel = maxPlayerLevel;
	}

	m_groupLevel = static_cast<int16>(groupLevel);
}

// ----------------------------------------------------------------------

void GroupObject::setMemberLevel(NetworkId const &memberId, int memberLevel)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_setGroupMemberDifficulty,
				0.0f,
				new MessageQueueGenericValueType<std::pair<NetworkId, int> >(std::make_pair(memberId, memberLevel)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		for (unsigned int i = 0; i < m_groupMembers.size(); ++i)
		{
			if (m_groupMembers.get(i).first == memberId)
			{
				m_groupMemberLevels.set(i, static_cast<int16>(memberLevel));
				calcGroupLevel();
			}
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::setMemberProfession(NetworkId const &memberId, uint8 profession)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_setGroupMemberProfession,
				0.0f,
				new MessageQueueGenericValueType<std::pair<NetworkId, uint32> >(std::make_pair(memberId, static_cast<uint32>(profession))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		for (unsigned int i = 0; i < m_groupMembers.size(); ++i)
		{
			if (m_groupMembers.get(i).first == memberId)
			{
				m_groupMemberProfessions.set(i, profession);
			}
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::onGroupMemberConnect(NetworkId const &memberId)
{
	// try to add them to the group chat room again
	for (unsigned int i = 0; i < m_groupMembers.size(); ++i)
		if (m_groupMembers.get(i).first == memberId)
		{
			GroupMember const & member = m_groupMembers.get(i);
			addToGroupChatRoom(member.second);
			addToGroupVoiceChatRoom(member.first, member.second);
		}
}

// ----------------------------------------------------------------------

GroupObject *GroupObject::asGroupObject()
{
	return this;
}

// ----------------------------------------------------------------------

GroupObject const *GroupObject::asGroupObject() const
{
	return this;
}

// ----------------------------------------------------------------------

void GroupObject::onGroupMemberRemoved(NetworkId const &memberId, bool disbanding)
{
	// have removed members with clients unobserve the group object, clear their group id, and get remove or disband messages
	Object *o = NetworkIdManager::getObjectById(memberId);
	if (o)
	{
		ServerObject *so = o->asServerObject();
		if (so)
		{
			CreatureObject *creature = so->asCreatureObject();
			if (creature && creature->isAuthoritative())
			{
				if (creature->getGroup() == this)
				{
					if (creature->getScriptObject())
					{
						ScriptParams scriptParams;
						scriptParams.addParam(getNetworkId());
						IGNORE_RETURN(creature->getScriptObject()->trigAllScripts(Scripting::TRIG_REMOVED_FROM_GROUP, scriptParams));
						if (disbanding)
						{
							IGNORE_RETURN(creature->getScriptObject()->trigAllScripts(Scripting::TRIG_GROUP_DISBANDED, scriptParams));
						}
					}

					creature->setGroup(0, disbanding);
					Client *client = creature->getClient();
					if (client)
					{
						ObserveTracker::onClientLeftGroup(*client, *this);
						ProsePackage pp;
						pp.stringId = disbanding ? GroupStringId::SID_GROUP_DISBANDED : GroupStringId::SID_GROUP_REMOVED;
						Chat::sendSystemMessage(*creature, pp);
					}

					//-- Update just the creature.  The group will be updated in ~GroupMemberObserver
					GroupMissionCriticalObjectsBuilder::updateGroupMissionCriticalObjects(*creature, false);
					if (creature->isPlayerControlled())
					{
						PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(creature);
						if (playerObject)
							GroupWaypointBuilder::updateGroupWaypoints(*playerObject, false);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::setFormationNameCrc(uint32 const formationNameCrc)
{
	m_formationNameCrc.set(formationNameCrc);
}

// ----------------------------------------------------------------------

void GroupObject::setShipForMember(NetworkId const & memberId, NetworkId const & shipId)
{
	if (isAuthoritative())
	{
		FATAL((m_groupMembers.size() != m_groupShipFormationMembers.size()), ("GroupObject::setShipForMember: m_groupMembers should always be the same size as m_groupShipFormationMembers"));

		int const numberOfGroupMembers = m_groupMembers.size();

		for (int i = 0; i < numberOfGroupMembers; ++i)
		{
			if (m_groupMembers[i].first == memberId)
			{
				GroupShipFormationMember data = m_groupShipFormationMembers[i];
				data.first = shipId;
				m_groupShipFormationMembers.set(i, data);
				return;
			}
		}
	}
	else
	{
		Controller *controller = getController();
		if (controller != 0)
		{
			controller->appendMessage(
				CM_setGroupShipForMember,
				0.0f,
				new MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> >(std::make_pair(memberId, shipId)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::setShipFormationSlotForMember(NetworkId const & memberId, int const formationSlot)
{
	if (isAuthoritative())
	{
		FATAL((m_groupMembers.size() != m_groupShipFormationMembers.size()), ("GroupObject::setShipForMember: m_groupMembers should always be the same size as m_groupShipFormationMembers"));

		int const numberOfGroupMembers = m_groupMembers.size();
		int previousFormationSlotOwnerIndex = -1;
		int newFormationSlotOwnerIndex = -1;

		for (int i = 0; i < numberOfGroupMembers; ++i)
		{
			// find the previous owner of this slot
			if (m_groupShipFormationMembers[i].second == formationSlot)
			{
				previousFormationSlotOwnerIndex = i;
			}

			// find the next owner of this slot
			if (m_groupMembers[i].first == memberId)
			{
				newFormationSlotOwnerIndex = i;
			}
		}

		// set the previous owner's slot to the current member's slot
		// and then set the member's slot to the new value.
		// since we always set the slot values in ::addGroupMember
		// we can just swap the slots to keep someone from sitting
		// in 2 slots at the same time.
		if (newFormationSlotOwnerIndex > -1)
		{
			GroupShipFormationMember data = m_groupShipFormationMembers[newFormationSlotOwnerIndex];
			int const slotToSwapOccupantTo = data.second;
			data.second = formationSlot;
			m_groupShipFormationMembers.set(newFormationSlotOwnerIndex, data);

			if (previousFormationSlotOwnerIndex > -1)
			{
				GroupShipFormationMember previousData = m_groupShipFormationMembers[previousFormationSlotOwnerIndex];
				previousData.second = slotToSwapOccupantTo;
				m_groupShipFormationMembers.set(previousFormationSlotOwnerIndex, previousData);
			}
		}
	}
	else
	{
		Controller *controller = getController();
		if (controller != 0)
		{
			controller->appendMessage(
				CM_setGroupShipFormationSlotForMember,
				0.0f,
				new MessageQueueGenericValueType<std::pair<NetworkId, int> >(std::make_pair(memberId, formationSlot)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::addPOBShipAndOwner(NetworkId const & shipId, NetworkId const & ownerId)
{
	if (isAuthoritative())
	{
		// make sure that the POB ship isn't already in the list
		int const numberOfGroupPOBShipAndOwners = m_groupPOBShipAndOwners.size();

		for (int i = 0; i < numberOfGroupPOBShipAndOwners; ++i)
		{
			if (m_groupPOBShipAndOwners[i].first == shipId)
			{
				if (m_groupPOBShipAndOwners[i].second != ownerId)
				{
					GroupPOBShipAndOwner data = m_groupPOBShipAndOwners[i];
					data.second = ownerId;
					m_groupPOBShipAndOwners.set(i, data);
				}
				return;
			}
		}

		// add if doesn't exist
		m_groupPOBShipAndOwners.push_back(std::make_pair(shipId, ownerId));
	}
	else
	{
		Controller *controller = getController();
		if (controller != 0)
		{
			controller->appendMessage(
				CM_addGroupPOBShipAndOwner,
				0.0f,
				new MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> >(std::make_pair(shipId, ownerId)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
}

// ----------------------------------------------------------------------

void GroupObject::removePOBShip(NetworkId const & shipId)
{
	if (isAuthoritative())
	{
		int const numberOfGroupPOBShipAndOwners = m_groupPOBShipAndOwners.size();

		for (int i = 0; i < numberOfGroupPOBShipAndOwners; ++i)
		{
			if (m_groupPOBShipAndOwners[i].first == shipId)
			{
				m_groupPOBShipAndOwners.erase(i);
				break;
			}
		}
	}
	else
	{
		Controller *controller = getController();
		if (controller != 0)
		{
			controller->appendMessage(
				CM_removeGroupPOBShip,
				0.0f,
				new MessageQueueGenericValueType<NetworkId>(shipId),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
}

// ----------------------------------------------------------------------

NetworkId const & GroupObject::getPOBShipOwnedByMember(NetworkId const & memberId) const
{
	int const numberOfGroupPOBShipAndOwners = m_groupPOBShipAndOwners.size();

	for (int i = 0; i < numberOfGroupPOBShipAndOwners; ++i)
	{
		if (m_groupPOBShipAndOwners[i].second == memberId)
		{
			return m_groupPOBShipAndOwners[i].first;
		}
	}
	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

NetworkId const & GroupObject::getPOBShipMemberIsContainedIn(NetworkId const & memberId) const
{
	NetworkId const & shipId = getMemberShip(memberId);

	if (shipId.isValid())
	{
		int const numberOfGroupPOBShipAndOwners = m_groupPOBShipAndOwners.size();

		for (int i = 0; i < numberOfGroupPOBShipAndOwners; ++i)
		{
			if (m_groupPOBShipAndOwners[i].first == shipId)
			{
				return shipId;
			}
		}
	}
	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

NetworkId const & GroupObject::getMemberShip(NetworkId const & memberId) const
{
	int const numberOfGroupMembers = m_groupMembers.size();

	for (int i = 0; i < numberOfGroupMembers; ++i)
	{
		if (m_groupMembers[i].first == memberId)
		{
			return m_groupShipFormationMembers[i].first;
		}
	}
	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

std::string const & GroupObject::getMemberName(NetworkId const & memberId) const
{
	int const numberOfGroupMembers = m_groupMembers.size();

	for (int i = 0; i < numberOfGroupMembers; ++i)
	{
		if (m_groupMembers[i].first == memberId)
		{
			return m_groupMembers.get(i).second;
		}
	}

	return cs_emptyString;
}

// ----------------------------------------------------------------------

int GroupObject::getMemberLevel(NetworkId const & memberId) const
{
	int const numberOfGroupMembers = m_groupMembers.size();

	for (int i = 0; i < numberOfGroupMembers; ++i)
	{
		if (m_groupMembers[i].first == memberId)
		{
			return m_groupMemberLevels.get(i);
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

int GroupObject::getMemberLevelByMemberIndex(unsigned int memberIndex) const
{
	if (memberIndex < m_groupMembers.size())
		return m_groupMemberLevels.get(memberIndex);

	return 0;
}

// ----------------------------------------------------------------------

uint8 GroupObject::getMemberProfession(NetworkId const & memberId) const
{
	int const numberOfGroupMembers = m_groupMembers.size();

	for (int i = 0; i < numberOfGroupMembers; ++i)
	{
		if (m_groupMembers[i].first == memberId)
		{
			return m_groupMemberProfessions.get(i);
		}
	}

	return static_cast<uint8>(LfgCharacterData::Prof_Unknown);
}

// ----------------------------------------------------------------------

uint8 GroupObject::getMemberProfessionByMemberIndex(unsigned int memberIndex) const
{
	if (memberIndex < m_groupMembers.size())
		return m_groupMemberProfessions.get(memberIndex);

	return static_cast<uint8>(LfgCharacterData::Prof_Unknown);
}

// ----------------------------------------------------------------------

bool GroupObject::getShipIsPOB(NetworkId const & shipId) const
{
	int const numberOfGroupPOBShipAndOwners = m_groupPOBShipAndOwners.size();

	for (int i = 0; i < numberOfGroupPOBShipAndOwners; ++i)
	{
		if (m_groupPOBShipAndOwners[i].first == shipId)
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

NetworkId const & GroupObject::getLootMasterId() const
{
	return m_lootMaster.get();
}

// ----------------------------------------------------------------------

void GroupObject::makeLootMaster(NetworkId const & newLootMasterId)
{
	if (!isMemberPC(newLootMasterId))
	{
		return;
	}
	
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller != 0)
		{
			controller->appendMessage(
				CM_groupMakeLootMaster,
				0.0f,
				new MessageQueueGenericValueType<NetworkId>(newLootMasterId),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		GroupUpdateObserver updater(this, Archive::ADOO_generic);
	
		int const numberOfGroupMembers = m_groupMembers.size();

		for (int i = 0; i < numberOfGroupMembers; ++i)
		{
			if (m_groupMembers[i].first == newLootMasterId)
			{
				m_lootMaster = newLootMasterId;
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

int GroupObject::getLootRule() const
{
	return m_lootRule.get();
}

// ----------------------------------------------------------------------

void GroupObject::setLootRule(int const rule)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller != 0)
		{
			controller->appendMessage(
				CM_groupSetLootRule,
				0.0f,
				new MessageQueueGenericValueType<int>(rule),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		m_lootRule = static_cast<uint32>(rule);
	}
}

// ----------------------------------------------------------------------

unsigned int GroupObject::getSecondsLeftOnGroupPickup() const
{
	std::pair<int32, int32> const & groupPickupTimer = m_groupPickupTimer.get();
	if ((groupPickupTimer.first > 0) && (groupPickupTimer.second > 0))
	{
		time_t const timeNow = ::time(nullptr);
		if (groupPickupTimer.second > timeNow)
			return (groupPickupTimer.second - (int)timeNow);
	}

	return 0;
}

// ----------------------------------------------------------------------

void GroupObject::setGroupPickupTimer(time_t startTime, time_t endTime)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_setGroupPickupTimer,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int32, int32> >(std::make_pair(static_cast<int32>(startTime), static_cast<int32>(endTime))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		m_groupPickupTimer.set(std::make_pair(static_cast<int32>(startTime), static_cast<int32>(endTime)));
	}
}

// ----------------------------------------------------------------------

void GroupObject::setGroupPickupLocation(std::string const & planetName, Vector const & location)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_setGroupPickupLocation,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::string, Vector> >(std::make_pair(planetName, location)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		m_groupPickupLocation.set(std::make_pair(planetName, location));
	}
}

// ----------------------------------------------------------------------

void GroupObject::reenterGroupChatRoom(CreatureObject const &who) const
{
	if (who.getGroup() != this)
		return;

	std::string firstName;
	size_t pos = 0;
	IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));

	std::string const groupChatRoomPath = getChatRoomPath();
	Chat::exitRoom(firstName, groupChatRoomPath);
	Chat::uninvite(firstName, groupChatRoomPath);
	Chat::invite(firstName, groupChatRoomPath);
	Chat::enterRoom(firstName, groupChatRoomPath, false, false);
}

// ======================================================================
