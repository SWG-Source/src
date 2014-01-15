//======================================================================
//
// ServerSharedObjectTemplateInterface.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerSharedObjectTemplateInterface.h"

#include "serverGame/ServerObject.h"
#include "serverGame/ShipObject.h"

//======================================================================

ServerSharedObjectTemplateInterface::ServerSharedObjectTemplateInterface()
: SharedObjectTemplateInterface()
{
}

//----------------------------------------------------------------------

ServerSharedObjectTemplateInterface::~ServerSharedObjectTemplateInterface()
{
}

//----------------------------------------------------------------------

SharedObjectTemplate const * ServerSharedObjectTemplateInterface::getSharedTemplate(Object const * const object) const
{
	ServerObject const * const serverObject = object->asServerObject();
	return static_cast<SharedObjectTemplate const *>(serverObject->getSharedTemplate());
}

//----------------------------------------------------------------------

bool ServerSharedObjectTemplateInterface::isShipObject(Object const * const object) const
{
	ServerObject const * const serverObject = (object != 0) ? object->asServerObject() : 0;
	ShipObject const * const shipObject = (serverObject != 0) ? serverObject->asShipObject() : 0;
	return shipObject != 0;
}

//----------------------------------------------------------------------

uint32 ServerSharedObjectTemplateInterface::getShipChassisType(Object const * const object) const
{
	ServerObject const * const serverObject = (object != 0) ? object->asServerObject() : 0;
	ShipObject const * const shipObject = (serverObject != 0) ? serverObject->asShipObject() : 0;
	return (shipObject != 0) ? shipObject->getChassisType() : 0;
}

//----------------------------------------------------------------------

uint32 ServerSharedObjectTemplateInterface::getShipComponentCrc(Object const * const object, int chassisSlot) const
{
	ServerObject const * const serverObject = (object != 0) ? object->asServerObject() : 0;
	ShipObject const * const shipObject = (serverObject != 0) ? serverObject->asShipObject() : 0;
	return (shipObject != 0) ? shipObject->getComponentCrc(chassisSlot) : 0;
}

//======================================================================
