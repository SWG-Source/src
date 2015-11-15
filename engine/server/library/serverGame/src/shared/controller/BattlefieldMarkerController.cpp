// ======================================================================
//
// BattlefieldMarkerController.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/BattlefieldMarkerController.h"

#include "serverGame/BattlefieldMarkerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

// ======================================================================

BattlefieldMarkerController::BattlefieldMarkerController(BattlefieldMarkerObject *newOwner) :
	TangibleController(newOwner)
{
}

// ----------------------------------------------------------------------

BattlefieldMarkerController::~BattlefieldMarkerController()
{
}

// ----------------------------------------------------------------------

void BattlefieldMarkerController::handleMessage(const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	BattlefieldMarkerObject *owner = dynamic_cast<BattlefieldMarkerObject *>(getOwner());
	NOT_NULL(owner);

	switch (message)
	{
	case CM_setBattlefieldParticipant:
		{
			MessageQueueGenericValueType<std::pair<NetworkId, uint32> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<NetworkId, uint32> > const *>(data);
			owner->setBattlefieldParticipant(msg->getValue().first, msg->getValue().second);
		}
		break;
		
	case CM_clearBattlefieldParticipants:
		{
			owner->clearBattlefieldParticipants();
		}
		break;
		
	default:
		TangibleController::handleMessage(message, value, data, flags);
		break;
	}
}

// ======================================================================

