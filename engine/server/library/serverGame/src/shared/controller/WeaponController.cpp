//WeaponController.cpp

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/WeaponController.h"

#include "serverGame/WeaponObject.h"

//-----------------------------------------------------------------------

WeaponController::WeaponController(WeaponObject * newOwner) :
		TangibleController(newOwner)
{
}

//-----------------------------------------------------------------------

WeaponController::~WeaponController()
{
}

//-----------------------------------------------------------------------

