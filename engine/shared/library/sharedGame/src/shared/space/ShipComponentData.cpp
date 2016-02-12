//======================================================================
//
// ShipComponentData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipComponentData.h"

#include "UnicodeUtils.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentFlags.h"
#include "sharedObject/ObjectTemplate.h"

//======================================================================

namespace ShipComponentDataNamespace
{
}

using namespace ShipComponentDataNamespace;

//----------------------------------------------------------------------

ShipComponentData::ShipComponentData (ShipComponentDescriptor const & shipComponentDescriptor) :
m_descriptor                   (&shipComponentDescriptor),
cm_slash                       (Unicode::narrowToWide("/")),
cm_shipComponentCategory       ("ship_component."),
m_armorHitpointsCurrent        (50.0f),
m_armorHitpointsMaximum        (50.0f),
m_efficiencyGeneral            (1.0f),
m_efficiencyEnergy             (1.0f),
m_energyMaintenanceRequirement (0.0f),
m_mass                         (0.0f),
m_hitpointsCurrent             (100.0f),
m_hitpointsMaximum             (100.0f),
m_flags                        (ShipComponentFlags::F_lowPower),
m_name                         ()
{
}

//----------------------------------------------------------------------

ShipComponentData::~ShipComponentData ()
{
	m_descriptor = 0;
}

//----------------------------------------------------------------------

void ShipComponentData::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
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

void ShipComponentData::getAttributes(AttributeVector & data) const
{
	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	//armor
	snprintf(buffer, buffer_size, "%.1f", m_armorHitpointsCurrent);
	attrib = Unicode::narrowToWide(buffer);
	attrib += cm_slash;
	snprintf(buffer, buffer_size, "%.1f", m_armorHitpointsMaximum);
	attrib += Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_armor, attrib));

	//hitpoints
	snprintf(buffer, buffer_size, "%.1f", m_hitpointsCurrent);
	attrib = Unicode::narrowToWide(buffer);
	attrib += cm_slash;
	snprintf(buffer, buffer_size, "%.1f", m_hitpointsMaximum);
	attrib += Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_hitpoints, attrib));

	//general efficiency
	if(m_efficiencyGeneral < 1.0f)
	{
		snprintf(buffer, buffer_size, "%.1f", m_efficiencyGeneral);
		attrib = Unicode::narrowToWide(buffer);
		data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_efficiency_general, attrib));
	}

	//energy efficiency
	if(m_efficiencyEnergy < 1.0f)
	{
		snprintf(buffer, buffer_size, "%.1f", m_efficiencyEnergy);
		attrib = Unicode::narrowToWide(buffer);
		data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_efficiency_energy, attrib));
	}

	//energy maintenance requirement
	if(m_energyMaintenanceRequirement > 0.0f)
	{
		snprintf(buffer, buffer_size, "%.1f", m_energyMaintenanceRequirement);
		attrib = Unicode::narrowToWide(buffer);
		data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_energy_required, attrib));
	}

	//mass
	snprintf(buffer, buffer_size, "%.1f", m_mass);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_mass, attrib));
}
//======================================================================
