//======================================================================
//
// ShipComponentDataReactor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataReactor.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataReactorNamespace
{
	namespace Objvars
	{
		std::string const energyGenerationRate        = "ship_comp.reactor.energy_generation_rate";
	}
}

using namespace ShipComponentDataReactorNamespace;

//----------------------------------------------------------------------

ShipComponentDataReactor::ShipComponentDataReactor (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData(shipComponentDescriptor),
m_energyGenerationRate (0.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataReactor::~ShipComponentDataReactor ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataReactor::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_energyGenerationRate = ship.getReactorEnergyGenerationRate ();
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataReactor::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);
	ship.setReactorEnergyGenerationRate (m_energyGenerationRate);
}

//----------------------------------------------------------------------

bool ShipComponentDataReactor::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	DynamicVariableList const &  objvars = component.getObjVars ();

	if (!objvars.getItem (Objvars::energyGenerationRate, m_energyGenerationRate))
		WARNING (true, ("ShipComponentDataReactor [%s] has no energyGenerationRate [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::energyGenerationRate.c_str ()));

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataReactor::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);
	component.setObjVarItem (Objvars::energyGenerationRate, m_energyGenerationRate);
}


//----------------------------------------------------------------------

void ShipComponentDataReactor::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);

	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";

	snprintf (buf, buf_size,
		"%sEnergyGenerationRate: %f (%f)\n",
		nPad.c_str (), m_energyGenerationRate, m_energyGenerationRate * m_efficiencyGeneral);

	result += Unicode::narrowToWide (buf);

}

//----------------------------------------------------------------------

void ShipComponentDataReactor::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", m_energyGenerationRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_reactor_generation_rate, attrib));
}

//======================================================================
