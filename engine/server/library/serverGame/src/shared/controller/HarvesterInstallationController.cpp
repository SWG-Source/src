//========================================================================
//
// HarvesterInstallationController.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/HarvesterInstallationController.h"

#include "serverGame/HarvesterInstallationObject.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "swgServerNetworkMessages/MessageQueueResourceAdd.h"
#include "swgServerNetworkMessages/MessageQueueResourceSetInstalledEfficiency.h"

//-----------------------------------------------------------------------

HarvesterInstallationController::HarvesterInstallationController(HarvesterInstallationObject * newOwner) :
	InstallationController(newOwner)
{
}

//-----------------------------------------------------------------------

HarvesterInstallationController::~HarvesterInstallationController()
{
}

//-----------------------------------------------------------------------

void HarvesterInstallationController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	HarvesterInstallationObject* owner = dynamic_cast<HarvesterInstallationObject *>(getOwner());
	NOT_NULL(owner);

	switch (message)
	{
		case CM_resourceSetInstalledEfficiency:
		{
			const MessageQueueResourceSetInstalledEfficiency *msg=
				dynamic_cast<const MessageQueueResourceSetInstalledEfficiency *>(data);
			NOT_NULL(msg);
			DEBUG_REPORT_LOG(true,("Got MessageQueueResourceSetInstalledEfficiency with value %f\n",msg->getInstalledEfficiency()));

			owner->finishActivate(msg->getInstalledEfficiency(), msg->getTickCount());
		}
		break;

		case CM_addResource:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *>(data);
			if(msg)
			{
				owner->addResource(msg->getValue().first, static_cast<float>(msg->getValue().second));
			}
		}
		break;

		case CM_resourceAdd:
		{
			if (owner->isAuthoritative())
			{
				const MessageQueueResourceAdd *msg=
					dynamic_cast<const MessageQueueResourceAdd *>(data);
				NOT_NULL(msg);
				
				owner->addResource(msg->getResourceId(),static_cast<float>(msg->getAmount()));
			}
		}
		break;

		default:
			InstallationController::handleMessage(message, value, data, flags);
			break;
	}
}

//-----------------------------------------------------------------------

