//======================================================================
//
// ShipObject_Components.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipObject_Components.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/MissileManager.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipComponentDataEngine.h"
#include "serverGame/ShipComponentDataManager.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentFlags.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedGame/ShipTurretManager.h"
#include "sharedLog/Log.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Hardpoint.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedObject/VolumeContainer.h"


// ======================================================================

namespace ShipObjectNamespace
{
	ConstCharCrcString const s_turretPitchHardpoint("turretpitch1");
}
using namespace ShipObjectNamespace;

// ======================================================================

uint32 ShipObject::getChassisType () const
{
	return m_chassisType.get ();
}

//----------------------------------------------------------------------

/**
 * Returns [0.0, 1.0] relating to the hitpoints of the chassis and all installed components
 * 1.0 == pristine, 0.0 == totally broken (though there are other rules to determine if a ship is flyable)
 */
float ShipObject::getOverallHealth() const
{
	float total = 0.0f;
	int numComponents = 0;
	if(getMaximumChassisHitPoints() > 0.0)
	{
		total += getCurrentChassisHitPoints() / getMaximumChassisHitPoints();
		++numComponents;
	}

	for(int i = ShipChassisSlotType::SCST_first; i != ShipChassisSlotType::SCST_num_types; ++i)
	{
		if(isSlotInstalled(i))
		{
			if(getComponentHitpointsMaximum(i) > 0.0f)
			{
				total += getComponentHitpointsCurrent(i) / getComponentHitpointsMaximum(i);
				++numComponents;
			}
		}
	}

	if(numComponents <= 0)
		return 0.0f;
	else
		return clamp(0.0f, total / numComponents, 1.0f);
}

//----------------------------------------------------------------------

float ShipObject::getOverallHealthWithShieldsAndArmor() const
{
	int numberObjectsToWeight = 0;

	float total = getOverallHealth();
	++numberObjectsToWeight;

	float const shieldHitpointsFrontMaximum = getShieldHitpointsFrontMaximum();
	if (shieldHitpointsFrontMaximum != 0.0)
	{
		total += getShieldHitpointsFrontCurrent() / shieldHitpointsFrontMaximum;
		++numberObjectsToWeight;
	}

	float const shieldHitpointsBackMaximum = getShieldHitpointsBackMaximum();
	if (shieldHitpointsBackMaximum != 0.0)
	{
		total += getShieldHitpointsBackCurrent() / shieldHitpointsBackMaximum;
		++numberObjectsToWeight;
	}

	float const componentArmorHitpointsMaximum0 = getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_0);
	if (componentArmorHitpointsMaximum0 != 0.0)
	{
		total += getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_0) / componentArmorHitpointsMaximum0;
		++numberObjectsToWeight;
	}

	float const componentArmorHitpointsMaximum1 = getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_1);
	if (componentArmorHitpointsMaximum1 != 0.0)
	{
		total += getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_1) / componentArmorHitpointsMaximum1;
		++numberObjectsToWeight;
	}

	return clamp(0.0f, total / static_cast<float>(numberObjectsToWeight), 1.0f);
}

//----------------------------------------------------------------------

float ShipObject::getComponentArmorHitpointsMaximum (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentArmorHitpointsMaximum.find (chassisSlot);
	if (it != m_componentArmorHitpointsMaximum.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentArmorHitpointsCurrent (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentArmorHitpointsCurrent.find (chassisSlot);
	if (it != m_componentArmorHitpointsCurrent.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentEfficiencyGeneral (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentEfficiencyGeneral.find (chassisSlot);
	if (it != m_componentEfficiencyGeneral.end ())
		return (*it).second;
	return 1.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentEfficiencyEnergy (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentEfficiencyEnergy.find (chassisSlot);
	if (it != m_componentEfficiencyEnergy.end ())
		return (*it).second;
	return 1.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentEnergyMaintenanceRequirement (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentEnergyMaintenanceRequirement.find (chassisSlot);
	if (it != m_componentEnergyMaintenanceRequirement.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentMass (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentMass.find (chassisSlot);
	if (it != m_componentMass.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

uint32 ShipObject::getComponentCrc (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, uint32>::const_iterator const it = m_componentCrc.find (chassisSlot);
	if (it != m_componentCrc.end ())
		return (*it).second;
	return 0;
}

//----------------------------------------------------------------------

float ShipObject::getComponentHitpointsCurrent (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentHitpointsCurrent.find (chassisSlot);
	if (it != m_componentHitpointsCurrent.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentHitpointsMaximum (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentHitpointsMaximum.find (chassisSlot);
	if (it != m_componentHitpointsMaximum.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

int ShipObject::getComponentFlags (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, int>::const_iterator const it = m_componentFlags.find (chassisSlot);
	if (it != m_componentFlags.end ())
		return (*it).second;
	return 0;
}

//----------------------------------------------------------------------

Unicode::String const &ShipObject::getComponentName(int chassisSlot) const
{
	Archive::AutoDeltaMap<int, Unicode::String>::const_iterator const it = m_componentNames.find (chassisSlot);
	if (it != m_componentNames.end() && !(*it).second.empty())
		return (*it).second;

	return Unicode::emptyString;
}

//----------------------------------------------------------------------

NetworkId const &ShipObject::getComponentCreator(int chassisSlot) const
{
	Archive::AutoDeltaMap<int, NetworkId>::const_iterator const it = m_componentCreators.find (chassisSlot);
	if (it != m_componentCreators.end())
		return (*it).second;

	return NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponDamageMaximum (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponDamageMaximum.find (chassisSlot);
	if (it != m_weaponDamageMaximum.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponDamageMinimum (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponDamageMinimum.find (chassisSlot);
	if (it != m_weaponDamageMinimum.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponEffectivenessShields (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponEffectivenessShields.find (chassisSlot);
	if (it != m_weaponEffectivenessShields.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponEffectivenessArmor (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponEffectivenessArmor.find (chassisSlot);
	if (it != m_weaponEffectivenessArmor.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponEnergyPerShot (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponEnergyPerShot.find (chassisSlot);
	if (it != m_weaponEnergyPerShot.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponActualEnergyPerShot(int chassisSlot) const
{
	float const weaponEfficiencyEnergy = std::max(0.1f, getComponentEfficiencyEnergy(chassisSlot));
	return getWeaponEnergyPerShot(chassisSlot) / weaponEfficiencyEnergy;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponRefireRate (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponRefireRate.find (chassisSlot);
	if (it != m_weaponRefireRate.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponActualRefireRate         (int chassisSlot) const
{
	if (!isSlotInstalled(chassisSlot))
		return 0.0f;

	float const eff = clamp(0.1f, getWeaponEfficiencyRefireRate(chassisSlot), 10.0f);
	return getWeaponRefireRate(chassisSlot) / eff;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponEfficiencyRefireRate (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponEfficiencyRefireRate.find (chassisSlot);
	if (it != m_weaponEfficiencyRefireRate.end ())
		return (*it).second;
	return 1.0f;
}

//----------------------------------------------------------------------

int ShipObject::getWeaponAmmoCurrent(int chassisSlot) const
{
	Archive::AutoDeltaMap<int, int>::const_iterator const it = m_weaponAmmoCurrent.find (chassisSlot);
	if (it != m_weaponAmmoCurrent.end ())
		return (*it).second;
	return 0;
}

//----------------------------------------------------------------------

int ShipObject::getWeaponAmmoMaximum(int chassisSlot) const
{
	Archive::AutoDeltaMap<int, int>::const_iterator const it = m_weaponAmmoMaximum.find (chassisSlot);
	if (it != m_weaponAmmoMaximum.end ())
		return (*it).second;
	return 0;
}

//----------------------------------------------------------------------

uint32 ShipObject::getWeaponAmmoType(int chassisSlot) const
{
	Archive::AutoDeltaMap<int, uint32>::const_iterator const it = m_weaponAmmoType.find (chassisSlot);
	if (it != m_weaponAmmoType.end ())
		return (*it).second;
	return 0;
}

//----------------------------------------------------------------------

float ShipObject::getShieldHitpointsFrontCurrent () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldHitpointsFrontCurrent.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getShieldHitpointsFrontMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldHitpointsFrontMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getShieldHitpointsBackCurrent () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldHitpointsBackCurrent.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getShieldHitpointsBackMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldHitpointsBackMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getShieldRechargeRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldRechargeRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getCapacitorEnergyCurrent () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_capacitor))
		return m_capacitorEnergyCurrent.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getCapacitorEnergyMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_capacitor))
		return m_capacitorEnergyMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getCapacitorEnergyEffectiveMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_capacitor) && isComponentFunctional(ShipChassisSlotType::SCST_capacitor))
		return m_capacitorEnergyMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getCapacitorEnergyRechargeRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_capacitor))
		return m_capacitorEnergyRechargeRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineAccelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineAccelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineDecelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineDecelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEnginePitchAccelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_enginePitchAccelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineYawAccelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineYawAccelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineRollAccelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineRollAccelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEnginePitchRateMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_enginePitchRateMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineYawRateMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineYawRateMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineRollRateMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineRollRateMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineSpeedMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineSpeedMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineSpeedRotationFactorMaximum() const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineSpeedRotationFactorMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineSpeedRotationFactorMinimum() const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineSpeedRotationFactorMinimum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineSpeedRotationFactorOptimal() const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineSpeedRotationFactorOptimal.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getReactorEnergyGenerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_reactor))
		return m_reactorEnergyGenerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterEnergyCurrent () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterEnergyCurrent.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterEnergyMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterEnergyMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterEnergyRechargeRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterEnergyRechargeRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterEnergyConsumptionRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterEnergyConsumptionRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterAcceleration () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterAcceleration.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterSpeedMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterSpeedMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getDroidInterfaceCommandSpeed () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_droid_interface))
		return m_droidInterfaceCommandSpeed.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

bool ShipObject::hasSlot (int chassisSlot) const
{
	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (getChassisType ());
	return shipChassis && shipChassis->getSlot (static_cast<ShipChassisSlotType::Type>(chassisSlot));
}

//----------------------------------------------------------------------

bool ShipObject::setComponentArmorHitpointsMaximum (int chassisSlot, float componentArmorHitpointsMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentArmorHitpointsMaximum () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentArmorHitpointsMaximum () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_componentArmorHitpointsMaximum.set (chassisSlot, componentArmorHitpointsMaximum);
	//-- force update of current hitpoints
	IGNORE_RETURN(setComponentArmorHitpointsCurrent (chassisSlot, getComponentArmorHitpointsCurrent (chassisSlot)));
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentArmorHitpointsCurrent (int chassisSlot, float componentArmorHitpointsCurrent)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentArmorHitpointsCurrent () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentArmorHitpointsCurrent () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	componentArmorHitpointsCurrent = std::min (getComponentArmorHitpointsMaximum (chassisSlot), componentArmorHitpointsCurrent);
	m_componentArmorHitpointsCurrent.set (chassisSlot, componentArmorHitpointsCurrent);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentEfficiencyGeneral (int chassisSlot, float componentEfficiencyGeneral)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentEfficiencyGeneral () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentEfficiencyGeneral () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	componentEfficiencyGeneral = clamp (0.1f, componentEfficiencyGeneral, 10.0f);
	m_componentEfficiencyGeneral.set (chassisSlot, componentEfficiencyGeneral);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentEfficiencyEnergy (int chassisSlot, float componentEfficiencyEnergy)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentEfficiencyEnergy () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentEfficiencyEnergy () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	componentEfficiencyEnergy = clamp (0.1f, componentEfficiencyEnergy, 10.0f);
	m_componentEfficiencyEnergy.set (chassisSlot, componentEfficiencyEnergy);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentEnergyMaintenanceRequirement (int chassisSlot, float componentEnergyMaintenanceRequirement)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentEnergyMaintenanceRequirement () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentEnergyMaintenanceRequirement () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_componentEnergyMaintenanceRequirement.set (chassisSlot, componentEnergyMaintenanceRequirement);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentMass (int chassisSlot, float componentMass)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentMass () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentMass () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_componentMass.set (chassisSlot, componentMass);

	internalSetChassisComponentMassCurrent(computeChassisComponentMassCurrent());
	return true;
}

//----------------------------------------------------------------------

/**
* Setting a component template crc to zero removes the component
*/

void ShipObject::setComponentCrc (int chassisSlot, uint32 componentCrc)
{
	if (componentCrc == 0)
	{
		IGNORE_RETURN (m_componentArmorHitpointsMaximum.erase (chassisSlot));
		IGNORE_RETURN (m_componentArmorHitpointsCurrent.erase (chassisSlot));
		IGNORE_RETURN (m_componentEfficiencyGeneral.erase (chassisSlot));
		IGNORE_RETURN (m_componentEfficiencyEnergy.erase (chassisSlot));
		IGNORE_RETURN (m_componentEnergyMaintenanceRequirement.erase (chassisSlot));
		IGNORE_RETURN (m_componentMass.erase (chassisSlot));
		IGNORE_RETURN (m_componentCrc.erase (chassisSlot));
		IGNORE_RETURN (m_componentCrcForClient.erase (chassisSlot));
		IGNORE_RETURN (m_componentHitpointsCurrent.erase (chassisSlot));
		IGNORE_RETURN (m_componentHitpointsMaximum.erase (chassisSlot));
		IGNORE_RETURN (m_componentFlags.erase (chassisSlot));
		IGNORE_RETURN (m_componentNames.erase (chassisSlot));
		IGNORE_RETURN (m_weaponDamageMaximum.erase (chassisSlot));
		IGNORE_RETURN (m_weaponDamageMinimum.erase (chassisSlot));
		IGNORE_RETURN (m_weaponEffectivenessShields.erase (chassisSlot));
		IGNORE_RETURN (m_weaponEffectivenessArmor.erase (chassisSlot));
		IGNORE_RETURN (m_weaponEnergyPerShot.erase (chassisSlot));
		IGNORE_RETURN (m_weaponRefireRate.erase (chassisSlot));
		IGNORE_RETURN (m_componentCreators.erase (chassisSlot));

		internalSetChassisComponentMassCurrent(computeChassisComponentMassCurrent());
	}
	else
	{
		m_componentCrc.set (chassisSlot, componentCrc);
		m_componentCrcForClient.set (chassisSlot, componentCrc);

		//-- all components start out at low power so script gets the first power up pulse
		IGNORE_RETURN(setComponentLowPower (chassisSlot, true));

		internalSetChassisComponentMassCurrent(computeChassisComponentMassCurrent());
	}

	if (ShipChassisSlotType::isWeaponChassisSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot)))
	{
		// Cache the turret positions
		{
			int const weaponIndex = ShipChassisSlotType::getWeaponIndex(static_cast<ShipChassisSlotType::Type>(chassisSlot));

			if (ShipTurretManager::isTurret(getChassisType(), weaponIndex))
			{
				m_turretWeaponIndices->setBit(weaponIndex);
			}
			else
			{
				m_turretWeaponIndices->clearBit(weaponIndex);
			}
		}

		// Cache for the fastest projectile weapon speed
		{
			m_fastestWeaponProjectileSpeed = 0.0f;

			for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
			{
				if (isProjectile(weaponIndex))
				{
					m_fastestWeaponProjectileSpeed = std::max(m_fastestWeaponProjectileSpeed, getWeaponProjectileSpeed(weaponIndex));
				}
			}
		}
	}
}

//----------------------------------------------------------------------

bool ShipObject::setComponentHitpointsCurrent (int chassisSlot, float componentHitpointsCurrent)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentHitpointsCurrent () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentHitpointsCurrent () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	float const maxHp = getComponentHitpointsMaximum(chassisSlot);

	componentHitpointsCurrent = std::min (maxHp, componentHitpointsCurrent);
	m_componentHitpointsCurrent.set (chassisSlot, componentHitpointsCurrent);

	bool const isDemolished = isComponentDemolished(chassisSlot);

	if (componentHitpointsCurrent <= 0.0f && maxHp > 0.0f && !isDemolished)
		IGNORE_RETURN(setComponentDemolished(chassisSlot, true));
	else if (isDemolished)
		IGNORE_RETURN(setComponentDemolished(chassisSlot, false));

	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentHitpointsMaximum (int chassisSlot, float componentHitpointsMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentHitpointsMaximum () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentHitpointsMaximum () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_componentHitpointsMaximum.set (chassisSlot, componentHitpointsMaximum);
	IGNORE_RETURN(setComponentHitpointsCurrent (chassisSlot, getComponentHitpointsCurrent (chassisSlot)));
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentFlags (int chassisSlot, int componentFlags)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentFlags () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentFlags () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_componentFlags.set (chassisSlot, componentFlags);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentName(int chassisSlot, Unicode::String const & componentName)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentName () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentName () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_componentNames.set (chassisSlot, componentName);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentCreator(int chassisSlot, NetworkId const & componentCreator)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentName () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentCreator () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	if (componentCreator != NetworkId::cms_invalid)
		m_componentCreators.set (chassisSlot, componentCreator);
	else
		m_componentCreators.erase(chassisSlot);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentDisabled                     (int chassisSlot, bool componentDisabled)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentDisabled                     () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentDisabled                     () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	int const flags = getComponentFlags (chassisSlot);

	if (componentDisabled)
	{
		IGNORE_RETURN(setComponentFlags(chassisSlot, flags | static_cast<int>(ShipComponentFlags::F_disabled)));
		IGNORE_RETURN(setComponentLowPower(chassisSlot, true));
		IGNORE_RETURN(setComponentActive(chassisSlot, false));
	}
	else
	{
		IGNORE_RETURN(setComponentFlags (chassisSlot, flags & ~static_cast<int>(ShipComponentFlags::F_disabled)));
		IGNORE_RETURN(setComponentDisabledNeedsPower(chassisSlot, false));
	}
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentDisabledNeedsPower(int chassisSlot, bool componentDisabledNeedsPower)
{
	FATAL(isInitialized() && !isAuthoritative(), ("ShipObject::setComponentDisabledNeedsPower() called on non-auth object."));
	if (!isSlotInstalled(chassisSlot))
	{
		WARNING(true, ("ShipObject::setComponentDisabledNeedsPower() called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	int const flags = getComponentFlags(chassisSlot);

	if (componentDisabledNeedsPower)
		IGNORE_RETURN(setComponentFlags(chassisSlot, flags | static_cast<int>(ShipComponentFlags::F_disabledNeedsPower)));
	else
		IGNORE_RETURN(setComponentFlags(chassisSlot, flags & ~static_cast<int>(ShipComponentFlags::F_disabledNeedsPower)));

	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentLowPower                     (int chassisSlot, bool componentLowPower)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentLowPower                     () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentLowPower                     () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	int const flags = getComponentFlags (chassisSlot);

	if (componentLowPower)
		IGNORE_RETURN(setComponentFlags (chassisSlot, flags | static_cast<int>(ShipComponentFlags::F_lowPower)));
	else
		IGNORE_RETURN(setComponentFlags (chassisSlot, flags & ~static_cast<int>(ShipComponentFlags::F_lowPower)));
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentActive                       (int chassisSlot, bool componentActive)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentActive() called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentActive() called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}
	
	int const flags = getComponentFlags (chassisSlot);
	
	if (componentActive)
		IGNORE_RETURN(setComponentFlags (chassisSlot, flags | static_cast<int>(ShipComponentFlags::F_active)));
	else
		IGNORE_RETURN(setComponentFlags (chassisSlot, flags & ~static_cast<int>(ShipComponentFlags::F_active)));
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setComponentDemolished(int chassisSlot, bool const componentDemolished)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setComponentDemolished() called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setComponentDemolished() called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}
	
	int const flags = getComponentFlags (chassisSlot);
	
	if (componentDemolished)
		IGNORE_RETURN(setComponentFlags (chassisSlot, flags | static_cast<int>(ShipComponentFlags::F_demolished)));
	else
		IGNORE_RETURN(setComponentFlags (chassisSlot, flags & ~static_cast<int>(ShipComponentFlags::F_demolished)));
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponDamageMaximum (int chassisSlot, float weaponDamageMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponDamageMaximum () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponDamageMaximum () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponDamageMaximum.set (chassisSlot, weaponDamageMaximum);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponDamageMinimum (int chassisSlot, float weaponDamageMinimum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponDamageMinimum () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponDamageMinimum () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponDamageMinimum.set (chassisSlot, weaponDamageMinimum);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponEffectivenessShields (int chassisSlot, float weaponEffectivenessShields)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponEffectivenessShields () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponEffectivenessShields () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponEffectivenessShields.set (chassisSlot, weaponEffectivenessShields);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponEffectivenessArmor (int chassisSlot, float weaponEffectivenessArmor)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponEffectivenessArmor () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponEffectivenessArmor () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponEffectivenessArmor.set (chassisSlot, weaponEffectivenessArmor);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponEnergyPerShot (int chassisSlot, float weaponEnergyPerShot)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponEnergyPerShot () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponEnergyPerShot () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponEnergyPerShot.set (chassisSlot, weaponEnergyPerShot);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponRefireRate (int chassisSlot, float weaponRefireRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponRefireRate () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponRefireRate () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponRefireRate.set (chassisSlot, weaponRefireRate);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponEfficiencyRefireRate (int chassisSlot, float weaponEfficiencyRefireRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponEfficiencyRefireRate () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponEfficiencyRefireRate () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponEfficiencyRefireRate.set (chassisSlot, weaponEfficiencyRefireRate);
	return true;
}

// ----------------------------------------------------------------------

/**
 * Set a weapon's refire timer to have the specified percentage of its time
 * already elapsed.  For example, 0.25 will prevent the weapon from firing
 * again until 75% of its regular refire time has elapsed.  This setting
 * applies to the next shot only; future shots will use the regular refire rate.
 */
void ShipObject::setWeaponRefireTimerPercent(int weaponIndex, float refireTimerPercent)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponRefireTimerPercent () called on non-auth object."));
	m_weaponRefireTimers[weaponIndex].setElapsedTime(refireTimerPercent * getWeaponActualRefireRate(static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + weaponIndex));
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponAmmoCurrent(int chassisSlot, int weaponAmmoCurrent)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponProjectileSpeed () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponAmmoCurrent () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponAmmoCurrent.set (chassisSlot, weaponAmmoCurrent);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponAmmoMaximum(int chassisSlot, int weaponAmmoMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponProjectileSpeed () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponAmmoMaximum () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponAmmoMaximum.set (chassisSlot, weaponAmmoMaximum);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setWeaponAmmoType(int chassisSlot, uint32 weaponAmmoType)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setWeaponProjectileSpeed () called on non-auth object."));
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::setWeaponAmmoType () called on slot [%d] with nothing installed.", chassisSlot));
		return false;
	}

	m_weaponAmmoType.set (chassisSlot, weaponAmmoType);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setShieldHitpointsFrontCurrent (float shieldHitpointsCurrent)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setShieldHitpointsFrontCurrent () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_shield_0)))
	{
		WARNING (true, ("ShipObject::setShieldHitpointsFrontCurrent () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_shield_0)));
		return false;
	}

	shieldHitpointsCurrent = std::min (getShieldHitpointsFrontMaximum (), shieldHitpointsCurrent);
	m_shieldHitpointsFrontCurrent.set (shieldHitpointsCurrent);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setShieldHitpointsFrontMaximum (float shieldHitpointsMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setShieldHitpointsFrontMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_shield_0)))
	{
		WARNING (true, ("ShipObject::setShieldHitpointsFrontMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_shield_0)));
		return false;
	}

	m_shieldHitpointsFrontMaximum.set (shieldHitpointsMaximum);
	IGNORE_RETURN(setShieldHitpointsFrontCurrent (getShieldHitpointsFrontCurrent ()));
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setShieldHitpointsBackCurrent (float shieldHitpointsCurrent)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setShieldHitpointsBackCurrent () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_shield_0)))
	{
		WARNING (true, ("ShipObject::setShieldHitpointsBackCurrent () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_shield_0)));
		return false;
	}

	shieldHitpointsCurrent = std::min (getShieldHitpointsBackMaximum (), shieldHitpointsCurrent);
	m_shieldHitpointsBackCurrent.set (shieldHitpointsCurrent);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setShieldHitpointsBackMaximum (float shieldHitpointsMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setShieldHitpointsBackMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_shield_0)))
	{
		WARNING (true, ("ShipObject::setShieldHitpointsBackMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_shield_0)));
		return false;
	}

	m_shieldHitpointsBackMaximum.set (shieldHitpointsMaximum);
	IGNORE_RETURN(setShieldHitpointsBackCurrent (getShieldHitpointsBackCurrent ()));
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setShieldRechargeRate (float shieldRechargeRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setShieldRechargeRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_shield_0)))
	{
		WARNING (true, ("ShipObject::setShieldRechargeRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_shield_0)));
		return false;
	}

	m_shieldRechargeRate.set (shieldRechargeRate);
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setCapacitorEnergyCurrent (float capacitorEnergyCurrent)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setCapacitorEnergyCurrent () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_capacitor)))
	{
		WARNING (true, ("ShipObject::setCapacitorEnergyCurrent () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_capacitor)));
		return false;
	}

	float const capacitorEnergyMaximum = getCapacitorEnergyEffectiveMaximum () * getComponentEfficiencyGeneral(ShipChassisSlotType::SCST_capacitor);
	capacitorEnergyCurrent = std::min (capacitorEnergyMaximum, capacitorEnergyCurrent);
	m_capacitorEnergyCurrent = capacitorEnergyCurrent;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setCapacitorEnergyMaximum (float capacitorEnergyMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setCapacitorEnergyMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_capacitor)))
	{
		WARNING (true, ("ShipObject::setCapacitorEnergyMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_capacitor)));
		return false;
	}

	m_capacitorEnergyMaximum = capacitorEnergyMaximum;
	IGNORE_RETURN(setCapacitorEnergyCurrent (getCapacitorEnergyCurrent ()));
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setCapacitorEnergyRechargeRate (float capacitorEnergyRechargeRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setCapacitorEnergyRechargeRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_capacitor)))
	{
		WARNING (true, ("ShipObject::setCapacitorEnergyRechargeRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_capacitor)));
		return false;
	}

	m_capacitorEnergyRechargeRate = capacitorEnergyRechargeRate;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineAccelerationRate (float engineAccelerationRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineAccelerationRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineAccelerationRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineAccelerationRate = engineAccelerationRate;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineDecelerationRate (float engineDecelerationRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineDecelerationRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineDecelerationRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineDecelerationRate = engineDecelerationRate;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEnginePitchAccelerationRate (float enginePitchAccelerationRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEnginePitchAccelerationRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEnginePitchAccelerationRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_enginePitchAccelerationRate = enginePitchAccelerationRate;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineYawAccelerationRate (float engineYawAccelerationRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineYawAccelerationRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineYawAccelerationRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineYawAccelerationRate = engineYawAccelerationRate;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineRollAccelerationRate (float engineRollAccelerationRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineRollAccelerationRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineRollAccelerationRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineRollAccelerationRate = engineRollAccelerationRate;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEnginePitchRateMaximum (float enginePitchRateMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEnginePitchRateMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEnginePitchRateMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_enginePitchRateMaximum = enginePitchRateMaximum;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineYawRateMaximum (float engineYawRateMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineYawRateMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineYawRateMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineYawRateMaximum = engineYawRateMaximum;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineRollRateMaximum (float engineRollRateMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineRollRateMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineRollRateMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineRollRateMaximum = engineRollRateMaximum;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineSpeedMaximum (float engineSpeedMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineSpeedMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineSpeedMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineSpeedMaximum = engineSpeedMaximum;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineSpeedRotationFactorMaximum(float engineSpeedRotationFactor)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineSpeedRotationFactorMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineSpeedRotationFactorMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineSpeedRotationFactorMaximum = engineSpeedRotationFactor;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineSpeedRotationFactorMinimum(float engineSpeedRotationFactorMin)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineSpeedRotationFactorMinimum() called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineSpeedRotationFactorMinimum() called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineSpeedRotationFactorMinimum = engineSpeedRotationFactorMin;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setEngineSpeedRotationFactorOptimal(float engineSpeedRotationFactorOptimal)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setEngineSpeedRotationFactorOptimal() called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_engine)))
	{
		WARNING (true, ("ShipObject::setEngineSpeedRotationFactorOptimal() called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_engine)));
		return false;
	}

	m_engineSpeedRotationFactorOptimal = engineSpeedRotationFactorOptimal;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setReactorEnergyGenerationRate (float reactorEnergyGenerationRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setReactorEnergyGenerationRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_reactor)))
	{
		WARNING (true, ("ShipObject::setReactorEnergyGenerationRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_reactor)));
		return false;
	}

	m_reactorEnergyGenerationRate = reactorEnergyGenerationRate;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setBoosterEnergyCurrent (float boosterEnergyCurrent)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setBoosterEnergyCurrent () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_booster)))
	{
		WARNING (true, ("ShipObject::setBoosterEnergyCurrent () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_booster)));
		return false;
	}

	boosterEnergyCurrent = std::min (getBoosterEnergyMaximum (), boosterEnergyCurrent);
	m_boosterEnergyCurrent = boosterEnergyCurrent;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setBoosterEnergyMaximum (float boosterEnergyMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setBoosterEnergyMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_booster)))
	{
		WARNING (true, ("ShipObject::setBoosterEnergyMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_booster)));
		return false;
	}

	m_boosterEnergyMaximum = boosterEnergyMaximum;
	IGNORE_RETURN(setBoosterEnergyCurrent (getBoosterEnergyCurrent ()));
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setBoosterEnergyRechargeRate (float boosterEnergyRechargeRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setBoosterEnergyRechargeRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_booster)))
	{
		WARNING (true, ("ShipObject::setBoosterEnergyRechargeRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_booster)));
		return false;
	}

	m_boosterEnergyRechargeRate = boosterEnergyRechargeRate;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setBoosterEnergyConsumptionRate (float boosterEnergyConsumptionRate)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setBoosterEnergyConsumptionRate () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_booster)))
	{
		WARNING (true, ("ShipObject::setBoosterEnergyConsumptionRate () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_booster)));
		return false;
	}

	m_boosterEnergyConsumptionRate = boosterEnergyConsumptionRate;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setBoosterAcceleration (float boosterAcceleration)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setBoosterAcceleration () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_booster)))
	{
		WARNING (true, ("ShipObject::setBoosterAcceleration () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_booster)));
		return false;
	}

	m_boosterAcceleration = boosterAcceleration;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setBoosterSpeedMaximum (float boosterSpeedMaximum)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setBoosterSpeedMaximum () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_booster)))
	{
		WARNING (true, ("ShipObject::setBoosterSpeedMaximum () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_booster)));
		return false;
	}

	m_boosterSpeedMaximum = boosterSpeedMaximum;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::setDroidInterfaceCommandSpeed (float droidInterfaceCommandSpeed)
{
	FATAL (isInitialized () && !isAuthoritative (), ("ShipObject::setDroidInterfaceCommandSpeed () called on non-auth object."));
	if (!isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_droid_interface)))
	{
		WARNING (true, ("ShipObject::setDroidInterfaceCommandSpeed () called on slot [%d] with nothing installed.", static_cast<int>(ShipChassisSlotType::SCST_droid_interface)));
		return false;
	}

	m_droidInterfaceCommandSpeed = droidInterfaceCommandSpeed;
	return true;
}

//----------------------------------------------------------------------

bool ShipObject::canInstallComponent     (int chassisSlot, TangibleObject const & component) const
{
	ShipComponentData const * const shipComponentData = ShipComponentDataManager::create (component);
	if (!shipComponentData)
	{
		WARNING (true, ("ShipObject::canInstallComponent failed for [%s] because a ShipComponentData could not be constructed.", component.getNetworkId ().getValueString ().c_str ()));
		return false;
	}

	if (chassisSlot < static_cast<int>(ShipChassisSlotType::SCST_first) ||
		chassisSlot >= static_cast<int>(ShipChassisSlotType::SCST_num_types))
	{
		WARNING (true, ("ShipObject::canInstallComponent failed for [%s] because a chassisSlot was invalid.", component.getNetworkId ().getValueString ().c_str ()));
		return false;
	}

	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (getChassisType ());
	if (shipChassis)
	{
		int effectiveCompatibilitySlot = chassisSlot;
		// weapon slots beyond 7 wrap back to slot 0 for the purposes of compatibility checking on captial ships
		if (isCapitalShip() && chassisSlot >= ShipChassisSlotType::SCST_num_explicit_types && chassisSlot <= ShipChassisSlotType::SCST_weapon_last)
			effectiveCompatibilitySlot = ShipChassisSlotType::SCST_weapon_first+((chassisSlot-ShipChassisSlotType::SCST_weapon_first)&7);

		ShipChassisSlot const * const slot = shipChassis->getSlot (static_cast<ShipChassisSlotType::Type>(effectiveCompatibilitySlot));

		if (slot != nullptr)
		{
			if (slot->canAcceptComponent (shipComponentData->getDescriptor ()))
			{
				//-- check for mass availability
				float const massAvailable = getChassisComponentMassMaximum() - getChassisComponentMassCurrent();
				return massAvailable >= shipComponentData->m_mass;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

/**
* This function destroys the component
*/

bool ShipObject::installComponent   (NetworkId const & installerId, int chassisSlot, TangibleObject & component)
{
	if (!canInstallComponent (chassisSlot, component))
	{
		WARNING (true, ("ShipObject::installComponent failed for [%s] because the component can't go here.", component.getNetworkId ().getValueString ().c_str ()));
		return false;
	}

	if (isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::installComponent failed for [%s] because the slot is occupied.", component.getNetworkId ().getValueString ().c_str ()));
		return false;
	}

	ShipComponentData const * const shipComponentData = ShipComponentDataManager::create (component);
	if (!shipComponentData)
	{
		WARNING (true, ("ShipObject::installComponent failed for [%s] because a ShipComponentData could not be constructed.", component.getNetworkId ().getValueString ().c_str ()));
		return false;
	}

	if(getScriptObject())
	{
		ScriptParams p;
		p.addParam(installerId);
		p.addParam(component.getNetworkId ());
		p.addParam(chassisSlot);

		if (getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_COMPONENT_INSTALLING, p) == SCRIPT_OVERRIDE)
			return false;
	}

	shipComponentData->writeDataToShip (chassisSlot, *this);

	LOG(
		"CustomerService",
		(
			"ShipComponents: %s (%s), [%s]: added to %s (%s) by %s",
			Unicode::wideToNarrow(component.getObjectName()).c_str(),
			component.getNetworkId().getValueString().c_str(),
			component.getObjectTemplateName(),
			Unicode::wideToNarrow(getObjectName()).c_str(),
			getNetworkId().getValueString().c_str(),
			PlayerObject::getAccountDescription(getOwnerId()).c_str()));

	IGNORE_RETURN (component.permanentlyDestroy (DeleteReasons::Consumed));

	if(getScriptObject())
	{
		ScriptParams p;
		p.addParam(installerId);
		p.addParam (chassisSlot);

		IGNORE_RETURN (getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_COMPONENT_INSTALLED, p));
	}

	return true;
}

//----------------------------------------------------------------------

void ShipObject::purgeComponent          (int chassisSlot)
{
	IGNORE_RETURN (internalUninstallComponent (NetworkId::cms_invalid, chassisSlot, nullptr));
}

//----------------------------------------------------------------------

TangibleObject *    ShipObject::internalUninstallComponent      (NetworkId const & uninstallerId, int chassisSlot, ServerObject * containerTarget, bool allowOverload)
{
	if (!isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("ShipObject::internalUninstallComponent failed... no component installed in slot"));
		return nullptr;
	}

	ShipComponentData * const shipComponentData = createShipComponentData (chassisSlot);

	if (shipComponentData == nullptr)
	{
		WARNING (true, ("ShipObject::internalUninstallComponent failed nullptr data"));
		delete shipComponentData;
		return nullptr;
	}

	if(getScriptObject())
	{
		ScriptParams p;
		p.addParam(uninstallerId);
		p.addParam (chassisSlot);
		p.addParam (containerTarget ? containerTarget->getNetworkId () : NetworkId::cms_invalid);

		if (getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_COMPONENT_UNINSTALLING, p) == SCRIPT_OVERRIDE)
			return nullptr;
	}

	TangibleObject * tangible = 0;

	if (containerTarget)
	{
		uint32 const objectTemplateCrc = shipComponentData->getDescriptor ().getObjectTemplateCrc ();

		ServerObject * obj = 0;
		if (allowOverload)
		{

			VolumeContainer * volContainer = ContainerInterface::getVolumeContainer(*containerTarget);
			if (volContainer == nullptr)
				return 0;

			int oldCapacity = volContainer->debugDoNotUseSetCapacity(-1);

			obj = ServerWorld::createNewObject(objectTemplateCrc, *containerTarget, true, true);

			volContainer->debugDoNotUseSetCapacity(oldCapacity);
			volContainer->recalculateVolume();
		}
		else
		{
			obj = safe_cast<ServerObject *>(ServerWorld::createNewObject (objectTemplateCrc, *containerTarget, true));
		}

		if (obj == nullptr)
		{
			WARNING (true, ("ShipObject::internalUninstallComponent failed for slot [%d] because ServerWorld could not create an object for [%d], or container is full", chassisSlot, objectTemplateCrc));
			delete shipComponentData;
			return nullptr;
		}

		tangible = obj->asTangibleObject ();

		if (tangible == nullptr)
		{
			WARNING (true, ("ShipObject::internalUninstallComponent failed for slot [%d] because object template [%d] is not tangible", chassisSlot, objectTemplateCrc));
			delete shipComponentData;
			IGNORE_RETURN (obj->permanentlyDestroy (DeleteReasons::SetupFailed));
			return nullptr;
		}

		shipComponentData->writeDataToComponent (*tangible);

		LOG(
			"CustomerService",
			(
				"ShipComponents: %s (%s), [%s]: removed from %s (%s) by %s",
				Unicode::wideToNarrow(tangible->getObjectName()).c_str(),
				tangible->getNetworkId().getValueString().c_str(),
				tangible->getObjectTemplateName(),
				Unicode::wideToNarrow(getObjectName()).c_str(),
				getNetworkId().getValueString().c_str(),
				PlayerObject::getAccountDescription(getOwnerId()).c_str()));
	}

	switch (chassisSlot)
	{
	case ShipChassisSlotType::SCST_reactor:
		IGNORE_RETURN(setReactorEnergyGenerationRate    (0.0f));
		break;
	case ShipChassisSlotType::SCST_engine:
		IGNORE_RETURN(setEngineAccelerationRate         (0.0f));
		IGNORE_RETURN(setEngineDecelerationRate         (0.0f));
		IGNORE_RETURN(setEnginePitchAccelerationRate    (0.0f));
		IGNORE_RETURN(setEngineYawAccelerationRate      (0.0f));
		IGNORE_RETURN(setEngineRollAccelerationRate     (0.0f));
		IGNORE_RETURN(setEnginePitchRateMaximum         (0.0f));
		IGNORE_RETURN(setEngineYawRateMaximum           (0.0f));
		IGNORE_RETURN(setEngineRollRateMaximum          (0.0f));
		IGNORE_RETURN(setEngineSpeedMaximum             (0.0f));
		break;

	case ShipChassisSlotType::SCST_capacitor:
		IGNORE_RETURN(setCapacitorEnergyCurrent         (0.0f));
		IGNORE_RETURN(setCapacitorEnergyMaximum         (0.0f));
		IGNORE_RETURN(setCapacitorEnergyRechargeRate    (0.0f));
		break;

	case ShipChassisSlotType::SCST_shield_0:
		IGNORE_RETURN(setShieldHitpointsFrontCurrent    (0.0f));
		IGNORE_RETURN(setShieldHitpointsFrontMaximum    (0.0f));
		IGNORE_RETURN(setShieldHitpointsBackCurrent     (0.0f));
		IGNORE_RETURN(setShieldHitpointsBackMaximum     (0.0f));
		IGNORE_RETURN(setShieldRechargeRate             (0.0f));
		break;

	case ShipChassisSlotType::SCST_booster:
		IGNORE_RETURN(setBoosterEnergyCurrent           (0.0f));
		IGNORE_RETURN(setBoosterEnergyMaximum           (0.0f));
		IGNORE_RETURN(setBoosterEnergyRechargeRate      (0.0f));
		IGNORE_RETURN(setBoosterEnergyConsumptionRate   (0.0f));
		IGNORE_RETURN(setBoosterAcceleration            (0.0f));
		IGNORE_RETURN(setBoosterSpeedMaximum            (0.0f));
		break;

	case ShipChassisSlotType::SCST_droid_interface:
		IGNORE_RETURN(setDroidInterfaceCommandSpeed     (0.0f));
		break;

	case ShipChassisSlotType::SCST_cargo_hold:
		{
			m_cargoHoldContents.clear();
			setCargoHoldContentsCurrent(0);
			setCargoHoldContentsMaximum(0);
		}
		break;

	default:
		break;
	}

	setComponentCrc (chassisSlot, 0);

	delete shipComponentData;

	if(getScriptObject())
	{
		ScriptParams p;
		p.addParam(uninstallerId);
		p.addParam (tangible ? tangible->getNetworkId () : NetworkId::cms_invalid);
		p.addParam (chassisSlot);
		p.addParam (containerTarget ? containerTarget->getNetworkId () : NetworkId::cms_invalid);

		IGNORE_RETURN (getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_COMPONENT_UNINSTALLED, p));
	}

	return tangible;
}

//----------------------------------------------------------------------

TangibleObject * ShipObject::uninstallComponent (NetworkId const & uninstallerId, int chassisSlot, ServerObject & containerTarget, bool allowOverload)
{
	return internalUninstallComponent (uninstallerId, chassisSlot, &containerTarget, allowOverload);
}

//----------------------------------------------------------------------

bool ShipObject::installComponentFromData(int chassisSlot, ShipComponentData const & shipComponentData)
{
	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (getChassisType ());
	if (shipChassis == nullptr)
	{
		DEBUG_WARNING (true, ("Ship [%s] chassis [%d] is invalid for installing component [%s]",
			getNetworkId().getValueString().c_str(), 
			static_cast<int>(getChassisType ()),
			shipComponentData.getDescriptor().getName().getString()));
		return false;
	}
	
	if (isSlotInstalled (chassisSlot))
	{
		DEBUG_WARNING (true, ("Ship [%s] chassis [%s] slot [%s] is already filled, cannot install [%s]",
			getNetworkId().getValueString().c_str(), 
			shipChassis->getName().getString(),
			ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlot)).c_str(),
			shipComponentData.getDescriptor().getName().getString()));
		return false;
	}

	int effectiveCompatibilitySlot = chassisSlot;
	// weapon slots beyond 7 wrap back to slot 0 for the purposes of compatibility checking on captial ships
	if (isCapitalShip() && chassisSlot >= ShipChassisSlotType::SCST_num_explicit_types && chassisSlot <= ShipChassisSlotType::SCST_weapon_last)
		effectiveCompatibilitySlot = ShipChassisSlotType::SCST_weapon_first+((chassisSlot-ShipChassisSlotType::SCST_weapon_first)&7);

	ShipChassisSlot const * const slot = shipChassis->getSlot (static_cast<ShipChassisSlotType::Type>(effectiveCompatibilitySlot));
	if (slot == nullptr)
	{
		DEBUG_WARNING (true, ("Ship [%s] chassis [%s] does not support slot [%s] for installing component [%s]",
			getNetworkId().getValueString().c_str(), 
			shipChassis->getName().getString(),
			ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlot)).c_str(),
			shipComponentData.getDescriptor().getName().getString()));
		return false;
	}
	
	if (!slot->canAcceptComponent(shipComponentData.getDescriptor()))
	{
		//this chassis check, in the warning, is to suppress output related to the npe tutorial falcon instance, as it seems "special"
		std::string chassis = shipChassis->getName().getString();

		DEBUG_WARNING ((chassis != "player_yt1300"), ("Component [%s], compat [%s] cannot be installed in ship [%s] chassis [%s], slot [%s], compats [%s].", 
			shipComponentData.getDescriptor().getName().getString(), 
			shipComponentData.getDescriptor().getCompatibility().getString(),
			getNetworkId().getValueString().c_str(), 
			chassis.c_str(),
			ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlot)).c_str(),
			slot->getCompatibilityString().c_str()));
		return false;
	}
	
	shipComponentData.writeDataToShip (chassisSlot, *this);

	return true;
}

//----------------------------------------------------------------------

bool ShipObject::pseudoInstallComponent(int chassisSlot, uint32 componentCrc)
{
	ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc (componentCrc);
	if (shipComponentDescriptor == nullptr)
	{
		WARNING (true, ("Invalid component name"));
		return false;
	}

	ShipComponentData * const shipComponentData = ShipComponentDataManager::create (*shipComponentDescriptor);

	if (shipComponentData == nullptr)
	{
		WARNING(true, ("ShipObject::pseudoInstallComponent invalid descriptor"));
		return false;
	}

	bool const retval = installComponentFromData(chassisSlot, *shipComponentData);

	delete shipComponentData;

	return retval;
}

//----------------------------------------------------------------------

ShipComponentData * ShipObject::createShipComponentData (int chassisSlot) const
{
	ShipComponentDescriptor const * shipComponentDescriptor = nullptr;

	uint32 const componentCrc = getComponentCrc (chassisSlot);
	if (componentCrc)
	{
		shipComponentDescriptor   = ShipComponentDescriptor::findShipComponentDescriptorByCrc (componentCrc);
		if (shipComponentDescriptor == nullptr)
		{
			WARNING (true, ("ShipObject::createShipComponentData failed for slot [%d] because component crc [%d] could not map to a component descriptor", chassisSlot, componentCrc));
			return nullptr;
		}
	}
	else
		return nullptr;

	ShipComponentData * const shipComponentData = ShipComponentDataManager::create (*shipComponentDescriptor);
	if (shipComponentData == nullptr)
	{
		WARNING (true, ("ShipObject::createShipComponentData failed for slot [%d] ship Component data could not be constructed", chassisSlot));
		delete(shipComponentData);
		return nullptr;
	}

	if (!shipComponentData->readDataFromShip (chassisSlot, *this))
		WARNING (true, ("ShipObject::createShipComponentData failed for read data from the ship"));

	return shipComponentData;
}

//----------------------------------------------------------------------

bool ShipObject::isSlotInstalled                          (int chassisSlot) const
{
	return getComponentCrc (chassisSlot) != 0;
}

//----------------------------------------------------------------------

bool ShipObject::isBoosterInstalled() const
{
	return isSlotInstalled(ShipChassisSlotType::SCST_booster);
}

//----------------------------------------------------------------------

bool ShipObject::isBoosterActive() const
{
	if (isBoosterInstalled())
	{
		return isComponentActive(ShipChassisSlotType::SCST_booster);
	}
	
	return false;
}

// ----------------------------------------------------------------------

void ShipObject::setBoosterActive(bool const active)
{
	if (isSlotInstalled(ShipChassisSlotType::SCST_booster))
		IGNORE_RETURN(setComponentActive(ShipChassisSlotType::SCST_booster, active));
}

//----------------------------------------------------------------------

float ShipObject::computeShipActualAccelerationRate       () const
{
	float rate = 0.0f;

	if (isSlotInstalled (ShipChassisSlotType::SCST_engine) && isComponentFunctional(ShipChassisSlotType::SCST_engine))
	{
		float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);
		rate = getEngineAccelerationRate () * efficiency;
	}
	
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster) && isBoosterActive () && isComponentFunctional(ShipChassisSlotType::SCST_booster))
	{
		float const boosterEfficiency = computeActualComponentEfficiencyGeneral (ShipChassisSlotType::SCST_booster);
		rate += getBoosterAcceleration () * boosterEfficiency;
	}
	
	//-- @todo: divide rate by mass

	rate = rate * m_movementPercent;
	
	return rate;
}

//----------------------------------------------------------------------

float ShipObject::computeShipActualDecelerationRate       () const
{
	float rate = 0.0f;

	float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);

	if (isSlotInstalled (ShipChassisSlotType::SCST_engine) && isComponentFunctional(ShipChassisSlotType::SCST_engine))
		rate = getEngineDecelerationRate () * efficiency;
	else
		rate = getShipDataEngine().m_engineDecelerationRate;

	//-- @todo: divide rate by mass

	rate = rate * m_movementPercent;
	
	return rate;
}

//----------------------------------------------------------------------

float ShipObject::computeSpeedRotationFactor() const
{
	//-- compute rotational factor
	float const engineSpeedRotationFactorBaseMax = getEngineSpeedRotationFactorMaximum();
	float const engineSpeedRotationFactorBaseMin = getEngineSpeedRotationFactorMinimum();
	float const engineSpeedRotationFactorOptimal = getEngineSpeedRotationFactorOptimal();

	float const speedMax = getShipActualSpeedMaximum();
	float const speedCur = getCurrentSpeed();
	float const speedPercent = (speedMax > 0.0f) ? (speedCur / speedMax) : 0.0f;
	float engineSpeedRotationFactor = 1.0f;
	
	if (speedPercent < engineSpeedRotationFactorOptimal || engineSpeedRotationFactorOptimal >= 1.0f)
	{
		float const speedPercentToOptimal = speedPercent / engineSpeedRotationFactorOptimal;
		engineSpeedRotationFactor = linearInterpolate(engineSpeedRotationFactorBaseMin, 1.0f, speedPercentToOptimal);
	}
	else
	{
		float const speedPercentFromOptimal = (speedPercent - engineSpeedRotationFactorOptimal) / (1.0f - engineSpeedRotationFactorOptimal);
		engineSpeedRotationFactor = linearInterpolate(1.0f, engineSpeedRotationFactorBaseMax, speedPercentFromOptimal);
	}

	// snap to discreet values, to avoid constant change
	return floorf((engineSpeedRotationFactor * 10.f) + 0.5f) / 10.f;
}

//----------------------------------------------------------------------

float ShipObject::computeShipActualPitchAccelerationRate  () const
{
	//float rate = 0.0f;
	//
	//float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);
	//
	//if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
	//{
	//	rate = getEnginePitchAccelerationRate () * efficiency;
	//
	//	if (isComponentDisabled(ShipChassisSlotType::SCST_engine))
	//	{
	//		//-- player ships are allowed to stop spinning
	//		if (isPlayerShip())
	//			rate *= 0.5f;
	//		else
	//			rate = 0.0f;
	//	}
	//}
	float rate = 0.0f;

	float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);

	if (isSlotInstalled (ShipChassisSlotType::SCST_engine) && isComponentFunctional(ShipChassisSlotType::SCST_engine))
		rate = getEnginePitchAccelerationRate () * efficiency;
	else
		rate = getShipDataEngine().m_enginePitchAccelerationRate;

	//-- @todo: divide rate by rotational inertia

	return rate;
}

//----------------------------------------------------------------------

float ShipObject::computeShipActualYawAccelerationRate    () const
{
	//float rate = 0.0f;
	//
	//float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);
	//
	//if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
	//{
	//	rate = getEngineYawAccelerationRate () * efficiency;
	//
	//	if (isComponentDisabled(ShipChassisSlotType::SCST_engine))
	//	{
	//		//-- player ships are allowed to stop spinning
	//		if (isPlayerShip())
	//			rate *= 0.5f;
	//		else
	//			rate = 0.0f;
	//	}
	//}
	float rate = 0.0f;

	float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);

	if (isSlotInstalled (ShipChassisSlotType::SCST_engine) && isComponentFunctional(ShipChassisSlotType::SCST_engine))
		rate = getEngineYawAccelerationRate () * efficiency;
	else
		rate = getShipDataEngine().m_engineYawAccelerationRate;

	//-- @todo: divide rate by rotational inertia

	return rate;
}

//----------------------------------------------------------------------

float ShipObject::computeShipActualRollAccelerationRate   () const
{
	//float rate = 0.0f;
	//
	//float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);
	//
	//if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
	//{
	//	rate = getEngineRollAccelerationRate () * efficiency;
	//
	//	if (isComponentDisabled(ShipChassisSlotType::SCST_engine))
	//	{
	//		//-- player ships are allowed to stop spinning
	//		if (isPlayerShip())
	//			rate *= 0.5f;
	//		else
	//			rate = 0.0f;
	//	}
	//}

	float rate = 0.0f;

	float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);

	if (isSlotInstalled (ShipChassisSlotType::SCST_engine) && isComponentFunctional(ShipChassisSlotType::SCST_engine))
		rate = getEngineRollAccelerationRate () * efficiency;
	else
		rate = getShipDataEngine().m_engineRollAccelerationRate;

	//-- @todo: divide rate by rotational inertia

	return rate;
}

//----------------------------------------------------------------------

float ShipObject::computeShipActualPitchRateMaximum       () const
{
	//float rate = 0.0f;
	//
	//float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);
	//
	//if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
	//{
	//	rate = getEnginePitchRateMaximum () * efficiency;
	//
	//	if (isComponentDisabled(ShipChassisSlotType::SCST_engine))
	//	{
	//		//-- player ships have zero max rotation rates so the ship will come to a stop
	//		if (isPlayerShip())
	//			rate = 0.0f;
	//	}
	//}
	float rate = 0.0f;

	float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);

	if (isSlotInstalled (ShipChassisSlotType::SCST_engine) && isComponentFunctional(ShipChassisSlotType::SCST_engine))
		rate = getEnginePitchRateMaximum () * efficiency;

	rate *= computeSpeedRotationFactor();

	return rate;
}

//----------------------------------------------------------------------

float ShipObject::computeShipActualYawRateMaximum         () const
{
	//float rate = 0.0f;
	//
	//float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);
	//
	//if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
	//{
	//	rate = getEngineYawRateMaximum () * efficiency;
	//
	//	if (isComponentDisabled(ShipChassisSlotType::SCST_engine))
	//	{
	//		//-- player ships have zero max rotation rates so the ship will come to a stop
	//		if (isPlayerShip())
	//			rate = 0.0f;
	//	}
	//}
	float rate = 0.0f;

	float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);

	if (isSlotInstalled (ShipChassisSlotType::SCST_engine) && isComponentFunctional(ShipChassisSlotType::SCST_engine))
		rate = getEngineYawRateMaximum () * efficiency;

	rate *= computeSpeedRotationFactor();

	return rate;
}

//----------------------------------------------------------------------

float ShipObject::computeShipActualRollRateMaximum        () const
{
	//float rate = 0.0f;
	//
	//float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);
	//
	//if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
	//{
	//	rate = getEngineRollRateMaximum () * efficiency;
	//
	//	if (isComponentDisabled(ShipChassisSlotType::SCST_engine))
	//	{
	//		//-- player ships have zero max rotation rates so the ship will come to a stop
	//		if (isPlayerShip())
	//			rate = 0.0f;
	//	}
	//}
	float rate = 0.0f;

	float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);

	if (isSlotInstalled (ShipChassisSlotType::SCST_engine) && isComponentFunctional(ShipChassisSlotType::SCST_engine))
		rate = getEngineRollRateMaximum () * efficiency;

	return rate;
}

//----------------------------------------------------------------------

float ShipObject::computeShipActualSpeedMaximum           () const
{
	float rate = 0.0f;

	if (isSlotInstalled (ShipChassisSlotType::SCST_engine) && isComponentFunctional(ShipChassisSlotType::SCST_engine))
	{
		float const efficiency = computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine);
		rate = getEngineSpeedMaximum () * efficiency;
	}

	if (isSlotInstalled(ShipChassisSlotType::SCST_booster) && isBoosterActive() && isComponentFunctional(ShipChassisSlotType::SCST_booster))
		rate += getBoosterSpeedMaximum () * computeActualComponentEfficiencyGeneral(ShipChassisSlotType::SCST_booster);
	
	if (hasWings() && hasCondition(TangibleObject::C_wingsOpened))
	{
		ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc(getChassisType());
		if (nullptr != shipChassis)			
		{
			float const wingOpenSpeedFactor = shipChassis->getWingOpenSpeedFactor();
			rate *= wingOpenSpeedFactor;
		}
	}

	rate *= m_chassisSpeedMaximumModifier.get();

	rate = std::min(512.0f, rate * m_movementPercent);
	
	return rate;
}

//----------------------------------------------------------------------

float ShipObject::getShipActualAccelerationRate       () const
{
	return m_shipActualAccelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualDecelerationRate       () const
{
	return m_shipActualDecelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualPitchAccelerationRate  () const
{
	return m_shipActualPitchAccelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualYawAccelerationRate    () const
{
	return m_shipActualYawAccelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualRollAccelerationRate   () const
{
	return m_shipActualRollAccelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualPitchRateMaximum       () const
{
	return m_shipActualPitchRateMaximum.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualYawRateMaximum         () const
{
	return m_shipActualYawRateMaximum.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualRollRateMaximum        () const
{
	return m_shipActualRollRateMaximum.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualYprRateMaximum() const
{
	return std::max(std::max(m_shipActualPitchRateMaximum.get(), m_shipActualYawRateMaximum.get()), m_shipActualRollRateMaximum.get());
}

//----------------------------------------------------------------------

float ShipObject::getShipActualSpeedMaximum           () const
{
	return m_shipActualSpeedMaximum.get();
}

//----------------------------------------------------------------------

bool ShipObject::isComponentDisabled                      (int chassisSlot) const
{
	return (getComponentFlags (chassisSlot) & static_cast<int>(ShipComponentFlags::F_disabled)) != 0;
}

//----------------------------------------------------------------------

bool ShipObject::isComponentDisabledNeedsPower(int chassisSlot) const
{
	return (getComponentFlags (chassisSlot) & static_cast<int>(ShipComponentFlags::F_disabledNeedsPower)) != 0;
}

//----------------------------------------------------------------------

bool ShipObject::isComponentLowPower                      (int chassisSlot) const
{
	return (getComponentFlags (chassisSlot) & static_cast<int>(ShipComponentFlags::F_lowPower)) != 0;
}

//----------------------------------------------------------------------

bool ShipObject::isComponentActive(int chassisSlot) const
{
	return (getComponentFlags (chassisSlot) & static_cast<int>(ShipComponentFlags::F_active)) != 0;
}

//----------------------------------------------------------------------

bool ShipObject::isComponentDemolished(int chassisSlot) const
{
	return (getComponentFlags(chassisSlot) & ShipComponentFlags::F_demolished) != 0;
}

//----------------------------------------------------------------------

void ShipObject::handlePowerPulse (float timeElapsedSecs)
{
	PROFILER_AUTO_BLOCK_DEFINE("ShipObject::handlePowerPulse");

	DEBUG_FATAL (!isAuthoritative(), ("ShipObject::handlePowerPulse called on non-auth ship object"));

	//-- handle componentized power system

	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (getChassisType ());
	if (!shipChassis)
		return;

	CreatureObject * const pilot = getPilot();

	float powerGenerated = 0.0f;

	if (isSlotInstalled (static_cast<int>(ShipChassisSlotType::SCST_reactor)) && isComponentFunctional(ShipChassisSlotType::SCST_reactor))
	{
		powerGenerated = getReactorEnergyGenerationRate () * computeActualComponentEfficiencyGeneral (static_cast<int>(ShipChassisSlotType::SCST_reactor)) * timeElapsedSecs;
		powerGenerated += (powerGenerated * m_nebulaEffectReactor);
		powerGenerated = std::max (0.0f, powerGenerated);
	}

	//-- discharge the capacitor if it is disabled (because the power logic below is skipped for disabled components)
	if (isSlotInstalled(ShipChassisSlotType::SCST_capacitor) && !isComponentFunctional(ShipChassisSlotType::SCST_capacitor))
		IGNORE_RETURN(setCapacitorEnergyCurrent(0.0f));
	
	for (int i = static_cast<int>(ShipChassisSlotType::SCST_first); i < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++i)
	{
		ShipChassisSlotType::Type const slotType = shipChassis->getSlotPowerPriority (i);
		if (slotType == ShipChassisSlotType::SCST_num_types || slotType == ShipChassisSlotType::SCST_reactor)
			continue;

		int const slotIndex = static_cast<int>(slotType);

		if (!isSlotInstalled (slotIndex))
			continue;

		if (isComponentDemolished(slotIndex))
			continue;

		if (isComponentDisabled(slotIndex))
		{			
			if (!isComponentDisabledNeedsPower(slotIndex))
				continue;
		}

		float const baseComponentEnergyRequired = getComponentEnergyMaintenanceRequirement (slotIndex) * timeElapsedSecs;

		float oo_componentEfficiencyEnergy   = getComponentEfficiencyEnergy (slotIndex);
		if (oo_componentEfficiencyEnergy > 0.0f)
			oo_componentEfficiencyEnergy = RECIP (oo_componentEfficiencyEnergy);

		float const energyRequired = baseComponentEnergyRequired * oo_componentEfficiencyEnergy;

		//-- first consume the base energy requirement for the component

		float energyInsufficiencyPercent = 0.0f;

		//-- insufficient power
		if (energyRequired > powerGenerated)
		{
			if(getScriptObject())
			{
				ScriptParams p;
				p.addParam (slotIndex);
				p.addParam (energyRequired);
				p.addParam (powerGenerated);

				IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_COMPONENT_POWER_INSUFFICIENT, p));
				
				//-- script has for some reason removed the component, stop processing it
				if (!isSlotInstalled(slotIndex))
					continue;
			}

			//-- energy req cannot be zero
			energyInsufficiencyPercent = 1.0f - powerGenerated / energyRequired;
			IGNORE_RETURN(setComponentLowPower (slotIndex, true));
		}
		//-- sufficient power
		else
		{
			if (isComponentLowPower (slotIndex))
			{
				if(getScriptObject())
				{
					ScriptParams p;
					p.addParam (slotIndex);
					p.addParam (energyRequired);
					
					IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_SHIP_COMPONENT_POWER_SUFFICIENT, p));
					
					//-- script has for some reason removed the component, stop processing it
					if (!isSlotInstalled(slotIndex))
						continue;					
				}

				IGNORE_RETURN(setComponentLowPower (slotIndex, false));
			}
		}

		powerGenerated = std::max (0.0f, powerGenerated - energyRequired);

		//-- 
		if (isComponentDisabled(slotIndex))
			continue;

		float const efficiencyGeneral = computeActualComponentEfficiencyGeneral(slotIndex);

		switch (slotType)
		{
			//--
			//-- recharge or decay the shields
			//--

		case ShipChassisSlotType::SCST_shield_0:
			{
				float const hpFrontCurrent = getShieldHitpointsFrontCurrent   ();
				float const hpFrontMaximum = getShieldHitpointsFrontMaximum   ();
				float const hpBackCurrent  = getShieldHitpointsBackCurrent    ();
				float const hpBackMaximum  = getShieldHitpointsBackMaximum    ();

				//-- recharge the shields
				if (!isComponentLowPower(slotIndex))
				{
					float shieldRechargeRateHps = getShieldRechargeRate () * efficiencyGeneral * timeElapsedSecs;
					shieldRechargeRateHps += (shieldRechargeRateHps * m_nebulaEffectShields);

					if (shieldRechargeRateHps > 0.0f)
					{
						float const hpFrontNeeded = std::max (0.0f, std::min (hpFrontMaximum - hpFrontCurrent, shieldRechargeRateHps));
						float const hpBackNeeded  = std::max (0.0f, std::min (hpBackMaximum - hpBackCurrent,   shieldRechargeRateHps));

						if (hpFrontNeeded > 0.0f || hpBackNeeded > 0.0f)
						{							
							if (hpFrontNeeded)
								IGNORE_RETURN(setShieldHitpointsFrontCurrent(hpFrontCurrent + hpFrontNeeded));
							if (hpBackNeeded)
								IGNORE_RETURN(setShieldHitpointsBackCurrent(hpBackCurrent + hpBackNeeded));
						}
					}
				}
				//-- decay the shields to some percentage
				else if (energyInsufficiencyPercent > 0.0f)
				{
					float const shieldDecayRateHps = std::max (1.0f, (getShieldRechargeRate () * timeElapsedSecs) * energyInsufficiencyPercent);

					float const minShieldsFront = (1.0f - energyInsufficiencyPercent) * getShieldHitpointsFrontMaximum();
					float const minShieldsBack = (1.0f - energyInsufficiencyPercent) * getShieldHitpointsBackMaximum();

					if (hpFrontCurrent > minShieldsFront)
						IGNORE_RETURN(setShieldHitpointsFrontCurrent(std::max(minShieldsFront, hpFrontCurrent - shieldDecayRateHps)));
					if (hpBackCurrent > minShieldsBack)
						IGNORE_RETURN(setShieldHitpointsBackCurrent(std::max(minShieldsFront, hpBackCurrent - shieldDecayRateHps)));
				}
			}
			break;

			//--
			//-- recharge or decay the capacitor
			//--

		case ShipChassisSlotType::SCST_capacitor:
			{
				float       capacitorEnergyCurrent = getCapacitorEnergyCurrent ();
				float const capacitorEnergyMaximum = getCapacitorEnergyEffectiveMaximum () * getComponentEfficiencyGeneral(ShipChassisSlotType::SCST_capacitor);

				//-- the efficiency might cause the current charge to exceed the max
				if (capacitorEnergyCurrent > capacitorEnergyMaximum)
				{
					IGNORE_RETURN(setCapacitorEnergyCurrent (capacitorEnergyMaximum));
					capacitorEnergyCurrent = getCapacitorEnergyCurrent ();
				}

				//-- recharge the capacitor
				if (!isComponentLowPower(slotIndex))
				{
					float const rechargeRate = getCapacitorEnergyRechargeRate () * efficiencyGeneral * timeElapsedSecs;
					float const capacitorEnergyRechargeNeeded = std::min (capacitorEnergyMaximum - capacitorEnergyCurrent, rechargeRate);

					if (capacitorEnergyRechargeNeeded > 0.0f)
					{						
						IGNORE_RETURN(setCapacitorEnergyCurrent (capacitorEnergyCurrent + capacitorEnergyRechargeNeeded));
					}
				}
				//-- decay the capacitors to some percentage
				else if (energyInsufficiencyPercent > 0.0f)
				{
					float const decayRate = std::max (1.0f, (getCapacitorEnergyRechargeRate () * timeElapsedSecs) * energyInsufficiencyPercent);

					float const minLevel = (1.0f - energyInsufficiencyPercent) * getCapacitorEnergyEffectiveMaximum();

					if (capacitorEnergyCurrent > minLevel)
						IGNORE_RETURN(setCapacitorEnergyCurrent (std::max(minLevel, capacitorEnergyCurrent - decayRate)));
				}
			}
			break;

			//--
			//-- recharge or decay the booster
			//--

		case ShipChassisSlotType::SCST_booster:
			{
				float boosterEnergyCurrent   = getBoosterEnergyCurrent();

				if (isBoosterActive())
				{
					float const boosterEnergyUsed = (getBoosterEnergyConsumptionRate() * timeElapsedSecs) * oo_componentEfficiencyEnergy;

					IGNORE_RETURN(setBoosterEnergyCurrent(std::max(0.0f, boosterEnergyCurrent - boosterEnergyUsed)));
					boosterEnergyCurrent   = getBoosterEnergyCurrent();

					//-- when the booster runs out of energy, kill it
					if (boosterEnergyCurrent <= 0.0f)
					{
						IGNORE_RETURN(setComponentActive(ShipChassisSlotType::SCST_booster, false));
						if (pilot != nullptr)
							Chat::sendSystemMessage(*pilot, SharedStringIds::booster_energy_depleted, Unicode::emptyString);

						restartBoosterTimer();
					}
					break;
				}
				
				float const boosterEnergyMaximum   = getBoosterEnergyMaximum   ();
				
				//recharge the booster
				if (!isComponentLowPower(slotIndex))
				{
					float const rechargeRate = getBoosterEnergyRechargeRate () * efficiencyGeneral * timeElapsedSecs;
					float const boosterEnergyRechargeNeeded = std::min (boosterEnergyMaximum - boosterEnergyCurrent, rechargeRate);
					
					if (boosterEnergyRechargeNeeded > 0.0f)
					{
						
						float const boosterEnergyRequired = boosterEnergyRechargeNeeded * oo_componentEfficiencyEnergy;
												
						IGNORE_RETURN(setBoosterEnergyCurrent (boosterEnergyCurrent + boosterEnergyRequired));
					}
				}
				//-- decay the boosters to some percentage
				else if (energyInsufficiencyPercent > 0.0f)
				{
					float const decayRate = std::max (1.0f, (getBoosterEnergyRechargeRate () * timeElapsedSecs) * energyInsufficiencyPercent);
					
					float const minLevel = (1.0f - energyInsufficiencyPercent) * getBoosterEnergyMaximum();
					
					if (boosterEnergyCurrent > minLevel)
						IGNORE_RETURN(setBoosterEnergyCurrent (std::max(minLevel, boosterEnergyCurrent - decayRate)));
				}
			}
			break;

		default:
			break;

		} //lint !e788 //switch cases not used
	}
}

//----------------------------------------------------------------------

int ShipObject::getProjectileIndexForWeapon(int const weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return -1;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_0));

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
	if (componentCrc == 0)
		return -1;

	return ShipComponentWeaponManager::getProjectileIndex(componentCrc);
}

//----------------------------------------------------------------------

float ShipObject::getWeaponRange(int weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return 0.0f;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_0));

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
	if (componentCrc == 0)
		return 0.0f;

	float range = 0.0f;

	if (ShipComponentWeaponManager::isMissile(componentCrc))
	{
		int const missileTypeId = static_cast<int>(getWeaponAmmoType(weaponChassisSlotType));
		range = MissileManager::getInstance().getRangeByMissileType(missileTypeId);
	}
	else
	{
		range = ShipComponentWeaponManager::getRange(componentCrc);
	}

	return range;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponProjectileSpeed(int weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return 0.0f;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_0));

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
	if (componentCrc == 0)
		return 0.0f;

	float speed = 0.0f;
	
	if (ShipComponentWeaponManager::isMissile(componentCrc))
	{
		int const missileTypeId = static_cast<int>(getWeaponAmmoType(weaponChassisSlotType));
		speed = MissileManager::getInstance().getSpeedByMissileType(missileTypeId);
	}
	else
	{
		speed = ShipComponentWeaponManager::getProjectileSpeed(componentCrc);
	}

	return speed;
}
//----------------------------------------------------------------------

float ShipObject::computeWeaponProjectileTimeToLive(int weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return 0.0f;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_0));

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
	if (componentCrc == 0)
		return 0.0f;

	float timeToLive = 0.0f;
	
	if (ShipComponentWeaponManager::isMissile(componentCrc))
	{
		int const missileTypeId = static_cast<int>(getWeaponAmmoType(weaponChassisSlotType));
		timeToLive = MissileManager::getInstance().getMaxTimeByMissileType(missileTypeId);
	}
	else
	{
		float const range = ShipComponentWeaponManager::getRange(componentCrc);
		float const projectileSpeed = ShipComponentWeaponManager::getProjectileSpeed(componentCrc);
		timeToLive = (projectileSpeed <= 0.0f) ? 0.0f : range / projectileSpeed;
	}

	return timeToLive;
}

//----------------------------------------------------------------------

float ShipObject::computeActualComponentEfficiencyGeneral(int chassisType) const
{
	float efficiency = getComponentEfficiencyGeneral(chassisType);
	float nebulaEffect = 0.0f;

	switch (chassisType)
	{
	case ShipChassisSlotType::SCST_engine:
		nebulaEffect = m_nebulaEffectEngine;
		break;
	case ShipChassisSlotType::SCST_reactor:
		nebulaEffect = m_nebulaEffectReactor;
		break;
	case ShipChassisSlotType::SCST_shield_0:
		nebulaEffect = m_nebulaEffectShields;
		break;
	default:
		break;
	}

	efficiency = clamp(0.1f, efficiency + nebulaEffect, 10.0f);
	return efficiency;
}

//----------------------------------------------------------------------

void ShipObject::setSlotTargetable(ShipChassisSlotType::Type slot)
{
	BitArray tmp(m_targetableSlotBitfield.get());
	tmp.setBit(slot);
	m_targetableSlotBitfield = tmp;
}

//----------------------------------------------------------------------

void ShipObject::clearSlotTargetable(ShipChassisSlotType::Type slot)
{
	BitArray tmp(m_targetableSlotBitfield.get());
	tmp.clearBit(slot);
	m_targetableSlotBitfield = tmp;
}

//----------------------------------------------------------------------

bool ShipObject::getSlotTargetable(ShipChassisSlotType::Type slot) const
{
	return m_targetableSlotBitfield.get().testBit(slot);
}

//----------------------------------------------------------------------

bool ShipObject::isValidTargetableSlot (ShipChassisSlotType::Type slot) const
{
	if(getSlotTargetable(slot))
	{
		if(isSlotInstalled(slot))
		{
			if(isComponentFunctional(slot))
			{
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------

void ShipObject::setPilotLookAtTargetSlot(ShipChassisSlotType::Type slot)
{
	if(isAuthoritative() && isValidTargetableSlot(slot))
	{
		m_pilotLookAtTargetSlot.set(slot);
	}
}

//----------------------------------------------------------------------

void ShipObject::setTurretTarget(int weaponIndex, CachedNetworkId const & target)
{
	if (isAuthoritative())
	{
		DEBUG_FATAL(!isTurret(weaponIndex), ("Assigning a target(%s) to a weaponIndex(%d) that is not a turret", target.getValueString().c_str(), weaponIndex));

		if (!target.isValid())
			IGNORE_RETURN(m_turretTargets.erase(weaponIndex));
		else
			m_turretTargets[weaponIndex] = target;
	}
}

//----------------------------------------------------------------------

CachedNetworkId const & ShipObject::getTurretTarget(int weaponIndex) const
{
	if (isAuthoritative())
	{
		DEBUG_FATAL(!isTurret(weaponIndex), ("Requesting the target of an weaponIndex(%d) that is not a turret", weaponIndex));

		std::map<int, CachedNetworkId>::const_iterator const it = m_turretTargets.find(weaponIndex);
		if (it != m_turretTargets.end())
			return (*it).second;
	}
	return CachedNetworkId::cms_cachedInvalid;
}

//----------------------------------------------------------------------

Transform const ShipObject::getTurretTransform(int weaponIndex) const
{
	int const chassisSlot = static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex;
	uint32 const componentCrc = getComponentCrc(chassisSlot);

	if (componentCrc != 0)
	{
		AppearanceTemplate const * exteriorAppearanceTemplate = 0;

		PortalProperty const * const portalProperty = getPortalProperty();
		if (portalProperty)
		{
			char const * const exteriorAppearanceName = portalProperty->getPortalPropertyTemplate().getExteriorAppearanceName();
			if (exteriorAppearanceName)
				exteriorAppearanceTemplate = AppearanceTemplateList::fetch(exteriorAppearanceName);
		}
		else
		{
			Appearance const * const appearance = getAppearance();
			if (appearance)
				exteriorAppearanceTemplate = appearance->getAppearanceTemplate();
		}

		if (exteriorAppearanceTemplate)
		{
			ShipComponentAttachmentManager::TemplateHardpointPairVector const &attachments = ShipComponentAttachmentManager::getAttachmentsForShip(getChassisType(), componentCrc, chassisSlot);
			if (!attachments.empty())
			{
				ShipComponentAttachmentManager::TemplateHardpointPair const &templateHardpointPair = *attachments.begin();
				int const turretYawHardpointIndex = exteriorAppearanceTemplate->getHardpointIndex(templateHardpointPair.second, true);
				WARNING(turretYawHardpointIndex == -1, ("Ship %s references hardpoint '%s' which could not be found.", getDebugInformation().c_str(), templateHardpointPair.second.getString()));
				if (turretYawHardpointIndex != -1)
				{
					Hardpoint const &turretYawHardpoint = exteriorAppearanceTemplate->getHardpoint(turretYawHardpointIndex);
					// we now need to find a weapon hardpoint on this attachment
					SharedObjectTemplate const * const objectTemplate = safe_cast<SharedObjectTemplate const *>(ObjectTemplateList::fetch(templateHardpointPair.first));
					if (objectTemplate)
					{
						AppearanceTemplate const * const appearanceTemplate = AppearanceTemplateList::fetch(objectTemplate->getAppearanceFilename().c_str());
						if (appearanceTemplate)
						{
							int const turretPitchHardpointIndex = appearanceTemplate->getHardpointIndex(s_turretPitchHardpoint, true);
							if (turretPitchHardpointIndex != -1)
							{
								Hardpoint const &turretPitchHardpoint = appearanceTemplate->getHardpoint(turretPitchHardpointIndex);
								return getTransform_o2p().rotateTranslate_l2p(turretYawHardpoint.getTransform().rotateTranslate_l2p(turretPitchHardpoint.getTransform()));
							}
						}
					}
					// we couldn't find a turret pitch hardpoint, so the turret yaw hardpoint is for the object which pitches also
					return getTransform_o2p().rotateTranslate_l2p(turretYawHardpoint.getTransform());
				}
			}
		}
	}

	// we couldn't find an attachment hardpoint, so assume we fire forward from the ship center
	return getTransform_o2p();
}

//----------------------------------------------------------------------

bool ShipObject::isMissile(int const weaponIndex) const
{
	bool ismissile = false;

	if(weaponIndex >= 0 && weaponIndex < ShipChassisSlotType::cms_numWeaponIndices)
	{
		int const chassisSlot = static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + weaponIndex;
		uint32 const componentCrc = getComponentCrc(chassisSlot);
		if (componentCrc != 0)
		{
			ismissile = ShipComponentWeaponManager::isMissile(componentCrc);
		}
	}

	return ismissile;
}

//----------------------------------------------------------------------

bool ShipObject::isTurret(int const weaponIndex) const
{
	DEBUG_FATAL(!ShipChassisSlotType::isWeaponIndex(weaponIndex), ("weaponIndex(%d) is out of range [0...%d]", weaponIndex, ShipChassisSlotType::cms_numWeaponIndices - 1));

	return m_turretWeaponIndices->testBit(weaponIndex);
}

//----------------------------------------------------------------------

bool ShipObject::isCountermeasure(int const weaponIndex) const
{
	ShipChassisSlotType::Type const shipChassisSlotType = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);
	if (isSlotInstalled(shipChassisSlotType))
	{
		uint32 const componentCrc = getComponentCrc(shipChassisSlotType);
		return ShipComponentWeaponManager::isCountermeasure(componentCrc);
	}
	return false;
}

//----------------------------------------------------------------------

bool ShipObject::isProjectile(int const weaponIndex) const
{
	bool missile = isMissile(weaponIndex);
	bool turret = isTurret(weaponIndex);
	bool countermeasure = isCountermeasure(weaponIndex);

	return (!missile && !turret && !countermeasure);
}

//----------------------------------------------------------------------

float ShipObject::computeChassisComponentMassCurrent() const
{
	float massCurrent = 0.0f;

	for (int chassisSlot = static_cast<int>(ShipChassisSlotType::SCST_first); chassisSlot < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlot)
	{
		if (!isSlotInstalled (chassisSlot))
			continue;

		massCurrent += getComponentMass(chassisSlot);
	}

	return massCurrent;
}

//----------------------------------------------------------------------

float ShipObject::getChassisComponentMassMaximum() const
{
	return m_chassisComponentMassMaximum.get();
}

//----------------------------------------------------------------------

float ShipObject::getChassisComponentMassCurrent() const
{
	return m_chassisComponentMassCurrent.get();
}

//----------------------------------------------------------------------

void ShipObject::setChassisComponentMassMaximum(float massMaximum)
{
	m_chassisComponentMassMaximum.set(massMaximum);
}

//----------------------------------------------------------------------

void ShipObject::internalSetChassisComponentMassCurrent(float massCurrent)
{
	m_chassisComponentMassCurrent.set(massCurrent);
}

// ----------------------------------------------------------------------

/**
 * Check whether a turret can fire towards a particular location without
 * exceeding its rotation limits.
 */
bool ShipObject::canTurretFireTowardsLocation_p(int weaponIndex, Vector const & targetLocation_p) const
{
	Transform turretTransform_o2p(getTurretTransform(weaponIndex));
	
	uint32 const chassisCrc = getChassisType();
	float const minYaw = ShipTurretManager::getTurretMinYaw(chassisCrc, weaponIndex);
	float const maxYaw = ShipTurretManager::getTurretMaxYaw(chassisCrc, weaponIndex);
	float const minPitch = ShipTurretManager::getTurretMinPitch(chassisCrc, weaponIndex);
	float const maxPitch = ShipTurretManager::getTurretMaxPitch(chassisCrc, weaponIndex);

	Vector const turretToTarget = turretTransform_o2p.rotateTranslate_p2l(targetLocation_p);
	
	float yaw=turretToTarget.theta();
	float pitch=turretToTarget.phi();

	// if yaw and pitch are limited for this turret, check that the desired yaw/pitch is within the range

	if ((maxYaw-minYaw)+0.001 < PI_TIMES_2)
		if (yaw > maxYaw || yaw < minYaw)
			return false;

	if ((maxPitch-minPitch)+0.001 < PI_TIMES_2)
		if (pitch > maxPitch || pitch < minPitch)
			return false;
	
	return true;
}

// ----------------------------------------------------------------------

/**
 * Given a position on the ship, figure out which targettable chassis slot covers
 * that position.  Used to determine which component a projectile hit.
 */
int ShipObject::findTargetChassisSlotByPosition(Vector const & position_o) const
{
	DEBUG_FATAL(!isCapitalShip(),("Programmer bug:  findChassisSlotByPosition() should only be called on capital ships"));

	AppearanceTemplate const * exteriorAppearanceTemplate = 0;

	PortalProperty const * const portalProperty = getPortalProperty();
	if (portalProperty)
	{
		char const * const exteriorAppearanceName = portalProperty->getPortalPropertyTemplate().getExteriorAppearanceName();
		if (exteriorAppearanceName)
			exteriorAppearanceTemplate = AppearanceTemplateList::fetch(exteriorAppearanceName);
	}
	else
	{
		Appearance const * const appearance = getAppearance();
		if (appearance)
			exteriorAppearanceTemplate = appearance->getAppearanceTemplate();
	}
	NOT_NULL(exteriorAppearanceTemplate);
	if (!exteriorAppearanceTemplate) //lint !e774 only always false in debug mode
		return ShipChassisSlotType::SCST_invalid;

	ShipChassisSlotType::Type closestSlot = ShipChassisSlotType::SCST_invalid;
	float closestDistance = 0.0f;
	
	for (int chassisSlotIterator = static_cast<int>(ShipChassisSlotType::SCST_first); chassisSlotIterator != static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotIterator)
	{
		ShipChassisSlotType::Type chassisSlot = static_cast<ShipChassisSlotType::Type>(chassisSlotIterator);
		uint32 const componentCrc = getComponentCrc(chassisSlot);

		//DEBUG_REPORT_LOG(componentCrc != 0 && !getSlotTargetable(chassisSlot),("Skipping slot %i because it is not targettable\n",chassisSlot));
		
		if (componentCrc != 0 && getSlotTargetable(chassisSlot))
		{
			float hitRange = ShipComponentAttachmentManager::getComponentHitRange(getChassisType(), componentCrc, chassisSlot);
			{
				// First look at regular attachments
				ShipComponentAttachmentManager::TemplateHardpointPairVector const &attachments = ShipComponentAttachmentManager::getAttachmentsForShip(getChassisType(), componentCrc, chassisSlot);
				for (ShipComponentAttachmentManager::TemplateHardpointPairVector::const_iterator i=attachments.begin(); i!=attachments.end(); ++i)
				{
					ShipComponentAttachmentManager::TemplateHardpointPair const &templateHardpointPair = *i;
				
					int const hardpointIndex = exteriorAppearanceTemplate->getHardpointIndex(templateHardpointPair.second, true);
					WARNING(hardpointIndex == -1, ("Ship %s references hardpoint '%s' which could not be found.", getDebugInformation().c_str(), templateHardpointPair.second.getString()));
					if (hardpointIndex != -1)
					{
						Hardpoint const & hardpoint = exteriorAppearanceTemplate->getHardpoint(hardpointIndex);
						Transform const & transform_o = hardpoint.getTransform();
						float const distanceSquared = (position_o - transform_o.getPosition_p()).magnitudeSquared();
					
						if (distanceSquared < sqr(hitRange) &&
							((closestSlot == ShipChassisSlotType::SCST_invalid) || (distanceSquared < closestDistance)))
						{
							closestSlot = chassisSlot;
							closestDistance = distanceSquared;
						}
					}
				}
			}
			{
				// Second look at "extra" hardpoints (hardpoints that exist for the sake of targeting only
				ShipComponentAttachmentManager::HardpointVector const &extraHardpoints = ShipComponentAttachmentManager::getExtraHardpointsForComponent(getChassisType(), componentCrc, chassisSlot);
				for (ShipComponentAttachmentManager::HardpointVector::const_iterator i=extraHardpoints.begin(); i!=extraHardpoints.end(); ++i)
				{
					int const hardpointIndex = exteriorAppearanceTemplate->getHardpointIndex(*i, true);
					WARNING(hardpointIndex == -1, ("Ship %s references hardpoint '%s' which could not be found.", getDebugInformation().c_str(), (*i).getString()));
					if (hardpointIndex != -1)
					{
						Hardpoint const & hardpoint = exteriorAppearanceTemplate->getHardpoint(hardpointIndex);
						Transform const & transform_o = hardpoint.getTransform();
						float const distanceSquared = (position_o - transform_o.getPosition_p()).magnitudeSquared();
					
						if (distanceSquared < sqr(hitRange) &&
							((closestSlot == ShipChassisSlotType::SCST_invalid) || (distanceSquared < closestDistance)))
						{
							closestSlot = chassisSlot;
							closestDistance = distanceSquared;
						}
					}
				}
			}
		}
	}
	return closestSlot;
}

// ----------------------------------------------------------------------

void ShipObject::findTransformsForComponent(int const chassisSlot, std::vector<Transform> & transforms) const
{
	transforms.clear();
	AppearanceTemplate const * const exteriorAppearanceTemplate = getExteriorAppearance();
	if (!exteriorAppearanceTemplate)
		return;
	uint32 const componentCrc = getComponentCrc(chassisSlot);
	
	{
		// First look at regular attachments
		ShipComponentAttachmentManager::TemplateHardpointPairVector const &attachments = ShipComponentAttachmentManager::getAttachmentsForShip(getChassisType(), componentCrc, chassisSlot);
		for (ShipComponentAttachmentManager::TemplateHardpointPairVector::const_iterator i=attachments.begin(); i!=attachments.end(); ++i)
		{
			ShipComponentAttachmentManager::TemplateHardpointPair const &templateHardpointPair = *i;
				
			int const hardpointIndex = exteriorAppearanceTemplate->getHardpointIndex(templateHardpointPair.second, true);
			WARNING(hardpointIndex == -1, ("Ship %s references hardpoint '%s' which could not be found.", getDebugInformation().c_str(), templateHardpointPair.second.getString()));
			if (hardpointIndex != -1)
			{
				Hardpoint const & hardpoint = exteriorAppearanceTemplate->getHardpoint(hardpointIndex);
				Transform const & transform_o = hardpoint.getTransform();
				transforms.push_back(transform_o);
			}
		}
	}

	{
		// Second look at "extra" hardpoints (hardpoints that exist for the sake of targeting only
		ShipComponentAttachmentManager::HardpointVector const &extraHardpoints = ShipComponentAttachmentManager::getExtraHardpointsForComponent(getChassisType(), componentCrc, chassisSlot);
		for (ShipComponentAttachmentManager::HardpointVector::const_iterator i=extraHardpoints.begin(); i!=extraHardpoints.end(); ++i)
		{
			int const hardpointIndex = exteriorAppearanceTemplate->getHardpointIndex(*i, true);
			WARNING(hardpointIndex == -1, ("Ship %s references hardpoint '%s' which could not be found.", getDebugInformation().c_str(), (*i).getString()));
			if (hardpointIndex != -1)
			{
				Hardpoint const & hardpoint = exteriorAppearanceTemplate->getHardpoint(hardpointIndex);
				Transform const & transform_o = hardpoint.getTransform();
				transforms.push_back(transform_o);
			}
		}
	}
}

// ----------------------------------------------------------------------

AppearanceTemplate const * ShipObject::getExteriorAppearance() const
{
	AppearanceTemplate const * exteriorAppearanceTemplate = 0;

	PortalProperty const * const portalProperty = getPortalProperty();
	if (portalProperty)
	{
		char const * const exteriorAppearanceName = portalProperty->getPortalPropertyTemplate().getExteriorAppearanceName();
		if (exteriorAppearanceName)
			exteriorAppearanceTemplate = AppearanceTemplateList::fetch(exteriorAppearanceName);
	}
	else
	{
		Appearance const * const appearance = getAppearance();
		if (appearance)
			exteriorAppearanceTemplate = appearance->getAppearanceTemplate();
	}
	NOT_NULL(exteriorAppearanceTemplate);
	return exteriorAppearanceTemplate;
}
	
// ----------------------------------------------------------------------

float ShipObject::getFastestWeaponProjectileSpeed() const
{
	return m_fastestWeaponProjectileSpeed;
}

//----------------------------------------------------------------------

int ShipObject::getCargoHoldContentsMaximum() const
{
	return m_cargoHoldContentsMaximum.get();
}

//----------------------------------------------------------------------

bool ShipObject::setCargoHoldContentsMaximum(int contents)
{
	if (isSlotInstalled(static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)))
	{
		m_cargoHoldContentsMaximum = contents;
		setCargoHoldContentsCurrent(getCargoHoldContentsCurrent());
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

int ShipObject::getCargoHoldContentsCurrent() const
{
	return m_cargoHoldContentsCurrent.get();
}

//----------------------------------------------------------------------

bool ShipObject::setCargoHoldContentsCurrent(int contents)
{
	if (isSlotInstalled(static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)))
	{
		int const maximum = getCargoHoldContentsMaximum();
		m_cargoHoldContentsCurrent = std::min(contents, maximum);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

ShipObject::NetworkIdIntMap const & ShipObject::getCargoHoldContents() const
{
	return m_cargoHoldContents.getMap();
}

//----------------------------------------------------------------------

void ShipObject::setCargoHoldContents(NetworkIdIntMap const & contents)
{
	if (isSlotInstalled(static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)))
	{
		m_cargoHoldContents.clear();

		NetworkIdIntMap::const_iterator const end = contents.end();
		for (NetworkIdIntMap::const_iterator it = contents.begin(); it != end; ++it)
		{
			m_cargoHoldContents.insert(it->first, it->second);
		}

		setCargoHoldContentsCurrent(computeCargoHoldContentsCurrent());
	}
}

//----------------------------------------------------------------------

void ShipObject::setCargoHoldContent(NetworkId const & resourceTypeId, int amount)
{
	if (isSlotInstalled(static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)))
	{
		if (amount <= 0)
		{
			m_cargoHoldContents.erase(resourceTypeId);
			m_cargoHoldContentsResourceTypeInfo.erase(resourceTypeId);
		}
		else
		{
			ResourceTypeObject const * const resourceTypeObject = ServerUniverse::getInstance().getResourceTypeById(resourceTypeId);
			if (nullptr == ServerUniverse::getInstance().getResourceTypeById(resourceTypeId))
			{
				WARNING(true, ("ShipObject::setCargoHoldContent invalid resource type [%s]", resourceTypeId.getValueString().c_str()));
				return;
			}
			
			bool alreadyHasThisResource = getCargoHoldContent(resourceTypeId) > 0;

			if (!alreadyHasThisResource)
			{
				m_cargoHoldContentsResourceTypeInfo.set(
					resourceTypeId, 
					ResourceTypeInfoPair(Unicode::narrowToWide(resourceTypeObject->getResourceName()), resourceTypeObject->getParentClass().getResourceClassName()));
			}

			m_cargoHoldContents.set(resourceTypeId, amount);

		}

		setCargoHoldContentsCurrent(computeCargoHoldContentsCurrent());
	}
}

//----------------------------------------------------------------------

int ShipObject::getCargoHoldContent(NetworkId const & resourceTypeId) const
{
	if (isSlotInstalled(static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)))
	{
		NetworkIdIntMap::const_iterator const it = m_cargoHoldContents.find(resourceTypeId);
		if (it != m_cargoHoldContents.end())
		{
			return (*it).second;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

int ShipObject::computeCargoHoldContentsCurrent() const
{
	int total = 0;
	NetworkIdIntMap::const_iterator const end = m_cargoHoldContents.end();
	for (NetworkIdIntMap::const_iterator it = m_cargoHoldContents.begin(); it != end; ++it)
	{
		total += (*it).second;
	}

	return total;
}

//======================================================================
