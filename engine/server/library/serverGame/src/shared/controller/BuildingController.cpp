//BuildingController.cpp

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/BuildingController.h"
#include "serverGame/BuildingObject.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

//-----------------------------------------------------------------------

BuildingController::BuildingController(BuildingObject * newOwner) :
		TangibleController(newOwner)
{
}

//-----------------------------------------------------------------------

BuildingController::~BuildingController()
{
}

//-----------------------------------------------------------------------

void BuildingController::handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags)
{
	BuildingObject * owner = dynamic_cast<BuildingObject *>(getOwner());
	NOT_NULL(owner);
	
	switch (message)
	{
	case CM_addAllowed:
		{
			const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
			if(msg)
			{
				owner->addAllowed(msg->getValue());
			}
		}
		break;
	case CM_removeAllowed:
		{
			const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
			if(msg)
			{
				owner->removeAllowed(msg->getValue());
			}
		}
		break;
	case CM_addBanned:
		{
			const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
			if(msg)
			{
				owner->addBanned(msg->getValue());
			}
		}
		break;
	case CM_removeBanned:
		{
			const MessageQueueGenericValueType<std::string> * const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
			if(msg)
			{
				owner->removeBanned(msg->getValue());
			}
		}
		break;
	case CM_setBuildingIsPublic:
		{
			MessageQueueGenericValueType<bool> const *msg = safe_cast<MessageQueueGenericValueType<bool> const *>(data);
			if (msg)
				owner->setIsPublic(msg->getValue());
		}
		break;
	case CM_setBuildingCityId:
		{
			MessageQueueGenericValueType<int> const *msg = safe_cast<MessageQueueGenericValueType<int> const *>(data);
			if (msg)
				owner->setCityId(msg->getValue());
		}
		break;
	default:
		TangibleController::handleMessage(message, value, data, flags);
		break;
	}
}
//-----------------------------------------------------------------------

