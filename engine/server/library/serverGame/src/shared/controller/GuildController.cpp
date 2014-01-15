// ======================================================================
//
// GuildController.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GuildController.h"

#include "serverGame/GuildObject.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

// ======================================================================

GuildController::GuildController(GuildObject *newOwner) :
	UniverseController(newOwner)
{
}

// ----------------------------------------------------------------------

GuildController::~GuildController()
{
}

// ----------------------------------------------------------------------

void GuildController::handleMessage(const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	GuildObject *owner = dynamic_cast<GuildObject *>(getOwner());
	NOT_NULL(owner);

	switch (message)
	{
	case CM_guildCreate:
		{
			MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::string> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::string> > > const *>(data);
			owner->createGuild(msg->getValue().second.first, msg->getValue().second.second, msg->getValue().first);
		}
		break;
	case CM_guildDisband:
		{
			MessageQueueGenericValueType<int> const *msg = safe_cast<MessageQueueGenericValueType<int> const *>(data);
			owner->disbandGuild(msg->getValue());
		}
		break;
	case CM_guildRemoveMember:
		{
			MessageQueueGenericValueType<std::pair<int, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, NetworkId> > const *>(data);
			owner->removeGuildMember(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_guildAddCreatorMember:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > > const *>(data);
			owner->addGuildCreatorMember(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second.first, msg->getValue().second.second.second);
		}
		break;
	case CM_guildAddSponsorMember:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > > const *>(data);
			owner->addGuildSponsorMember(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second.first, msg->getValue().second.second.second);
		}
		break;
	case CM_guildSetMemberPermission:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, int>, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, int>, NetworkId> > const *>(data);
			owner->setGuildMemberPermission(msg->getValue().first.first, msg->getValue().second, msg->getValue().first.second);
		}
		break;
	case CM_guildSetMemberTitle:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *>(data);
			owner->setGuildMemberTitle(msg->getValue().second.second, msg->getValue().second.first, msg->getValue().first);
		}
		break;
	case CM_guildSetMemberAllegiance:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, NetworkId> > const *>(data);
			owner->setGuildMemberAllegiance(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second);
		}
		break;
	case CM_guildSetMemberPermissionAndAllegiance:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, NetworkId> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, NetworkId> > > const *>(data);
			owner->setGuildMemberPermissionAndAllegiance(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second);
		}
		break;
	case CM_guildSetMemberNameAndPermission:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *>(data);
			owner->setGuildMemberNameAndPermision(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second);
		}
		break;
	case CM_guildSetMemberProfessionInfo:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *>(data);
			owner->setGuildMemberProfessionInfo(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second);
		}
		break;
	case CM_guildAddMemberRank:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *>(data);
			owner->addGuildMemberRank(msg->getValue().second.second, msg->getValue().second.first, msg->getValue().first);
		}
		break;
	case CM_guildRemoveMemberRank:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *>(data);
			owner->removeGuildMemberRank(msg->getValue().second.second, msg->getValue().second.first, msg->getValue().first);
		}
		break;
	case CM_guildSetLeader:
		{
			MessageQueueGenericValueType<std::pair<int, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, NetworkId> > const *>(data);
			owner->setGuildLeader(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_guildSetGuildElectionEndTime:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, int>, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, int>, int> > const *>(data);
			owner->setGuildElectionEndTime(msg->getValue().second, msg->getValue().first.first, msg->getValue().first.second);
		}
		break;
	case CM_guildSetGuildFaction:
		{
			MessageQueueGenericValueType<std::pair<int, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, int> > const *>(data);
			owner->setGuildFaction(msg->getValue().first, static_cast<uint32>(msg->getValue().second));
		}
		break;
	case CM_guildSetGuildGcwDefenderRegion:
		{
			MessageQueueGenericValueType<std::pair<int, std::string> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::string> > const *>(data);
			owner->setGuildGcwDefenderRegion(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_guildRemoveEnemy:
		{
			MessageQueueGenericValueType<std::pair<int, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, int> > const *>(data);
			owner->removeGuildEnemy(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_guildSetEnemy:
		{
			MessageQueueGenericValueType<std::pair<int, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, int> > const *>(data);
			owner->setGuildEnemy(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_guildUpdateGuildWarKillTrackingData:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, int>, std::pair<int, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, int>, std::pair<int, int> > > const *>(data);
			owner->modifyGuildWarKillTracking(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second);
		}
		break;
	case CM_guildSetName:
		{
			MessageQueueGenericValueType<std::pair<int, std::string> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::string> > const *>(data);
			owner->setGuildName(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_guildSetAbbrev:
		{
			MessageQueueGenericValueType<std::pair<int, std::string> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::string> > const *>(data);
			owner->setGuildAbbrev(msg->getValue().first, msg->getValue().second);
		}
		break;
	default:
		UniverseController::handleMessage(message, value, data, flags);
		break;
	}
}

// ======================================================================

