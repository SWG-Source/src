//======================================================================
//
// ServerShipComponentData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerShipComponentData.h"

#include "serverGame/ShipObject.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ServerShipComponentDataNamespace
{
	namespace Objvars
	{
		std::string const armorHitpointsCurrent        = "ship_comp.armor_hitpoints_current";
		std::string const armorHitpointsMaximum        = "ship_comp.armor_hitpoints_maximum";
		std::string const energyMaintenanceRequirement = "ship_comp.energy_maintenance_requirement";
		std::string const mass                         = "ship_comp.mass";
		std::string const hitpointsCurrent             = "ship_comp.hitpoints_current";
		std::string const hitpointsMaximum             = "ship_comp.hitpoints_maximum";
		std::string const flags                        = "ship_comp.flags";
	}
}

using namespace ServerShipComponentDataNamespace;

//----------------------------------------------------------------------

ServerShipComponentData::ServerShipComponentData (ShipComponentDescriptor const & shipComponentDescriptor) :
ShipComponentData      (shipComponentDescriptor)
{
}

//----------------------------------------------------------------------

ServerShipComponentData::~ServerShipComponentData ()
{
}

//----------------------------------------------------------------------

bool ServerShipComponentData::readDataFromShip      (int const chassisSlot, ShipObject const & ship)
{
	m_armorHitpointsMaximum        = ship.getComponentArmorHitpointsMaximum        (chassisSlot);
	m_armorHitpointsCurrent        = ship.getComponentArmorHitpointsCurrent        (chassisSlot);
	m_efficiencyGeneral            = ship.getComponentEfficiencyGeneral            (chassisSlot);
	m_efficiencyEnergy             = ship.getComponentEfficiencyEnergy             (chassisSlot);
	m_energyMaintenanceRequirement = ship.getComponentEnergyMaintenanceRequirement (chassisSlot);
	m_mass                         = ship.getComponentMass                         (chassisSlot);
	m_hitpointsCurrent             = ship.getComponentHitpointsCurrent             (chassisSlot);
	m_hitpointsMaximum             = ship.getComponentHitpointsMaximum             (chassisSlot);
	m_flags                        = ship.getComponentFlags                        (chassisSlot);

	m_name = ship.getComponentName(chassisSlot);
	m_creator = ship.getComponentCreator(chassisSlot);

	return true;
}

//----------------------------------------------------------------------

void ServerShipComponentData::writeDataToShip       (int const chassisSlot, ShipObject & ship) const
{
	if (!m_descriptor)
		return;

	ship.setComponentCrc(chassisSlot, m_descriptor->getCrc());

	IGNORE_RETURN(ship.setComponentArmorHitpointsMaximum(chassisSlot, m_armorHitpointsMaximum));
	IGNORE_RETURN(ship.setComponentArmorHitpointsCurrent(chassisSlot, m_armorHitpointsCurrent));
	IGNORE_RETURN(ship.setComponentEfficiencyGeneral(chassisSlot, m_efficiencyGeneral));
	IGNORE_RETURN(ship.setComponentEfficiencyEnergy(chassisSlot, m_efficiencyEnergy));
	IGNORE_RETURN(ship.setComponentEnergyMaintenanceRequirement(chassisSlot, m_energyMaintenanceRequirement));
	IGNORE_RETURN(ship.setComponentMass(chassisSlot, m_mass));
	IGNORE_RETURN(ship.setComponentHitpointsMaximum(chassisSlot, m_hitpointsMaximum));
	IGNORE_RETURN(ship.setComponentHitpointsCurrent(chassisSlot, m_hitpointsCurrent));
	IGNORE_RETURN(ship.setComponentFlags(chassisSlot, m_flags));
	IGNORE_RETURN(ship.setComponentName(chassisSlot, m_name));
	IGNORE_RETURN(ship.setComponentCreator(chassisSlot, m_creator));
}

//----------------------------------------------------------------------

bool ServerShipComponentData::readDataFromComponent (TangibleObject const & component)
{
	if (component.getObjectTemplate () == nullptr)
	{
		WARNING (true, ("ShipComponentData [%s] has no object template", component.getNetworkId ().getValueString ().c_str ()));
		return false;
	}

	uint32 const objectTemplateCrc = component.getObjectTemplate ()->getCrcName ().getCrc ();
	m_descriptor = ShipComponentDescriptor::findShipComponentDescriptorByObjectTemplate (objectTemplateCrc);

	if (!m_descriptor)
	{
		WARNING (true, ("ShipComponentData [%s]  does not map to a component", component.getNetworkId ().getValueString ().c_str ()));
		return false;
	}

	DynamicVariableList const &  objvars = component.getObjVars ();

	if (!objvars.getItem (Objvars::armorHitpointsCurrent, m_armorHitpointsCurrent))
		WARNING (true, ("ShipComponentData [%s] has no armorHitpointsCurrent [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::armorHitpointsCurrent.c_str ()));

	if (!objvars.getItem (Objvars::armorHitpointsMaximum, m_armorHitpointsMaximum))
		WARNING (true, ("ShipComponentData [%s] has no armorHitpointsMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::armorHitpointsMaximum.c_str ()));

	if (!objvars.getItem (Objvars::energyMaintenanceRequirement, m_energyMaintenanceRequirement))
		WARNING (true, ("ShipComponentData [%s] has no energyMaintenanceRequirement [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::energyMaintenanceRequirement.c_str ()));

	if (!objvars.getItem (Objvars::mass, m_mass))
		WARNING (true, ("ShipComponentData [%s] has no mass [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::mass.c_str ()));

	if (!objvars.getItem (Objvars::hitpointsCurrent, m_hitpointsCurrent))
		WARNING (true, ("ShipComponentData [%s] has no hitpointsCurrent [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::hitpointsCurrent.c_str ()));

	if (!objvars.getItem (Objvars::hitpointsMaximum, m_hitpointsMaximum))
		WARNING (true, ("ShipComponentData [%s] has no hitpointsMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::hitpointsMaximum.c_str ()));

	if (!objvars.getItem (Objvars::flags, m_flags))
		WARNING (true, ("ShipComponentData [%s] has no flags [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::flags.c_str ()));

	m_name = component.getAssignedObjectName();
	m_creator = component.getCreatorId();

	return true;
}

//----------------------------------------------------------------------

void ServerShipComponentData::writeDataToComponent  (TangibleObject & component) const
{
	if (m_descriptor == nullptr)
	{
		WARNING (true, ("ShipComponentData::writeDataToComponent [%s] nullptr descriptor", component.getNetworkId ().getValueString ().c_str ()));
		return;
	}

	if (component.getObjectTemplate () == nullptr)
	{
		WARNING (true, ("ShipComponentData::writeDataToComponent [%s] has no object template", component.getNetworkId ().getValueString ().c_str ()));
		return;
	}

	if (m_descriptor->getObjectTemplateCrc () != component.getObjectTemplate ()->getCrcName ().getCrc ())
	{
		WARNING (true, ("ShipComponentData::writeDataToComponent [%s] writeDataToComponent failed... wrong object template", component.getNetworkId ().getValueString ().c_str ()));
		return;
	}

	ShipComponentDescriptor const * const shipComponentDescriptor = 
		ShipComponentDescriptor::findShipComponentDescriptorByObjectTemplate (component.getObjectTemplate ()->getCrcName ().getCrc ());

	if (shipComponentDescriptor != m_descriptor)
	{
		WARNING (true, ("ShipComponentData::writeDataToComponent [%s] does not match ship component descriptor", component.getNetworkId ().getValueString ().c_str ()));
		return;
	}

	IGNORE_RETURN(component.setObjVarItem(Objvars::armorHitpointsCurrent, m_armorHitpointsCurrent));
	IGNORE_RETURN(component.setObjVarItem(Objvars::armorHitpointsMaximum, m_armorHitpointsMaximum));
	IGNORE_RETURN(component.setObjVarItem(Objvars::energyMaintenanceRequirement, m_energyMaintenanceRequirement));
	IGNORE_RETURN(component.setObjVarItem(Objvars::mass, m_mass));
	IGNORE_RETURN(component.setObjVarItem(Objvars::hitpointsCurrent, m_hitpointsCurrent));
	IGNORE_RETURN(component.setObjVarItem(Objvars::hitpointsMaximum, m_hitpointsMaximum));	
	IGNORE_RETURN(component.setObjVarItem(Objvars::flags, m_flags));

	component.setObjectName(m_name);
	component.setCreatorId(m_creator);
}

//----------------------------------------------------------------------

void ServerShipComponentData::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	if (m_descriptor == nullptr)
		return;

	char buf [2048];
	const size_t buf_size = sizeof (buf);

	std::string const & nPad = Unicode::wideToNarrow (padding);

	float oo_efficiencyEnergy = m_efficiencyEnergy;
	if (oo_efficiencyEnergy > 0.0f)
		oo_efficiencyEnergy = RECIP (oo_efficiencyEnergy);

	snprintf (buf, buf_size,
		"%s\\#pcontrast2 Descriptor:\\#.\n"
		"%s    Name: %s (%s), (%s) compat:[%s]\n"
		"%sArmor: %f/%f, Hitpoints: %f/%f\n"
		"%sEfficiency: gen: %f, energy: %f\n"
		"%sEnergy Req: %f (%f)\n"
		"%sMass: %f\n"
		"%sFlags: %d\n"
		"%s----------\n",
		nPad.c_str (),
		nPad.c_str (), m_descriptor->getName ().getString (), Unicode::wideToNarrow(m_name).c_str(),
		ShipComponentType::getNameFromType (m_descriptor->getComponentType ()).c_str (),
		m_descriptor->getCompatibility ().getString (),
		nPad.c_str (), m_armorHitpointsCurrent, m_armorHitpointsMaximum,
		m_hitpointsCurrent, m_hitpointsMaximum,
		nPad.c_str (), m_efficiencyGeneral, m_efficiencyEnergy,
		nPad.c_str (), m_energyMaintenanceRequirement, m_energyMaintenanceRequirement * oo_efficiencyEnergy,
		nPad.c_str (), m_mass,
		nPad.c_str (), m_flags,
		nPad.c_str ());

	result += Unicode::narrowToWide (buf);
}

//----------------------------------------------------------------------

void ServerShipComponentData::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	//everything is handled in the shared, base class
	ShipComponentData::getAttributes(data);
}

//======================================================================
