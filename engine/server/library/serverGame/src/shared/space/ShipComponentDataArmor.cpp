//======================================================================
//
// ShipComponentDataArmor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataArmor.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataArmorNamespace
{
	namespace Objvars
	{
	}
}

using namespace ShipComponentDataArmorNamespace;

//----------------------------------------------------------------------

ShipComponentDataArmor::ShipComponentDataArmor (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData(shipComponentDescriptor)
{
}

//----------------------------------------------------------------------

ShipComponentDataArmor::~ShipComponentDataArmor ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataArmor::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataArmor::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);
}

//----------------------------------------------------------------------

bool ShipComponentDataArmor::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataArmor::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);
}


//----------------------------------------------------------------------

void ShipComponentDataArmor::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);
	result += padding + Unicode::narrowToWide ("Armor has no special properties.\n");
}

//----------------------------------------------------------------------

void ShipComponentDataArmor::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);
}

//======================================================================
