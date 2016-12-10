//======================================================================
//
// ShipComponentDataModification.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataModification.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataModificationNamespace
{
	namespace Objvars
	{
	}
}

using namespace ShipComponentDataModificationNamespace;

//----------------------------------------------------------------------

ShipComponentDataModification::ShipComponentDataModification (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData(shipComponentDescriptor)
{
}

//----------------------------------------------------------------------

ShipComponentDataModification::~ShipComponentDataModification ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataModification::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataModification::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);
}

//----------------------------------------------------------------------

bool ShipComponentDataModification::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataModification::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);
}


//----------------------------------------------------------------------

void ShipComponentDataModification::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);
	result += padding + Unicode::narrowToWide ("Modifications have no special properties.\n");
}

//----------------------------------------------------------------------

void ShipComponentDataModification::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);
}

//======================================================================
