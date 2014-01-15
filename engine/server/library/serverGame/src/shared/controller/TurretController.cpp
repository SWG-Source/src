//TurretController.cpp

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/TurretController.h"

#include "serverGame/WeaponObject.h"

//-----------------------------------------------------------------------

TurretController::TurretController(WeaponObject * newOwner) :
		WeaponController(newOwner)
{
}

//-----------------------------------------------------------------------

TurretController::~TurretController()
{
}

//-----------------------------------------------------------------------
