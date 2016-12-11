//======================================================================
//
// ShipComponentDataTargetingStation.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataTargetingStation.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataTargetingStationNamespace
{
}

using namespace ShipComponentDataTargetingStationNamespace;

//----------------------------------------------------------------------

ShipComponentDataTargetingStation::ShipComponentDataTargetingStation (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData            (shipComponentDescriptor)
{
}

//----------------------------------------------------------------------

ShipComponentDataTargetingStation::~ShipComponentDataTargetingStation ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataTargetingStation::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataTargetingStation::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);
}

//----------------------------------------------------------------------

bool ShipComponentDataTargetingStation::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	DynamicVariableList const &  objvars = component.getObjVars ();
	UNREF(objvars);

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataTargetingStation::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);
}

//----------------------------------------------------------------------

void ShipComponentDataTargetingStation::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);
	UNREF(buf_size);

	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	UNREF(nPad);

	result += Unicode::narrowToWide (buf);

}

//----------------------------------------------------------------------

void ShipComponentDataTargetingStation::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);
}

//======================================================================
