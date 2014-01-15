// ======================================================================
//
// ResourceContainerController.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ResourceContainerController.h"

#include "serverGame/ResourceContainerObject.h"

//-----------------------------------------------------------------------

ResourceContainerController::ResourceContainerController(ResourceContainerObject * newOwner) :
		TangibleController(newOwner)
{
}

//-----------------------------------------------------------------------

ResourceContainerController::~ResourceContainerController()
{
}

//-----------------------------------------------------------------------
