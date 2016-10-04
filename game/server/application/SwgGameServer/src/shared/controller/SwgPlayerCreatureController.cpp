// ======================================================================
//
// SwgPlayerCreatureController.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/SwgPlayerCreatureController.h"

#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "SwgGameServer/SwgCreatureObject.h"
#include "SwgGameServer/SwgPlayerObject.h"


// ======================================================================

SwgPlayerCreatureController::SwgPlayerCreatureController(SwgCreatureObject *newOwner) :
	PlayerCreatureController(newOwner)
{
}

// ----------------------------------------------------------------------

SwgPlayerCreatureController::~SwgPlayerCreatureController()
{
}

// ----------------------------------------------------------------------

void SwgPlayerCreatureController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	SwgCreatureObject * const owner = safe_cast<SwgCreatureObject*>(getOwner());
	SwgPlayerObject * playerOwner = safe_cast<SwgPlayerObject*>(getPlayerObject(owner));
	NOT_NULL(playerOwner);

	switch (message)
	{
	case CM_setJediState:
		{
			const MessageQueueGenericValueType<int> * const msg = dynamic_cast<const MessageQueueGenericValueType<int> *>(data);
			if (msg != nullptr)
				playerOwner->setJediState(static_cast<JediState>(msg->getValue()));
		}
		break;

	default:
		PlayerCreatureController::handleMessage(message, value, data, flags);
		break;
	}
}

// ----------------------------------------------------------------------

