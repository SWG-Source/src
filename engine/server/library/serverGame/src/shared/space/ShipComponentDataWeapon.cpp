//======================================================================
//
// ShipComponentDataWeapon.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataWeapon.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataWeaponNamespace
{
	namespace Objvars
	{
		std::string const weaponDamageMaximum        = "ship_comp.weapon.damage_maximum";
		std::string const weaponDamageMinimum        = "ship_comp.weapon.damage_minimum";
		std::string const weaponEffectivenessShields = "ship_comp.weapon.effectiveness_shields";
		std::string const weaponEffectivenessArmor   = "ship_comp.weapon.effectiveness_armor";
		std::string const weaponEnergyPerShot        = "ship_comp.weapon.energy_per_shot";
		std::string const weaponRefireRate           = "ship_comp.weapon.refire_rate";
		std::string const weaponAmmoCurrent          = "ship_comp.weapon.ammo_current";
		std::string const weaponAmmoMaximum          = "ship_comp.weapon.ammo_maximum";
		std::string const weaponAmmoType             = "ship_comp.weapon.ammo_type";
	}
}

using namespace ShipComponentDataWeaponNamespace;

//----------------------------------------------------------------------

ShipComponentDataWeapon::ShipComponentDataWeapon (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData      (shipComponentDescriptor),
m_weaponDamageMaximum        (10.0f),
m_weaponDamageMinimum        (1.0f),
m_weaponEffectivenessShields (0.5f),
m_weaponEffectivenessArmor   (0.5f),
m_weaponEnergyPerShot        (10.0f),
m_weaponRefireRate           (0.5f),
m_weaponEfficiencyRefireRate (1.0f),
m_weaponAmmoCurrent          (0),
m_weaponAmmoMaximum          (0),
m_weaponAmmoType             (0)
{
}

//----------------------------------------------------------------------

ShipComponentDataWeapon::~ShipComponentDataWeapon ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataWeapon::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_weaponDamageMaximum         = ship.getWeaponDamageMaximum        (chassisSlot);
	m_weaponDamageMinimum         = ship.getWeaponDamageMinimum        (chassisSlot);
	m_weaponEffectivenessShields  = ship.getWeaponEffectivenessShields (chassisSlot);
	m_weaponEffectivenessArmor    = ship.getWeaponEffectivenessArmor   (chassisSlot);
	m_weaponEnergyPerShot         = ship.getWeaponEnergyPerShot        (chassisSlot);
	m_weaponRefireRate            = ship.getWeaponRefireRate           (chassisSlot);
	m_weaponEfficiencyRefireRate  = ship.getWeaponEfficiencyRefireRate(chassisSlot);
	m_weaponAmmoCurrent           = ship.getWeaponAmmoCurrent          (chassisSlot);
	m_weaponAmmoMaximum           = ship.getWeaponAmmoMaximum          (chassisSlot);
	m_weaponAmmoType              = ship.getWeaponAmmoType             (chassisSlot);
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataWeapon::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);

	IGNORE_RETURN(ship.setWeaponDamageMaximum        (chassisSlot, m_weaponDamageMaximum));
	IGNORE_RETURN(ship.setWeaponDamageMinimum        (chassisSlot, m_weaponDamageMinimum));
	IGNORE_RETURN(ship.setWeaponEffectivenessShields (chassisSlot, m_weaponEffectivenessShields));
	IGNORE_RETURN(ship.setWeaponEffectivenessArmor   (chassisSlot, m_weaponEffectivenessArmor));
	IGNORE_RETURN(ship.setWeaponEnergyPerShot        (chassisSlot, m_weaponEnergyPerShot));
	IGNORE_RETURN(ship.setWeaponRefireRate           (chassisSlot, m_weaponRefireRate));
	IGNORE_RETURN(ship.setWeaponEfficiencyRefireRate(chassisSlot, m_weaponEfficiencyRefireRate));
	IGNORE_RETURN(ship.setWeaponAmmoCurrent          (chassisSlot, m_weaponAmmoCurrent));
	IGNORE_RETURN(ship.setWeaponAmmoMaximum          (chassisSlot, m_weaponAmmoMaximum));
	IGNORE_RETURN(ship.setWeaponAmmoType             (chassisSlot, m_weaponAmmoType));
}

//----------------------------------------------------------------------

bool ShipComponentDataWeapon::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;
	
	DynamicVariableList const &  objvars = component.getObjVars ();
	
	if (!objvars.getItem (Objvars::weaponDamageMaximum, m_weaponDamageMaximum))
		DEBUG_WARNING (true, ("ShipComponentDataWeapon [%s] has no weaponDamageMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::weaponDamageMaximum.c_str ()));
	
	if (!objvars.getItem (Objvars::weaponDamageMinimum, m_weaponDamageMinimum))
		DEBUG_WARNING (true, ("ShipComponentDataWeapon [%s] has no weaponDamageMinimum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::weaponDamageMinimum.c_str ()));
	
	if (!objvars.getItem (Objvars::weaponEffectivenessShields, m_weaponEffectivenessShields))
		DEBUG_WARNING (true, ("ShipComponentDataWeapon [%s] has no weaponEffectivenessShields [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::weaponEffectivenessShields.c_str ()));
	
	if (!objvars.getItem (Objvars::weaponEffectivenessArmor, m_weaponEffectivenessArmor))
		DEBUG_WARNING (true, ("ShipComponentDataWeapon [%s] has no weaponEffectivenessArmor [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::weaponEffectivenessArmor.c_str ()));
	
	if (!objvars.getItem (Objvars::weaponEnergyPerShot, m_weaponEnergyPerShot))
		DEBUG_WARNING (true, ("ShipComponentDataWeapon [%s] has no weaponEnergyPerShot [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::weaponEnergyPerShot.c_str ()));
	
	if (!objvars.getItem (Objvars::weaponRefireRate, m_weaponRefireRate))
		DEBUG_WARNING (true, ("ShipComponentDataWeapon [%s] has no weaponRefireRate [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::weaponRefireRate.c_str ()));

	if (!objvars.getItem (Objvars::weaponAmmoCurrent, m_weaponAmmoCurrent))
		DEBUG_WARNING (true, ("ShipComponentDataWeapon [%s] has no m_weaponAmmoCurrent [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::weaponAmmoCurrent.c_str ()));

	if (!objvars.getItem (Objvars::weaponAmmoMaximum, m_weaponAmmoMaximum))
		DEBUG_WARNING (true, ("ShipComponentDataWeapon [%s] has no m_weaponAmmoMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::weaponAmmoMaximum.c_str ()));

	int ammoType = 0;
	if (!objvars.getItem (Objvars::weaponAmmoType, ammoType))
		DEBUG_WARNING (true, ("ShipComponentDataWeapon [%s] has no m_weaponAmmoType [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::weaponAmmoType.c_str ()));
	else
		m_weaponAmmoType = static_cast<uint32>(ammoType);
	
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataWeapon::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);

	IGNORE_RETURN (component.setObjVarItem (Objvars::weaponDamageMaximum,        m_weaponDamageMaximum));
	IGNORE_RETURN (component.setObjVarItem (Objvars::weaponDamageMinimum,        m_weaponDamageMinimum));
	IGNORE_RETURN (component.setObjVarItem (Objvars::weaponEffectivenessShields, m_weaponEffectivenessShields));
	IGNORE_RETURN (component.setObjVarItem (Objvars::weaponEffectivenessArmor,   m_weaponEffectivenessArmor));
	IGNORE_RETURN (component.setObjVarItem (Objvars::weaponEnergyPerShot,        m_weaponEnergyPerShot));
	IGNORE_RETURN (component.setObjVarItem (Objvars::weaponRefireRate,           m_weaponRefireRate));
	IGNORE_RETURN (component.setObjVarItem (Objvars::weaponAmmoCurrent,          m_weaponAmmoCurrent));
	IGNORE_RETURN (component.setObjVarItem (Objvars::weaponAmmoMaximum,          m_weaponAmmoMaximum));
	IGNORE_RETURN (component.setObjVarItem (Objvars::weaponAmmoType,             static_cast<int>(m_weaponAmmoType)));
}


//----------------------------------------------------------------------

void ShipComponentDataWeapon::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);

	char buf [1024];
	const size_t buf_size = sizeof (buf);

	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";

	snprintf (buf, buf_size,
		"%sDamage:                %f-%f\n"
		"%sEffectivenessShields:  %f\n"
		"%sEffectivenessArmor:    %f\n"
		"%sEnergyPerShot:         %f\n"
		"%sRefireRate:            %f @ %f\n"
		"%sAmmo:                  (%lu) %d/%d\n",
		nPad.c_str (), m_weaponDamageMaximum, m_weaponDamageMinimum,
		nPad.c_str (), m_weaponEffectivenessShields,
		nPad.c_str (), m_weaponEffectivenessArmor,
		nPad.c_str (), m_weaponEnergyPerShot,
		nPad.c_str (), m_weaponRefireRate, m_weaponEfficiencyRefireRate,
		nPad.c_str (), m_weaponAmmoType, m_weaponAmmoCurrent, m_weaponAmmoMaximum);

	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ShipComponentDataWeapon::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f-%.1f", m_weaponDamageMinimum, m_weaponDamageMaximum);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_damage, attrib));

	snprintf(buffer, buffer_size, "%.3f", m_weaponEffectivenessShields);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_effectiveness_shields, attrib));

	snprintf(buffer, buffer_size, "%.3f", m_weaponEffectivenessArmor);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_effectiveness_armor, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_weaponEnergyPerShot);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_energy_per_shot, attrib));

	snprintf(buffer, buffer_size, "%.3f", m_weaponRefireRate);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_refire_rate, attrib));

	if(m_weaponAmmoMaximum != 0)
	{
		IGNORE_RETURN(_itoa(m_weaponAmmoCurrent, buffer, 10));
		attrib = Unicode::narrowToWide(buffer);
		attrib += cm_slash;
		IGNORE_RETURN(_itoa(m_weaponAmmoMaximum, buffer, 10));
		attrib += Unicode::narrowToWide(buffer);
		data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_weapon_ammo, attrib));
	}
}

//======================================================================
