//========================================================================
//
// VehicleController.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/VehicleController.h"

#include "serverGame/GameServer.h"
#include "serverGame/VehicleObject.h"

//-----------------------------------------------------------------------

VehicleController::VehicleController(VehicleObject * newOwner) :
		TangibleController(newOwner)
{
}

//-----------------------------------------------------------------------

VehicleController::~VehicleController()
{
}

//-----------------------------------------------------------------------
