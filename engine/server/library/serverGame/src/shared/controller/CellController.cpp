// CellController.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/CellController.h"
#include "serverGame/CellObject.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

//-----------------------------------------------------------------------

CellController::CellController(CellObject * newOwner) :
	ServerController(newOwner)
{
}

//-----------------------------------------------------------------------

CellController::~CellController()
{
}

//-----------------------------------------------------------------------

void CellController::handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags)
{
	CellObject * owner = dynamic_cast<CellObject *>(getOwner());
	NOT_NULL(owner);

	switch (message)
	{
	case CM_addAllowed:
	{
		const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if (msg)
		{
			owner->addAllowed(msg->getValue());
		}
	}
	break;
	case CM_removeAllowed:
	{
		const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if (msg)
		{
			owner->removeAllowed(msg->getValue());
		}
	}
	break;
	case CM_addBanned:
	{
		const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if (msg)
		{
			owner->addBanned(msg->getValue());
		}
	}
	break;
	case CM_removeBanned:
	{
		const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if (msg)
		{
			owner->removeBanned(msg->getValue());
		}
	}
	break;
	default:
		ServerController::handleMessage(message, value, data, flags);
		break;
	}
}

//-----------------------------------------------------------------------

void CellController::setGoal(Transform const &, ServerObject *, bool teleport = false)
{
	DEBUG_WARNING(true, ("Cell setGoal was called for CellObject %s. Cells may not have goals!", getOwner()->getNetworkId().getValueString().c_str()));
}

//-----------------------------------------------------------------------