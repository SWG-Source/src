//======================================================================
//
// ShipComponentDataBridge.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataBridge.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataBridgeNamespace
{
}

using namespace ShipComponentDataBridgeNamespace;

//----------------------------------------------------------------------

ShipComponentDataBridge::ShipComponentDataBridge (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData            (shipComponentDescriptor)
{
}

//----------------------------------------------------------------------

ShipComponentDataBridge::~ShipComponentDataBridge ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataBridge::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataBridge::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);
}

//----------------------------------------------------------------------

bool ShipComponentDataBridge::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	DynamicVariableList const &  objvars = component.getObjVars ();
	UNREF(objvars);

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataBridge::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);
}

//----------------------------------------------------------------------

void ShipComponentDataBridge::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
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

void ShipComponentDataBridge::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);
}

//======================================================================
