//========================================================================
//
// InstallationController.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/InstallationController.h"

#include "serverGame/InstallationObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "swgServerNetworkMessages/MessageQueueActivateInstallation.h"
#include "swgServerNetworkMessages/MessageQueueDeactivateInstallation.h"
#include "swgServerNetworkMessages/MessageQueueInstallationHarvest.h"

//-----------------------------------------------------------------------

InstallationController::InstallationController(InstallationObject * newOwner) :
	TangibleController(newOwner)
{
}

//-----------------------------------------------------------------------

InstallationController::~InstallationController()
{
}

//-----------------------------------------------------------------------

void InstallationController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	InstallationObject* owner = dynamic_cast<InstallationObject *>(getOwner());
	NOT_NULL(owner);

	switch (message)
	{
		case CM_activateInstallation:
		{
			const MessageQueueActivateInstallation *msg=
				dynamic_cast<const MessageQueueActivateInstallation *>(data);
			NOT_NULL(msg);
			DEBUG_REPORT_LOG(true,("Got MessageQueueActivateInstallation\n"));

			owner->activate(NetworkId::cms_invalid);
		}
		break;
		
		case CM_installationHarvest:
		{
			const MessageQueueInstallationHarvest *msg=
				dynamic_cast<const MessageQueueInstallationHarvest *>(data);
			NOT_NULL(msg);
			
			owner->harvest();
		}
		break;

		case CM_deactivateInstallation:
		{
			const MessageQueueDeactivateInstallation *msg=
				dynamic_cast<const MessageQueueDeactivateInstallation *>(data);
			NOT_NULL(msg);

			owner->deactivate();
		}
		break;
		
		default:
			TangibleController::handleMessage(message, value, data, flags);
			break;
	}
}

//-----------------------------------------------------------------------
