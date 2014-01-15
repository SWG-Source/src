// GroundVehicleController.cpp
// copyright 2000 Verant Interactive


//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "serverGame/GroundVehicleController.h"
#include "serverGame/VehicleObject.h"

//-----------------------------------------------------------------------

GroundVehicleController::GroundVehicleController(VehicleObject * newOwner) :
	VehicleController(newOwner),
	yawPitchRollRates(convertDegreesToRadians(60.0f),
					  convertDegreesToRadians(60.0f), 
					  convertDegreesToRadians(60.0f)
					  ),
	maxForwardVelocity(60)
{
}

//-----------------------------------------------------------------------

GroundVehicleController::~GroundVehicleController()
{
}

//-----------------------------------------------------------------------

