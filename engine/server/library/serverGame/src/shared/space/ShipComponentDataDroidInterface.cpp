//======================================================================
//
// ShipComponentDataDroidInterface.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataDroidInterface.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataDroidInterfaceNamespace
{
	namespace Objvars
	{
		std::string const droidInterfaceCommandSpeed        = "ship_comp.droid_interface.command_speed";
	}
}

using namespace ShipComponentDataDroidInterfaceNamespace;

//----------------------------------------------------------------------

ShipComponentDataDroidInterface::ShipComponentDataDroidInterface (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData      (shipComponentDescriptor),
m_droidInterfaceCommandSpeed (0.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataDroidInterface::~ShipComponentDataDroidInterface ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataDroidInterface::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_droidInterfaceCommandSpeed = ship.getDroidInterfaceCommandSpeed ();
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataDroidInterface::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);

	ship.setDroidInterfaceCommandSpeed (m_droidInterfaceCommandSpeed);
}

//----------------------------------------------------------------------

bool ShipComponentDataDroidInterface::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	DynamicVariableList const &  objvars = component.getObjVars ();

	if (!objvars.getItem (Objvars::droidInterfaceCommandSpeed, m_droidInterfaceCommandSpeed))
		WARNING (true, ("ShipComponentDataDroidInterface [%s] has no droidInterfaceCommandSpeed [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::droidInterfaceCommandSpeed.c_str ()));

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataDroidInterface::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);

	component.setObjVarItem (Objvars::droidInterfaceCommandSpeed, m_droidInterfaceCommandSpeed);
}


//----------------------------------------------------------------------

void ShipComponentDataDroidInterface::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);

	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";

	snprintf (buf, buf_size,
		"%sDroidInterfaceCommandSpeed: %f\n",
		nPad.c_str (), m_droidInterfaceCommandSpeed);

	result += Unicode::narrowToWide (buf);

}

//----------------------------------------------------------------------

void ShipComponentDataDroidInterface::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", m_droidInterfaceCommandSpeed);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_droidinterface_speed, attrib));
}

//======================================================================
