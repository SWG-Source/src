// ManufactureInstallationController.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/ManufactureInstallationController.h"
#include "serverGame/ManufactureInstallationObject.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/NetworkIdManager.h"

//-----------------------------------------------------------------------

ManufactureInstallationController::ManufactureInstallationController(ManufactureInstallationObject * owner) :
InstallationController(owner)
{
}

//-----------------------------------------------------------------------

ManufactureInstallationController::~ManufactureInstallationController()
{
}

//-----------------------------------------------------------------------

void ManufactureInstallationController::handleMessage(int message, float value, const MessageQueue::Data * data, uint32 flags)
{
	ManufactureInstallationObject * owner = safe_cast<ManufactureInstallationObject *>(getOwner());
	NOT_NULL(owner);

	switch(message)
	{
	case CM_addSchematic:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > *>(data);
			if(msg)
			{
				ManufactureSchematicObject * schematic = safe_cast<ManufactureSchematicObject *>(NetworkIdManager::getObjectById(msg->getValue().first));
				if(schematic)
				{
					ServerObject * transferer = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(msg->getValue().second));
					owner->addSchematic(*schematic, transferer);
				}
			}
		}
		break;
	case CM_manufactureInstallationCreateObject:
		{
			owner->createObject();
		}
		break;
	default:
		InstallationController::handleMessage(message, value, data, flags);
		break;
	}
}

//-----------------------------------------------------------------------

