//======================================================================
//
// ShipComponentDataCargoHold.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataCargoHold.h"

#include "UnicodeUtils.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"

//======================================================================

namespace ShipComponentDataCargoHoldNamespace
{
	namespace Objvars
	{
		std::string const cargoHoldContentsCurrent          = "ship_comp.cargo_hold.contents_current";
		std::string const cargoHoldContentsMaximum          = "ship_comp.cargo_hold.contents_maximum";
		std::string const cargoHoldContentsTypes            = "ship_comp.cargo_hold.contents_types";
		std::string const cargoHoldContentsAmounts          = "ship_comp.cargo_hold.contents_amounts";
	}
}

using namespace ShipComponentDataCargoHoldNamespace;

//----------------------------------------------------------------------

ShipComponentDataCargoHold::ShipComponentDataCargoHold (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData(shipComponentDescriptor),
m_cargoHoldContentsMaximum(0),
m_cargoHoldContentsCurrent(0),
m_cargoHoldContents()
{
}

//----------------------------------------------------------------------

ShipComponentDataCargoHold::~ShipComponentDataCargoHold ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataCargoHold::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_cargoHoldContentsMaximum = ship.getCargoHoldContentsMaximum();
	m_cargoHoldContentsCurrent = ship.getCargoHoldContentsCurrent();
	m_cargoHoldContents = ship.getCargoHoldContents();

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataCargoHold::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);

	ship.setCargoHoldContentsMaximum(m_cargoHoldContentsMaximum);
	ship.setCargoHoldContents(m_cargoHoldContents);
}

//----------------------------------------------------------------------

bool ShipComponentDataCargoHold::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	DynamicVariableList const &  objvars = component.getObjVars ();

	if (!objvars.getItem (Objvars::cargoHoldContentsCurrent, m_cargoHoldContentsCurrent))
		WARNING (true, ("ShipComponentDataCargoHold [%s] has no cargoHoldContentsCurrent [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::cargoHoldContentsCurrent.c_str ()));

	if (!objvars.getItem (Objvars::cargoHoldContentsMaximum, m_cargoHoldContentsMaximum))
		WARNING (true, ("ShipComponentDataCargoHold [%s] has no cargoHoldContentsMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::cargoHoldContentsMaximum.c_str ()));

	//-- read contents
	{
		std::vector<NetworkId> types;
		
		if (objvars.getItem(Objvars::cargoHoldContentsTypes, types))
		{
			std::vector<int> amounts;
			if (!objvars.getItem(Objvars::cargoHoldContentsAmounts, amounts))
				WARNING (true, ("ShipComponentDataCargoHold [%s] has no cargoHoldContentsAmounts [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::cargoHoldContentsAmounts.c_str ()));
			else if (amounts.size() != types.size())
				WARNING (true, ("ShipComponentDataCargoHold [%s] cargoHoldContentsAmounts/Types sizes mismatched", component.getNetworkId ().getValueString ().c_str ()));
			else
			{
				m_cargoHoldContents.clear();
				
				int const size = types.size();
				for (int i = 0; i < size; ++i)
				{
					m_cargoHoldContents[types[i]] = amounts[i];
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataCargoHold::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);

	IGNORE_RETURN(component.setObjVarItem (Objvars::cargoHoldContentsCurrent,         m_cargoHoldContentsCurrent));
	IGNORE_RETURN(component.setObjVarItem (Objvars::cargoHoldContentsMaximum,         m_cargoHoldContentsMaximum));

	//-- write contents
	{
		std::vector<NetworkId> types;
		std::vector<int> amounts;

		types.reserve(m_cargoHoldContents.size());
		amounts.reserve(m_cargoHoldContents.size());

		for (ShipObject::NetworkIdIntMap::const_iterator it = m_cargoHoldContents.begin(); it != m_cargoHoldContents.end(); ++it)
		{
			NetworkId const & id = (*it).first;
			int const amount = (*it).second;
			types.push_back(id);
			amounts.push_back(amount);
		}

		component.setObjVarItem(Objvars::cargoHoldContentsTypes, types);
		component.setObjVarItem(Objvars::cargoHoldContentsAmounts, amounts);
	}
}

//----------------------------------------------------------------------

void ShipComponentDataCargoHold::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);
	
	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	
	snprintf (buf, buf_size,
		"%scontents: %d/%d\n",
		nPad.c_str (), m_cargoHoldContentsCurrent, m_cargoHoldContentsMaximum);

	result += Unicode::narrowToWide (buf);

	for (ShipObject::NetworkIdIntMap::const_iterator it = m_cargoHoldContents.begin(); it != m_cargoHoldContents.end(); ++it)
	{
		NetworkId const & id = (*it).first;
		int const amount = (*it).second;

		ResourceTypeObject const * const resourceType = ServerUniverse::getInstance().getResourceTypeById(id);
		std::string const resourceName = resourceType ? resourceType->getResourceName() : "nullptr RESOURCE";

		snprintf(buf, buf_size, "%s     %15s (%s): %3d\n", nPad.c_str (), id.getValueString().c_str(), resourceName.c_str(), amount);
		result += Unicode::narrowToWide(buf);
	}
}

//----------------------------------------------------------------------

void ShipComponentDataCargoHold::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%d", m_cargoHoldContentsMaximum);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_cargo_hold_contents_max, attrib));
}

//======================================================================
