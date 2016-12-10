//======================================================================
//
// ShipComponentDataCapacitor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataCapacitor.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataCapacitorNamespace
{
	namespace Objvars
	{
		std::string const capacitorEnergyCurrent      = "ship_comp.capacitor.energy_current";
		std::string const capacitorEnergyMaximum      = "ship_comp.capacitor.energy_maximum";
		std::string const capacitorEnergyRechargeRate = "ship_comp.capacitor.energy_recharge_rate";
	}
}

using namespace ShipComponentDataCapacitorNamespace;

//----------------------------------------------------------------------

ShipComponentDataCapacitor::ShipComponentDataCapacitor (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData        (shipComponentDescriptor),
m_capacitorEnergyCurrent       (0.0f),
m_capacitorEnergyMaximum       (0.0f),
m_capacitorEnergyRechargeRate  (0.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataCapacitor::~ShipComponentDataCapacitor ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataCapacitor::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_capacitorEnergyCurrent        = ship.getCapacitorEnergyCurrent ();
	m_capacitorEnergyMaximum        = ship.getCapacitorEnergyMaximum ();
	m_capacitorEnergyRechargeRate   = ship.getCapacitorEnergyRechargeRate ();
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataCapacitor::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);

	ship.setCapacitorEnergyMaximum      (m_capacitorEnergyMaximum);
	ship.setCapacitorEnergyCurrent      (m_capacitorEnergyCurrent);
	ship.setCapacitorEnergyRechargeRate (m_capacitorEnergyRechargeRate);
}

//----------------------------------------------------------------------

bool ShipComponentDataCapacitor::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	DynamicVariableList const &  objvars = component.getObjVars ();

	if (!objvars.getItem (Objvars::capacitorEnergyCurrent, m_capacitorEnergyCurrent))
		WARNING (true, ("ShipComponentDataCapacitor [%s] has no capacitorEnergyCurrent [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::capacitorEnergyCurrent.c_str ()));
	if (!objvars.getItem (Objvars::capacitorEnergyMaximum, m_capacitorEnergyMaximum))
		WARNING (true, ("ShipComponentDataCapacitor [%s] has no capacitorEnergyMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::capacitorEnergyMaximum.c_str ()));
	if (!objvars.getItem (Objvars::capacitorEnergyRechargeRate, m_capacitorEnergyRechargeRate))
		WARNING (true, ("ShipComponentDataCapacitor [%s] has no capacitorEnergyRechargeRate [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::capacitorEnergyRechargeRate.c_str ()));

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataCapacitor::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);

	component.setObjVarItem (Objvars::capacitorEnergyCurrent, m_capacitorEnergyCurrent);
	component.setObjVarItem (Objvars::capacitorEnergyMaximum, m_capacitorEnergyMaximum);
	component.setObjVarItem (Objvars::capacitorEnergyRechargeRate, m_capacitorEnergyRechargeRate);
}


//----------------------------------------------------------------------

void ShipComponentDataCapacitor::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);
	
	char buf [1024];
	const size_t buf_size = sizeof (buf);
	
	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	
	snprintf (buf, buf_size,
		"%sCapacitorEnergyCurrent:  %f\n"
		"%sCapacitorEnergyMaximum:  %f\n"
		"%sCapacitorEnergyRechargeRate:  %f\n",
		nPad.c_str (), m_capacitorEnergyCurrent,
		nPad.c_str (), m_capacitorEnergyMaximum,
		nPad.c_str (), m_capacitorEnergyRechargeRate);
	
	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ShipComponentDataCapacitor::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", m_capacitorEnergyCurrent);
	attrib = Unicode::narrowToWide(buffer);
	attrib += cm_slash;
	snprintf(buffer, buffer_size, "%.1f", m_capacitorEnergyMaximum);
	attrib += Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_capacitor_energy, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_capacitorEnergyRechargeRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_capacitor_energy_recharge_rate, attrib));
}

//======================================================================
