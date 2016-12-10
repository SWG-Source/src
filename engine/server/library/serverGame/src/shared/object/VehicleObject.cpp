//========================================================================
//
// VehicleObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/VehicleObject.h"

#include "serverGame/GameServer.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "serverGame/ServerVehicleObjectTemplate.h"
#include "serverGame/VehicleController.h"


//-----------------------------------------------------------------------
VehicleObject::VehicleObject(const ServerVehicleObjectTemplate* newTemplate) : 
	TangibleObject(newTemplate)
{
	addMembersToPackages();
}

//-----------------------------------------------------------------------

VehicleObject::~VehicleObject()
{
}
 
//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* VehicleObject::createDefaultController(void)
{
	Controller* controller = new VehicleController(this);

	setController(controller);
	return controller;
}	// VehicleObject::createDefaultController

//-----------------------------------------------------------------------

void VehicleObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	TangibleObject::getAttributes(data);
}

// ----------------------------------------------------------------------
