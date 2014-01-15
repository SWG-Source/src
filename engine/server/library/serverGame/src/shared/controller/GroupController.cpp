// ======================================================================
//
// GroupController.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GroupController.h"

#include "serverGame/GroupObject.h"
#include "serverNetworkMessages/GroupMemberParam.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

// ======================================================================

GroupController::GroupController(GroupObject *newOwner) :
	UniverseController(newOwner)
{
}

// ----------------------------------------------------------------------

GroupController::~GroupController()
{
}

// ----------------------------------------------------------------------

void GroupController::handleMessage(const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	GroupObject *owner = dynamic_cast<GroupObject *>(getOwner());
	NOT_NULL(owner);

	switch (message)
	{
	case CM_groupSetName:
		{
			MessageQueueGenericValueType<std::string> const *msg = safe_cast<MessageQueueGenericValueType<std::string> const *>(data);
			if (msg)
				owner->setGroupName(msg->getValue());
		}
		break;
	case CM_groupAddMember:
		{
			MessageQueueGenericValueType<GroupMemberParam> const *msg = safe_cast<MessageQueueGenericValueType<GroupMemberParam> const *>(data);
			if (msg)
			{
				owner->addGroupMember(msg->getValue());
			}
		}
		break;
	case CM_groupRemoveMember:
		{
			MessageQueueGenericValueType<NetworkId> const *msg = safe_cast<MessageQueueGenericValueType<NetworkId> const *>(data);
			if (msg)
				owner->removeGroupMember(msg->getValue());
		}
		break;
	case CM_groupDisband:
		{
			owner->disbandGroup();
		}
		break;
	case CM_groupMakeLeader:
		{
			MessageQueueGenericValueType<NetworkId> const *msg = safe_cast<MessageQueueGenericValueType<NetworkId> const *>(data);
			if (msg)
				owner->makeLeader(msg->getValue());
		}
		break;
	case CM_setGroupMemberDifficulty:
		{
			MessageQueueGenericValueType<std::pair<NetworkId, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<NetworkId, int> > const *>(data);
			if (msg)
				owner->setMemberLevel(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_setGroupMemberProfession:
		{
			MessageQueueGenericValueType<std::pair<NetworkId, uint32> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<NetworkId, uint32> > const *>(data);
			if (msg)
				owner->setMemberProfession(msg->getValue().first, static_cast<uint8>(msg->getValue().second));
		}
		break;
	case CM_setGroupShipForMember:
		{
			MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > const *>(data);
			if (msg)
				owner->setShipForMember(msg->getValue().first, msg->getValue().second);
		}
		break;

	case CM_setGroupShipFormationSlotForMember:
		{
			MessageQueueGenericValueType<std::pair<NetworkId, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<NetworkId, int> > const *>(data);
			if (msg)
				owner->setShipFormationSlotForMember(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_addGroupPOBShipAndOwner:
		{
			MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > const *>(data);
			if (msg)
				owner->addPOBShipAndOwner(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_removeGroupPOBShip:
		{
			MessageQueueGenericValueType<NetworkId> const *msg = safe_cast<MessageQueueGenericValueType<NetworkId> const *>(data);
			if (msg)
				owner->removePOBShip(msg->getValue());
		}
		break;

	case CM_groupMakeLootMaster:
		{
			MessageQueueGenericValueType<NetworkId> const *msg = safe_cast<MessageQueueGenericValueType<NetworkId> const *>(data);
			if (msg)
				owner->makeLootMaster(msg->getValue());
		}
		break;

	case CM_groupSetLootRule:
		{
			MessageQueueGenericValueType<int> const *msg = safe_cast<MessageQueueGenericValueType<int> const *>(data);
			if (msg)
				owner->setLootRule(msg->getValue());
		}
		break;

	case CM_setGroupPickupTimer:
		{
			MessageQueueGenericValueType<std::pair<int32, int32> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int32, int32> > const *>(data);
			if (msg)
				owner->setGroupPickupTimer(static_cast<time_t>(msg->getValue().first), static_cast<time_t>(msg->getValue().second));
		}
		break;

	case CM_setGroupPickupLocation:
		{
			MessageQueueGenericValueType<std::pair<std::string, Vector> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, Vector> > const *>(data);
			if (msg)
				owner->setGroupPickupLocation(msg->getValue().first, msg->getValue().second);
		}
		break;

	default:
		UniverseController::handleMessage(message, value, data, flags);
		break;
	}
}

// ======================================================================

