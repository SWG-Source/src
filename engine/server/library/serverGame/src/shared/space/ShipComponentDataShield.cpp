//======================================================================
//
// ShipComponentDataShield.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataShield.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataShieldNamespace
{
	namespace Objvars
	{
		std::string const shieldHitpointsFrontCurrent  = "ship_comp.shield.hitpoints_front_current";
		std::string const shieldHitpointsFrontMaximum  = "ship_comp.shield.hitpoints_front_maximum";
		std::string const shieldHitpointsBackCurrent   = "ship_comp.shield.hitpoints_back_current";
		std::string const shieldHitpointsBackMaximum   = "ship_comp.shield.hitpoints_back_maximum";
		std::string const shieldRechargeRate           = "ship_comp.shield.recharge_rate";
	}
}

using namespace ShipComponentDataShieldNamespace;

//----------------------------------------------------------------------

ShipComponentDataShield::ShipComponentDataShield (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData        (shipComponentDescriptor),
m_shieldHitpointsFrontCurrent  (0.0f),
m_shieldHitpointsFrontMaximum  (0.0f),
m_shieldHitpointsBackCurrent   (0.0f),
m_shieldHitpointsBackMaximum   (0.0f),
m_shieldRechargeRate           (0.0f)
{
}

//----------------------------------------------------------------------

ShipComponentDataShield::~ShipComponentDataShield ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataShield::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_shieldHitpointsFrontCurrent  = ship.getShieldHitpointsFrontCurrent ();
	m_shieldHitpointsFrontMaximum  = ship.getShieldHitpointsFrontMaximum ();
	m_shieldHitpointsBackCurrent   = ship.getShieldHitpointsBackCurrent  ();
	m_shieldHitpointsBackMaximum   = ship.getShieldHitpointsBackMaximum  ();
	m_shieldRechargeRate           = ship.getShieldRechargeRate          ();

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataShield::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);

	ship.setShieldHitpointsFrontMaximum    (m_shieldHitpointsFrontMaximum);
	ship.setShieldHitpointsFrontCurrent    (m_shieldHitpointsFrontCurrent);
	ship.setShieldHitpointsBackMaximum     (m_shieldHitpointsBackMaximum);
	ship.setShieldHitpointsBackCurrent     (m_shieldHitpointsBackCurrent);
	ship.setShieldRechargeRate             (m_shieldRechargeRate);
}

//----------------------------------------------------------------------

bool ShipComponentDataShield::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	DynamicVariableList const &  objvars = component.getObjVars ();

	if (!objvars.getItem (Objvars::shieldHitpointsFrontCurrent, m_shieldHitpointsFrontCurrent))
		WARNING (true, ("ShipComponentDataShield [%s] has no shieldHitpointsFrontCurrent [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::shieldHitpointsFrontCurrent.c_str ()));

	if (!objvars.getItem (Objvars::shieldHitpointsFrontMaximum, m_shieldHitpointsFrontMaximum))
		WARNING (true, ("ShipComponentDataShield [%s] has no shieldHitpointsFrontMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::shieldHitpointsFrontMaximum.c_str ()));

	if (!objvars.getItem (Objvars::shieldHitpointsBackCurrent, m_shieldHitpointsBackCurrent))
		WARNING (true, ("ShipComponentDataShield [%s] has no shieldHitpointsBackCurrent [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::shieldHitpointsBackCurrent.c_str ()));

	if (!objvars.getItem (Objvars::shieldHitpointsBackMaximum, m_shieldHitpointsBackMaximum))
		WARNING (true, ("ShipComponentDataShield [%s] has no shieldHitpointsBackMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::shieldHitpointsBackMaximum.c_str ()));

	if (!objvars.getItem (Objvars::shieldRechargeRate, m_shieldRechargeRate))
		WARNING (true, ("ShipComponentDataShield [%s] has no shieldRechargeRate [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::shieldRechargeRate.c_str ()));


	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataShield::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);

	IGNORE_RETURN (component.setObjVarItem (Objvars::shieldHitpointsFrontCurrent, m_shieldHitpointsFrontCurrent));
	IGNORE_RETURN (component.setObjVarItem (Objvars::shieldHitpointsFrontMaximum, m_shieldHitpointsFrontMaximum));
	IGNORE_RETURN (component.setObjVarItem (Objvars::shieldHitpointsBackCurrent,  m_shieldHitpointsBackCurrent));
	IGNORE_RETURN (component.setObjVarItem (Objvars::shieldHitpointsBackMaximum,  m_shieldHitpointsBackMaximum));
	IGNORE_RETURN (component.setObjVarItem (Objvars::shieldRechargeRate,          m_shieldRechargeRate));
}


//----------------------------------------------------------------------

void ShipComponentDataShield::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);
	
	char buf [1024];
	const size_t buf_size = sizeof (buf);
	
	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	
	snprintf (buf, buf_size,
		"%sShieldHitpointsFront:  %f / %f\n"
		"%sShieldHitpointsBack:   %f / %f\n"
		"%sShieldRechargeRate:    %f (%f)\n",
		nPad.c_str (), m_shieldHitpointsFrontCurrent, m_shieldHitpointsFrontMaximum,
		nPad.c_str (), m_shieldHitpointsBackCurrent, m_shieldHitpointsBackMaximum,
		nPad.c_str (), m_shieldRechargeRate, m_shieldRechargeRate * m_efficiencyGeneral);
	
	result += Unicode::narrowToWide (buf);
	
}

//----------------------------------------------------------------------

void ShipComponentDataShield::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", m_shieldHitpointsFrontCurrent);
	attrib = Unicode::narrowToWide(buffer);
	attrib += cm_slash;
	snprintf(buffer, buffer_size, "%.1f", m_shieldHitpointsFrontMaximum);
	attrib += Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_shield_hitpoints_front, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_shieldHitpointsBackCurrent);
	attrib = Unicode::narrowToWide(buffer);
	attrib += cm_slash;
	snprintf(buffer, buffer_size, "%.1f", m_shieldHitpointsBackMaximum);
	attrib += Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_shield_hitpoints_back, attrib));

	snprintf(buffer, buffer_size, "%.2f", m_shieldRechargeRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_shield_recharge_rate, attrib));
}

//======================================================================
