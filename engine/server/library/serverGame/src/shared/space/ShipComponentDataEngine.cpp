//======================================================================
//
// ShipComponentDataEngine.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataEngine.h"

#include "serverGame/ShipObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ShipComponentDataEngineNamespace
{
	namespace Objvars
	{
		std::string const engineAccelerationRate         = "ship_comp.engine.acceleration_rate";
		std::string const engineDecelerationRate         = "ship_comp.engine.deceleration_rate";
		std::string const enginePitchAccelerationRate    = "ship_comp.engine.pitch_acceleration_rate";
		std::string const engineYawAccelerationRate      = "ship_comp.engine.yaw_acceleration_rate";
		std::string const engineRollAccelerationRate     = "ship_comp.engine.roll_acceleration_rate";
		std::string const enginePitchRateMaximum         = "ship_comp.engine.pitch_rate_maximum";
		std::string const engineYawRateMaximum           = "ship_comp.engine.yaw_rate_maximum";
		std::string const engineRollRateMaximum          = "ship_comp.engine.roll_rate_maximum";
		std::string const engineSpeedMaximum             = "ship_comp.engine.speed_maximum";
		std::string const engineSpeedRotationFactorMaximum = "ship_comp.engine.speed_rotation_factor_max";
		std::string const engineSpeedRotationFactorMaximum_old = "ship_comp.engine.speed_rotation_factor";
		std::string const engineSpeedRotationFactorMinimum = "ship_comp.engine.speed_rotation_factor_min";
		std::string const engineSpeedRotationFactorOptimal = "ship_comp.engine.speed_rotation_factor_optimal";
	}
}

using namespace ShipComponentDataEngineNamespace;

//----------------------------------------------------------------------

ShipComponentDataEngine::ShipComponentDataEngine (ShipComponentDescriptor const & shipComponentDescriptor) :
ServerShipComponentData      (shipComponentDescriptor),
m_engineAccelerationRate        (30.0f),
m_engineDecelerationRate        (30.0f),
m_enginePitchAccelerationRate   (PI),
m_engineYawAccelerationRate     (PI),
m_engineRollAccelerationRate    (PI),
m_enginePitchRateMaximum        (PI),
m_engineYawRateMaximum          (PI),
m_engineRollRateMaximum         (PI),
m_engineSpeedMaximum            (50.0f),
m_engineSpeedRotationFactorMaximum(1.0f),
m_engineSpeedRotationFactorMinimum(1.0f),
m_engineSpeedRotationFactorOptimal(0.5f)
{
}

//----------------------------------------------------------------------

ShipComponentDataEngine::~ShipComponentDataEngine ()
{
}

//----------------------------------------------------------------------

bool ShipComponentDataEngine::readDataFromShip      (int chassisSlot, ShipObject const & ship)
{
	if (!ServerShipComponentData::readDataFromShip (chassisSlot, ship))
		return false;

	m_engineAccelerationRate        = ship.getEngineAccelerationRate      ();
	m_engineDecelerationRate        = ship.getEngineDecelerationRate      ();
	m_enginePitchAccelerationRate   = ship.getEnginePitchAccelerationRate ();
	m_engineYawAccelerationRate     = ship.getEngineYawAccelerationRate   ();
	m_engineRollAccelerationRate    = ship.getEngineRollAccelerationRate  ();
	m_enginePitchRateMaximum        = ship.getEnginePitchRateMaximum      ();
	m_engineYawRateMaximum          = ship.getEngineYawRateMaximum        ();
	m_engineRollRateMaximum         = ship.getEngineRollRateMaximum       ();
	m_engineSpeedMaximum            = ship.getEngineSpeedMaximum          ();
	m_engineSpeedRotationFactorMaximum = ship.getEngineSpeedRotationFactorMaximum();
	m_engineSpeedRotationFactorMinimum = ship.getEngineSpeedRotationFactorMinimum();
	m_engineSpeedRotationFactorOptimal = ship.getEngineSpeedRotationFactorOptimal();


	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataEngine::writeDataToShip       (int chassisSlot, ShipObject & ship) const
{
	ServerShipComponentData::writeDataToShip (chassisSlot, ship);

	ship.setEngineAccelerationRate      (m_engineAccelerationRate);
	ship.setEngineDecelerationRate      (m_engineDecelerationRate);
	ship.setEnginePitchAccelerationRate (m_enginePitchAccelerationRate);
	ship.setEngineYawAccelerationRate   (m_engineYawAccelerationRate);
	ship.setEngineRollAccelerationRate  (m_engineRollAccelerationRate);
	ship.setEnginePitchRateMaximum      (m_enginePitchRateMaximum);
	ship.setEngineYawRateMaximum        (m_engineYawRateMaximum);
	ship.setEngineRollRateMaximum       (m_engineRollRateMaximum);
	ship.setEngineSpeedMaximum          (m_engineSpeedMaximum);
	ship.setEngineSpeedRotationFactorMaximum(m_engineSpeedRotationFactorMaximum);
	ship.setEngineSpeedRotationFactorMinimum(m_engineSpeedRotationFactorMinimum);
	ship.setEngineSpeedRotationFactorOptimal(m_engineSpeedRotationFactorOptimal);
}

//----------------------------------------------------------------------

bool ShipComponentDataEngine::readDataFromComponent (TangibleObject const & component)
{
	if (!ServerShipComponentData::readDataFromComponent (component))
		return false;

	DynamicVariableList const &  objvars = component.getObjVars ();

	if (!objvars.getItem (Objvars::engineAccelerationRate, m_engineAccelerationRate))
		DEBUG_WARNING (true, ("ShipComponentDataEngine [%s] has no engineAccelerationRate [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::engineAccelerationRate.c_str ()));

	if (!objvars.getItem (Objvars::engineDecelerationRate, m_engineDecelerationRate))
		DEBUG_WARNING (true, ("ShipComponentDataEngine [%s] has no engineDecelerationRate [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::engineDecelerationRate.c_str ()));
	
	if (!objvars.getItem (Objvars::enginePitchAccelerationRate, m_enginePitchAccelerationRate))
		DEBUG_WARNING (true, ("ShipComponentDataEngine [%s] has no enginePitchAccelerationRate [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::enginePitchAccelerationRate.c_str ()));
	else
		m_enginePitchAccelerationRate = convertDegreesToRadians(m_enginePitchAccelerationRate);
	
	if (!objvars.getItem (Objvars::engineYawAccelerationRate, m_engineYawAccelerationRate))
		DEBUG_WARNING (true, ("ShipComponentDataEngine [%s] has no engineYawAccelerationRate [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::engineYawAccelerationRate.c_str ()));
	else
		m_engineYawAccelerationRate = convertDegreesToRadians(m_engineYawAccelerationRate);
	
	if (!objvars.getItem (Objvars::engineRollAccelerationRate, m_engineRollAccelerationRate))
		DEBUG_WARNING (true, ("ShipComponentDataEngine [%s] has no engineRollAccelerationRate [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::engineRollAccelerationRate.c_str ()));
	else
		m_engineRollAccelerationRate = convertDegreesToRadians(m_engineRollAccelerationRate);
	
	if (!objvars.getItem (Objvars::enginePitchRateMaximum, m_enginePitchRateMaximum))
		DEBUG_WARNING (true, ("ShipComponentDataEngine [%s] has no enginePitchRateMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::enginePitchRateMaximum.c_str ()));
	else
		m_enginePitchRateMaximum = convertDegreesToRadians(m_enginePitchRateMaximum);
	
	if (!objvars.getItem (Objvars::engineYawRateMaximum, m_engineYawRateMaximum))
		DEBUG_WARNING (true, ("ShipComponentDataEngine [%s] has no engineYawRateMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::engineYawRateMaximum.c_str ()));
	else
		m_engineYawRateMaximum = convertDegreesToRadians(m_engineYawRateMaximum);
	
	if (!objvars.getItem (Objvars::engineRollRateMaximum, m_engineRollRateMaximum))
		DEBUG_WARNING (true, ("ShipComponentDataEngine [%s] has no engineRollRateMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::engineRollRateMaximum.c_str ()));
	else
		m_engineRollRateMaximum = convertDegreesToRadians(m_engineRollRateMaximum);
	
	if (!objvars.getItem (Objvars::engineSpeedMaximum, m_engineSpeedMaximum))
		DEBUG_WARNING (true, ("ShipComponentDataEngine [%s] has no engineSpeedMaximum [%s]", component.getNetworkId ().getValueString ().c_str (), Objvars::engineSpeedMaximum.c_str ()));

	if (!objvars.getItem(Objvars::engineSpeedRotationFactorMaximum, m_engineSpeedRotationFactorMaximum))
	{
		IGNORE_RETURN(objvars.getItem(Objvars::engineSpeedRotationFactorMaximum_old, m_engineSpeedRotationFactorMaximum));
	}

	IGNORE_RETURN(objvars.getItem(Objvars::engineSpeedRotationFactorMinimum, m_engineSpeedRotationFactorMinimum));
	IGNORE_RETURN(objvars.getItem(Objvars::engineSpeedRotationFactorOptimal, m_engineSpeedRotationFactorOptimal));
		
	return true;
}

//----------------------------------------------------------------------

void ShipComponentDataEngine::writeDataToComponent  (TangibleObject & component) const
{
	ServerShipComponentData::writeDataToComponent (component);

	component.setObjVarItem (Objvars::engineAccelerationRate,      m_engineAccelerationRate);
	component.setObjVarItem (Objvars::engineDecelerationRate,      m_engineDecelerationRate);
	component.setObjVarItem (Objvars::enginePitchAccelerationRate, convertRadiansToDegrees(m_enginePitchAccelerationRate));
	component.setObjVarItem (Objvars::engineYawAccelerationRate,   convertRadiansToDegrees(m_engineYawAccelerationRate));
	component.setObjVarItem (Objvars::engineRollAccelerationRate,  convertRadiansToDegrees(m_engineRollAccelerationRate));
	component.setObjVarItem (Objvars::enginePitchRateMaximum,      convertRadiansToDegrees(m_enginePitchRateMaximum));
	component.setObjVarItem (Objvars::engineYawRateMaximum,        convertRadiansToDegrees(m_engineYawRateMaximum));
	component.setObjVarItem (Objvars::engineRollRateMaximum,       convertRadiansToDegrees(m_engineRollRateMaximum));
	component.setObjVarItem (Objvars::engineSpeedMaximum,          m_engineSpeedMaximum);
	component.setObjVarItem (Objvars::engineSpeedRotationFactorMaximum,   m_engineSpeedRotationFactorMaximum);
	component.setObjVarItem (Objvars::engineSpeedRotationFactorMinimum, m_engineSpeedRotationFactorMinimum);
	component.setObjVarItem (Objvars::engineSpeedRotationFactorOptimal, m_engineSpeedRotationFactorOptimal);
}


//----------------------------------------------------------------------

void ShipComponentDataEngine::printDebugString      (Unicode::String & result, Unicode::String const & padding) const
{
	ServerShipComponentData::printDebugString (result, padding);
	
	char buf [1024];
	const size_t buf_size = sizeof (buf);
	
	std::string const & nPad = Unicode::wideToNarrow (padding) + "   ";
	
	snprintf (buf, buf_size,
		"%sEngineAccelerationRate:  %f\n"
		"%sEngineDecelerationRate:  %f\n"
		"%sEnginePitchAccelerationRate:  %f\n"
		"%sEngineYawAccelerationRate:  %f\n"
		"%sEngineRollAccelerationRate:  %f\n"
		"%sEnginePitchRateMaximum:  %f\n"
		"%sEngineYawRateMaximum:  %f\n"
		"%sEngineRollRateMaximum:  %f\n"
		"%sEngineSpeedMaximum:  %f\n"
		"%sengineSpeedRotationFactor:  %f-%f, Optimal=%f\n",
		nPad.c_str (), m_engineAccelerationRate,
		nPad.c_str (), m_engineDecelerationRate,
		nPad.c_str (), convertRadiansToDegrees(m_enginePitchAccelerationRate),
		nPad.c_str (), convertRadiansToDegrees(m_engineYawAccelerationRate),
		nPad.c_str (), convertRadiansToDegrees(m_engineRollAccelerationRate),
		nPad.c_str (), convertRadiansToDegrees(m_enginePitchRateMaximum),
		nPad.c_str (), convertRadiansToDegrees(m_engineYawRateMaximum),
		nPad.c_str (), convertRadiansToDegrees(m_engineRollRateMaximum),
		nPad.c_str (), m_engineSpeedMaximum,
		nPad.c_str(), m_engineSpeedRotationFactorMinimum, m_engineSpeedRotationFactorMaximum, m_engineSpeedRotationFactorOptimal);
	
	result += Unicode::narrowToWide (buf);
	
}

//----------------------------------------------------------------------

void ShipComponentDataEngine::getAttributes(std::vector<std::pair<std::string, Unicode::String> > & data) const
{
	ServerShipComponentData::getAttributes(data);

	Unicode::String attrib;
	static char buffer[128];
	static const size_t buffer_size = sizeof (buffer);

	snprintf(buffer, buffer_size, "%.1f", convertRadiansToDegrees(m_enginePitchRateMaximum));
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_engine_pitch_rate_maximum, attrib));
	
	snprintf(buffer, buffer_size, "%.1f", convertRadiansToDegrees(m_engineYawRateMaximum));
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_engine_yaw_rate_maximum, attrib));

	snprintf(buffer, buffer_size, "%.1f", convertRadiansToDegrees(m_engineRollRateMaximum));
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_engine_roll_rate_maximum, attrib));

	snprintf(buffer, buffer_size, "%.1f", m_engineSpeedMaximum);
	attrib = Unicode::narrowToWide(buffer);
	data.push_back(std::make_pair(cm_shipComponentCategory + SharedObjectAttributes::ship_component_engine_speed_maximum, attrib));
}

//======================================================================
