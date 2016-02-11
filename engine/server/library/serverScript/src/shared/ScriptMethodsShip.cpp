// ======================================================================
//
// ScriptMethodsShip.cpp
// Copyright 2003, Sony Online Entertainment, Inc.
// All rights reserved.
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "UnicodeUtils.h"
#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/MiningAsteroidController.h"
#include "serverGame/Missile.h"
#include "serverGame/MissileManager.h"
#include "serverGame/Pvp.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipInternalDamageOverTime.h"
#include "serverGame/ShipInternalDamageOverTimeManager.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceVisibilityManager.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "sharedNetworkMessages/MessageQueueShipHit.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/Nebula.h"
#include "sharedGame/NebulaManager.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedNetworkMessages/DestroyShipComponentMessage.h"
#include "sharedNetworkMessages/DestroyShipMessage.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"

using namespace JNIWrappersNamespace;


// ======================================================================

namespace ScriptMethodsShipNamespace
{
	bool verifyShipsEnabled()
	{
		if (!ConfigServerGame::getShipsEnabled())
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("Ship support is not enabled.  Make sure [GameServer] shipsEnabled=1 is in your config file."));
			return false;
		}

		return true;
	}

	ShipChassisSlotType::Type getSlotType(int slotType)
	{
		if(slotType < ShipChassisSlotType::SCST_first || slotType > ShipChassisSlotType::SCST_num_types)
			return ShipChassisSlotType::SCST_num_types;
		else
			return static_cast<ShipChassisSlotType::Type>(slotType);
	}

	//----------------------------------------------------------------------

	ShipObject * const getShipThrowTestSlot(JNIEnv * env, jlong jobject_shipId, char const * const functionName, bool throwIfNotOnServer, int chassisSlot)
	{
		//-- Make sure ships are enabled
		if (!verifyShipsEnabled())
			return nullptr;

		char buf[256];
		snprintf(buf, sizeof(buf), "JavaLibrary::%s: ship obj_id did not resolve to a ShipObject", functionName);
		ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, buf, throwIfNotOnServer);
		if (!shipObject)
			return nullptr;

		if (chassisSlot != ShipChassisSlotType::SCST_num_types && !shipObject->isSlotInstalled(chassisSlot))
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::%s chassisSlot [%d] not installed", functionName, chassisSlot));

		return shipObject;
	}

	//----------------------------------------------------------------------

	void sendGameNetworkMessageToObservers(ServerObject const & sourceObject, GameNetworkMessage const & message, bool sendToSourceObject)
	{
		// send the projectile to all clients observing the owner except the owner
		// time is in the context of the connection server's sync stamp, so we need a unique message per connection server
		typedef std::map<ConnectionServerConnection *, std::vector<NetworkId> > DistributionList;
		DistributionList distributionList;

		//-- Build distribution list of clients observing object, but don't include the client doing the firing
		{
			typedef std::set<Client *> ObserverList;
			ObserverList const & observers = sourceObject.getObservers();
			for (ObserverList::const_iterator iter = observers.begin(); iter != observers.end(); ++iter)
			{
				Client * const client = *iter;
				if (sendToSourceObject || client != sourceObject.getClient())
					distributionList[client->getConnection()].push_back(client->getCharacterObjectId());
			}
		}

		//-- Send message to distribution list
		{
			for (DistributionList::const_iterator iter = distributionList.begin(); iter != distributionList.end(); ++iter)
			{
				GameClientMessage const gameClientMessage(iter->second, true, message);
				iter->first->send(gameClientMessage, true);
			}
		}
	}

	bool install();

	jboolean     JNICALL pilotShip(JNIEnv *env, jobject self, jlong pilotId, jlong shipIdId);
	jboolean     JNICALL unpilotShip(JNIEnv *env, jobject self, jlong pilotId);
	jlong        JNICALL getShipPilot(JNIEnv * env, jobject self, jlong shipId);
	jlong        JNICALL getPilotedShip(JNIEnv * env, jobject self, jlong pilotId);
	jboolean     JNICALL getShipHasWings(JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipCurrentSpeed(JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL setShipSlideDampener(JNIEnv * env, jobject self, jlong shipId, jfloat slideDampener);
	jfloat       JNICALL getShipSlideDampener(JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL setShipCurrentYaw(JNIEnv * env, jobject self, jlong shipId, jfloat currentYaw);
	jfloat       JNICALL getShipCurrentYaw(JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL setShipCurrentPitch(JNIEnv * env, jobject self, jlong shipId, jfloat currentPitch);
	jfloat       JNICALL getShipCurrentPitch(JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL setShipCurrentRoll(JNIEnv * env, jobject self, jlong shipId, jfloat currentRoll);
	jfloat       JNICALL getShipCurrentRoll(JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL setShipCurrentChassisHitPoints(JNIEnv * env, jobject self, jlong shipId, jfloat currentChassisHitPoints);
	jfloat       JNICALL getShipCurrentChassisHitPoints(JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL setShipMaximumChassisHitPoints(JNIEnv * env, jobject self, jlong shipId, jfloat maximumChassisHitPoints);
	jfloat       JNICALL getShipMaximumChassisHitPoints(JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL launchMissile(JNIEnv * env, jobject self, jlong player, jlong ship, jlong target, jint missileInfo, jint targetedComponent);
	jboolean     JNICALL launchCountermeasure(JNIEnv * env, jobject self, jlong ship, jint targettedMissile, jboolean successful, jint countermeasureType);
	jint         JNICALL getNearestUnlockedMissileForTarget(JNIEnv * env, jobject self, jlong target);
	jintArray    JNICALL getAllUnlockedMissilesForTarget(JNIEnv * env, jobject self, jlong jtarget);
	jboolean     JNICALL destroyMissile(JNIEnv * env, jobject self, jint missileId);
	jint         JNICALL getTypeByMissile(JNIEnv * env, jobject self, jint missileId);
	jint         JNICALL getWeaponIndexByMissile(JNIEnv * env, jobject self, jint missileId);
	jint         JNICALL getTimeSinceFiredByMissile(JNIEnv * env, jobject self, jint missileId);
	jint         JNICALL getTotalTimeByMissile(JNIEnv * env, jobject self, jint missileId);
	jlong        JNICALL getWhoFiredByMissile(JNIEnv * env, jobject self, jint missileId);
	jboolean     JNICALL setLookAtTargetShipComponent(JNIEnv * env, jobject self, jlong ship, jint targetedComponent);
	jint         JNICALL getLookAtTargetShipComponent(JNIEnv * env, jobject self, jlong ship);
	jlongArray   JNICALL getObservedShips(JNIEnv *env, jobject self, jlong jsource, jboolean excludeSource);
	jlongArray   JNICALL getObservedPlayerShips(JNIEnv *env, jobject self, jlong jsource, jboolean excludeSource);
	jlongArray   JNICALL getObservedEnemyShips(JNIEnv *env, jobject self, jlong jsource);
	jlongArray   JNICALL getObservedEnemyPlayerShips(JNIEnv *env, jobject self, jlong jsource);
	void         JNICALL notifyShipDamage(JNIEnv * env, jobject self, jlong victim, jlong attacker, jfloat totalDamage);
	jfloat       JNICALL getShipChassisSpeedMaximumModifier            (JNIEnv * env, jobject self, jlong shipId);
	void         JNICALL setShipChassisSpeedMaximumModifier            (JNIEnv * env, jobject self, jlong shipId, jfloat value);
	jfloat       JNICALL getShipActualAccelerationRate                 (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipActualDecelerationRate                 (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipActualPitchAccelerationRateDegrees     (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipActualYawAccelerationRateDegrees       (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipActualRollAccelerationRateDegrees      (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipActualPitchRateMaximumDegrees          (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipActualYawRateMaximumDegrees            (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipActualRollRateMaximumDegrees           (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipActualSpeedMaximum                     (JNIEnv * env, jobject self, jlong shipId);
	jstring      JNICALL getShipChassisType                            (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getChassisComponentMassCurrent                (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getChassisComponentMassMaximum                (JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL setChassisComponentMassMaximum                (JNIEnv * env, jobject self, jlong shipId, jfloat chassisComponentMassMaximum);
	jboolean     JNICALL isShipSlotInstalled                           (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jboolean     JNICALL isShipComponentDisabled                       (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jboolean     JNICALL isShipComponentLowPower                       (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jboolean     JNICALL isShipComponentDisabledNeedsPower             (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipComponentArmorHitpointsMaximum         (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipComponentArmorHitpointsCurrent         (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipComponentEfficiencyGeneral             (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipComponentEfficiencyEnergy              (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipComponentEnergyMaintenanceRequirement  (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipComponentMass                          (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jint         JNICALL getShipComponentCrc                           (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipComponentHitpointsCurrent              (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipComponentHitpointsMaximum              (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jint         JNICALL getShipComponentFlags                         (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jstring      JNICALL getShipComponentName                          (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipWeaponDamageMaximum                    (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipWeaponDamageMinimum                    (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipWeaponEffectivenessShields             (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipWeaponEffectivenessArmor               (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipWeaponEnergyPerShot                    (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipWeaponRefireRate                       (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipWeaponActualRefireRate                 (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipWeaponEfficiencyRefireRate             (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jint         JNICALL getShipWeaponAmmoCurrent                      (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jint         JNICALL getShipWeaponAmmoMaximum                      (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jint         JNICALL getShipWeaponAmmoType                         (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jfloat       JNICALL getShipShieldHitpointsFrontCurrent            (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipShieldHitpointsFrontMaximum            (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipShieldHitpointsBackCurrent             (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipShieldHitpointsBackMaximum             (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipShieldRechargeRate                     (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipCapacitorEnergyCurrent                 (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipCapacitorEnergyMaximum                 (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipCapacitorEnergyRechargeRate            (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineAccelerationRate                 (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineDecelerationRate                 (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEnginePitchAccelerationRateDegrees     (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineYawAccelerationRateDegrees       (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineRollAccelerationRateDegrees      (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEnginePitchRateMaximumDegrees          (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineYawRateMaximumDegrees            (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineRollRateMaximumDegrees           (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineSpeedMaximum                     (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineSpeedRotationFactorMaximum       (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineSpeedRotationFactorMinimum       (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipEngineSpeedRotationFactorOptimal       (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipReactorEnergyGenerationRate            (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipBoosterEnergyCurrent                   (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipBoosterEnergyMaximum                   (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipBoosterEnergyRechargeRate              (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipBoosterEnergyConsumptionRate           (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipBoosterAcceleration                    (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipBoosterSpeedMaximum                    (JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL isShipBoosterActive                           (JNIEnv * env, jobject self, jlong shipId);
	jfloat       JNICALL getShipDroidInterfaceCommandSpeed             (JNIEnv * env, jobject self, jlong shipId);
	int          JNICALL getShipCargoHoldContentsMaximum               (JNIEnv * env, jobject self, jlong shipId);
	int          JNICALL getShipCargoHoldContentsCurrent               (JNIEnv * env, jobject self, jlong shipId);
	void         JNICALL setShipComponentArmorHitpointsMaximum        (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat componentArmorHitpointsMaximum);
	void         JNICALL setShipComponentArmorHitpointsCurrent        (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat componentArmorHitpointsCurrent);
	void         JNICALL setShipComponentEfficiencyGeneral            (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat componentEfficiencyGeneral);
	void         JNICALL setShipComponentEfficiencyEnergy             (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat componentEfficiencyEnergy);
	void         JNICALL setShipComponentEnergyMaintenanceRequirement (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat componentEnergyMaintenanceRequirement);
	void         JNICALL setShipComponentMass                         (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat componentMass);
	void         JNICALL setShipComponentHitpointsCurrent             (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat componentHitpointsCurrent);
	void         JNICALL setShipComponentHitpointsMaximum             (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat componentHitpointsMaximum);
	void         JNICALL setShipComponentFlags                        (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jint componentFlags);
	void         JNICALL setShipComponentName                         (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jstring componentName);
	void         JNICALL setShipComponentDisabled                     (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jboolean componentDisabled);
	void         JNICALL setShipComponentLowPower                     (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jboolean componentLowPower);
	void         JNICALL setShipComponentDisabledNeedsPower           (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jboolean componentDisabledNeedsPower);
	void         JNICALL setShipWeaponDamageMaximum                   (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat weaponDamageMaximum);
	void         JNICALL setShipWeaponDamageMinimum                   (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat weaponDamageMinimum);
	void         JNICALL setShipWeaponEffectivenessShields            (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat weaponEffectivenessShields);
	void         JNICALL setShipWeaponEffectivenessArmor              (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat weaponEffectivenessArmor);
	void         JNICALL setShipWeaponEnergyPerShot                   (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat weaponEnergyPerShot);
	void         JNICALL setShipWeaponRefireRate                      (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat weaponRefireRate);
	void         JNICALL setShipWeaponEfficiencyRefireRate            (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jfloat weaponEfficiencyRefireRate);
	void         JNICALL setShipWeaponAmmoCurrent                     (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jint ammoCurrent);
	void         JNICALL setShipWeaponAmmoMaximum                     (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jint ammoMaximum);
	void         JNICALL setShipWeaponAmmoType                        (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jint ammoType);
	void         JNICALL setShipShieldHitpointsFrontCurrent           (JNIEnv * env, jobject self, jlong shipId, jfloat shieldHitpointsFrontCurrent);
	void         JNICALL setShipShieldHitpointsFrontMaximum           (JNIEnv * env, jobject self, jlong shipId, jfloat shieldHitpointsFrontMaximum);
	void         JNICALL setShipShieldHitpointsBackCurrent            (JNIEnv * env, jobject self, jlong shipId, jfloat shieldHitpointsBackCurrent);
	void         JNICALL setShipShieldHitpointsBackMaximum            (JNIEnv * env, jobject self, jlong shipId, jfloat shieldHitpointsBackMaximum);
	void         JNICALL setShipShieldRechargeRate                    (JNIEnv * env, jobject self, jlong shipId, jfloat shieldRechargeRate);
	void         JNICALL setShipCapacitorEnergyCurrent                (JNIEnv * env, jobject self, jlong shipId, jfloat capacitorEnergyCurrent);
	void         JNICALL setShipCapacitorEnergyMaximum                (JNIEnv * env, jobject self, jlong shipId, jfloat capacitorEnergyMaximum);
	void         JNICALL setShipCapacitorEnergyRechargeRate           (JNIEnv * env, jobject self, jlong shipId, jfloat capacitorEnergyRechargeRate);
	void         JNICALL setShipEngineAccelerationRate                (JNIEnv * env, jobject self, jlong shipId, jfloat engineAccelerationRate);
	void         JNICALL setShipEngineDecelerationRate                (JNIEnv * env, jobject self, jlong shipId, jfloat engineDecelerationRate);
	void         JNICALL setShipEnginePitchAccelerationRateDegrees    (JNIEnv * env, jobject self, jlong shipId, jfloat enginePitchAccelerationRateDegrees);
	void         JNICALL setShipEngineYawAccelerationRateDegrees      (JNIEnv * env, jobject self, jlong shipId, jfloat engineYawAccelerationRateDegrees);
	void         JNICALL setShipEngineRollAccelerationRateDegrees     (JNIEnv * env, jobject self, jlong shipId, jfloat engineRollAccelerationRateDegrees);
	void         JNICALL setShipEnginePitchRateMaximumDegrees         (JNIEnv * env, jobject self, jlong shipId, jfloat enginePitchRateMaximumDegrees);
	void         JNICALL setShipEngineYawRateMaximumDegrees           (JNIEnv * env, jobject self, jlong shipId, jfloat engineYawRateMaximumDegrees);
	void         JNICALL setShipEngineRollRateMaximumDegrees          (JNIEnv * env, jobject self, jlong shipId, jfloat engineRollRateMaximumDegrees);
	void         JNICALL setShipEngineSpeedMaximum                    (JNIEnv * env, jobject self, jlong shipId, jfloat engineSpeedMaximum);
	void         JNICALL setShipEngineSpeedRotationFactorMaximum      (JNIEnv * env, jobject self, jlong shipId, jfloat engineSpeedRotationFactorMaximum);
	void         JNICALL setShipEngineSpeedRotationFactorMinimum      (JNIEnv * env, jobject self, jlong shipId, jfloat engineSpeedRotationFactorMinimum);
	void         JNICALL setShipEngineSpeedRotationFactorOptimal      (JNIEnv * env, jobject self, jlong shipId, jfloat engineSpeedRotationFactorOptimal);
	void         JNICALL setShipReactorEnergyGenerationRate           (JNIEnv * env, jobject self, jlong shipId, jfloat reactorEnergyGenerationRate);
	void         JNICALL setShipBoosterEnergyCurrent                  (JNIEnv * env, jobject self, jlong shipId, jfloat boosterEnergyCurrent);
	void         JNICALL setShipBoosterEnergyMaximum                  (JNIEnv * env, jobject self, jlong shipId, jfloat boosterEnergyMaximum);
	void         JNICALL setShipBoosterEnergyRechargeRate             (JNIEnv * env, jobject self, jlong shipId, jfloat boosterEnergyRechargeRate);
	void         JNICALL setShipBoosterEnergyConsumptionRate          (JNIEnv * env, jobject self, jlong shipId, jfloat boosterEnergyConsumptionRate);
	void         JNICALL setShipBoosterAcceleration                   (JNIEnv * env, jobject self, jlong shipId, jfloat boosterAcceleration);
	void         JNICALL setShipBoosterSpeedMaximum                   (JNIEnv * env, jobject self, jlong shipId, jfloat boosterSpeedMaximum);
	void         JNICALL setShipDroidInterfaceCommandSpeed            (JNIEnv * env, jobject self, jlong shipId, jfloat droidInterfaceCommandSpeed);
	void         JNICALL setShipCargoHoldContentsMaximum              (JNIEnv * env, jobject self, jlong shipId, jint contentsMaximum);
	void         JNICALL setShipCargoHoldContent                      (JNIEnv * env, jobject self, jlong shipId, jlong resourceTypeId, jint amount);
	jint         JNICALL getShipCargoHoldContent                      (JNIEnv * env, jobject self, jlong shipId, jlong resourceTypeId);
	jlongArray   JNICALL getShipCargoHoldContentsResourceTypes      (JNIEnv * env, jobject self, jlong shipId);
	jintArray    JNICALL getShipCargoHoldContentsAmounts               (JNIEnv * env, jobject self, jlong shipId);
	jboolean     JNICALL shipCanInstallComponent                      (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jlong component);
	jboolean     JNICALL shipPseudoInstallComponent                   (JNIEnv * env, jobject self, jlong jship, jint chassisSlot, jint componentCrc);
	jboolean     JNICALL shipInstallComponent                         (JNIEnv * env, jobject self, jlong installerId, jlong shipId, jint chassisSlot, jlong component);
	jlong        JNICALL shipUninstallComponent                       (JNIEnv * env, jobject self, jlong uninstallerId, jlong shipId, jint chassisSlot, jlong containerTarget);
	// allow an uninstalled component to overflow the container.
	jlong        JNICALL shipUninstallComponentAllowOverload          (JNIEnv * env, jobject self, jlong uninstallerId, jlong shipId, jint chassisSlot, jlong containerTarget);
	jlong        JNICALL shipUninstallComponentBase          (JNIEnv * env, jobject self, jlong uninstallerId, jlong shipId, jint chassisSlot, jlong containerTarget, bool allowOverload);
	jintArray    JNICALL getShipChassisSlots                          (JNIEnv * env, jobject self, jlong shipId);
	void         JNICALL setShipSlotTargetable                        (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot, jboolean targetable);
	jboolean     JNICALL isShipSlotTargetable                         (JNIEnv * env, jobject self, jlong shipId, jint chassisSlot);
	jint         JNICALL getShipComponentDescriptorType                   (JNIEnv * env, jobject self, jlong component);
	jint         JNICALL getShipComponentDescriptorTypeByName             (JNIEnv * env, jobject self, jstring typeName);
	jstring      JNICALL getShipComponentDescriptorTypeName               (JNIEnv * env, jobject self, jint componentType);
	jint         JNICALL getShipComponentDescriptorCrc                    (JNIEnv * env, jobject self, jlong component);
	jstring      JNICALL getShipComponentDescriptorCrcName                (JNIEnv * env, jobject self, jint componentCrc);
	jstring      JNICALL getShipComponentDescriptorCompatibility          (JNIEnv * env, jobject self, jint componentCrc);
	jboolean     JNICALL getShipComponentDescriptorWeaponIsAmmoConsuming  (JNIEnv * env, jobject self, jint componentCrc);
	jboolean     JNICALL getShipComponentDescriptorWeaponIsMissile        (JNIEnv * env, jobject self, jint componentCrc);
	jboolean     JNICALL getShipComponentDescriptorWeaponIsCountermeasure (JNIEnv * env, jobject self, jint componentCrc);
	jboolean     JNICALL getShipComponentDescriptorWeaponIsMining         (JNIEnv * env, jobject self, jint componentCrc);
	jboolean     JNICALL getShipComponentDescriptorWeaponIsTractor        (JNIEnv * env, jobject self, jint componentCrc);
	jfloat       JNICALL getShipComponentDescriptorWeaponRange            (JNIEnv * env, jobject self, jint componentCrc);
	jfloat       JNICALL getShipComponentDescriptorWeaponProjectileSpeed  (JNIEnv * env, jobject self, jint componentCrc);
	jlong        JNICALL getDroidControlDeviceForShip                 (JNIEnv * env, jobject self, jlong jship);
	jboolean     JNICALL associateDroidControlDeviceWithShip          (JNIEnv * env, jobject self, jlong jship, jlong jdroidControlDevice);
	jboolean     JNICALL removeDroidControlDeviceFromShip             (JNIEnv * env, jobject self, jlong jship);
	jlong        JNICALL launchShipFromHangar                         (JNIEnv * env, jobject self, jlong jshipId, jstring templateToSpawn, jobject deltaFromHangarHardpoint);
	void         JNICALL commPlayers                                  (JNIEnv * env, jobject self, jlong jtaunter, jstring appearanceOverloadServerTemplate, jstring soundeffect, jfloat duration, jlongArray jplayers, jstring jtauntOob, jboolean chronicles);
	void         JNICALL handleShipDestruction(JNIEnv * env, jobject self, jlong ship, jfloat severity);
	void         JNICALL handleShipComponentDestruction(JNIEnv * env, jobject self, jlong ship, jint chassisSlot, jfloat severity);
	jboolean     JNICALL setShipInternalDamageOverTime(JNIEnv * env, jobject self, jlong ship, jint chassisSlot, jfloat damageRate, jfloat damageThreshold);
	jboolean     JNICALL removeShipInternalDamageOverTime(JNIEnv * env, jobject self, jlong ship, jint chassisSlot);
	jfloat       JNICALL getShipInternalDamageOverTimeDamageRate(JNIEnv * env, jobject self, jlong ship, jint chassisSlot);
	jfloat       JNICALL getShipInternalDamageOverTimeDamageThreshold(JNIEnv * env, jobject self, jlong ship, jint chassisSlot);
	jboolean     JNICALL hasShipInternalDamageOverTime(JNIEnv * env, jobject self, jlong ship, jint chassisSlot);
	jint         JNICALL shipGetSpaceFaction(JNIEnv *env, jobject self, jlong jship);
	jintArray    JNICALL shipGetSpaceFactionAllies(JNIEnv *env, jobject self, jlong jship);
	jintArray    JNICALL shipGetSpaceFactionEnemies(JNIEnv *env, jobject self, jlong jship);
	jboolean     JNICALL shipGetSpaceFactionIsAggro(JNIEnv *env, jobject self, jlong jship);
	void         JNICALL shipSetSpaceFaction(JNIEnv *env, jobject self, jlong jship, jint spaceFaction);
	void         JNICALL shipSetSpaceFactionAllies(JNIEnv *env, jobject self, jlong jship, jintArray jfactionList);
	void         JNICALL shipSetSpaceFactionEnemies(JNIEnv *env, jobject self, jlong jship, jintArray jfactionList);
	void         JNICALL shipSetSpaceFactionIsAggro(JNIEnv *env, jobject self, jlong jship, jboolean isAggro);
	jboolean     JNICALL shipIsInNebula(JNIEnv *env, jobject self, jlong ship, jstring nebulaName);
	jboolean     JNICALL setShipWingName(JNIEnv *env, jobject self, jlong target, jstring str);
	jstring      JNICALL getShipWingName(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setShipTypeName(JNIEnv *env, jobject self, jlong target, jstring str);
	jstring      JNICALL getShipTypeName(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setShipDifficulty(JNIEnv *env, jobject self, jlong target, jstring str);
	jstring      JNICALL getShipDifficulty(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setShipFaction(JNIEnv *env, jobject self, jlong target, jstring str);
	jstring      JNICALL getShipFaction(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL addMissionCriticalObject(JNIEnv *env, jobject self, jlong playerObjectId, jlong missionCriticalObjectId);
	jboolean     JNICALL removeMissionCriticalObject(JNIEnv *env, jobject self, jlong playerObjectId, jlong missionCriticalObjectId);
	jboolean     JNICALL clearMissionCriticalObjects(JNIEnv *env, jobject self, jlong playerObjectId);
	jboolean     JNICALL isMissionCriticalObject(JNIEnv *env, jobject self, jlong playerObjectId, jlong missionCriticalObjectId);
	jstring      JNICALL getShipComponentDebugString(JNIEnv * env, jobject self, jlong jobject_shipId, jint chassisSlot);
	void         JNICALL notifyShipHit(JNIEnv * env, jobject self, jlong jobject_shipId, jobject jobject_vectorUp_w, jobject jobject_vectorHitLocation_o, jint type, jfloat integrity, jfloat previousIntegrity);
	void         JNICALL setDynamicMiningAsteroidVelocity(JNIEnv * env, jobject self, jlong jobject_asteroidId, jobject jobject_velocity_w);
	jobject      JNICALL getDynamicMiningAsteroidVelocity(JNIEnv * env, jobject self, jlong jobject_asteroidId);
	void         JNICALL openSpaceMiningUi(JNIEnv * env, jobject self, jlong player, jlong spaceStation, jstring spaceStationName);
}

using namespace ScriptMethodsShipNamespace;


//========================================================================
// install
//========================================================================

bool ScriptMethodsShipNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsShipNamespace::c)}
	JF("_pilotShip", "(JJ)Z", pilotShip),
	JF("_unpilotShip", "(J)Z", unpilotShip),
	JF("_getPilotId", "(J)J", getShipPilot),
	JF("_getShipPilot", "(J)J", getShipPilot),
	JF("_getPilotedShip", "(J)J", getPilotedShip),
	JF("_getShipHasWings", "(J)Z", getShipHasWings),
	JF("_getShipCurrentSpeed", "(J)F", getShipCurrentSpeed),
	JF("_setShipSlideDampener", "(JF)Z", setShipSlideDampener),
	JF("_getShipSlideDampener", "(J)F", getShipSlideDampener),
	JF("_setShipCurrentYaw", "(JF)Z", setShipCurrentYaw),
	JF("_getShipCurrentYaw", "(J)F", getShipCurrentYaw),
	JF("_setShipCurrentPitch", "(JF)Z", setShipCurrentPitch),
	JF("_getShipCurrentPitch", "(J)F", getShipCurrentPitch),
	JF("_setShipCurrentRoll", "(JF)Z", setShipCurrentRoll),
	JF("_getShipCurrentRoll", "(J)F", getShipCurrentRoll),
	JF("_setShipCurrentChassisHitPoints", "(JF)Z", setShipCurrentChassisHitPoints),
	JF("_getShipCurrentChassisHitPoints", "(J)F", getShipCurrentChassisHitPoints),
	JF("_setShipMaximumChassisHitPoints", "(JF)Z", setShipMaximumChassisHitPoints),
	JF("_getShipMaximumChassisHitPoints", "(J)F", getShipMaximumChassisHitPoints),
	JF("_launchMissile", "(JJJII)Z", launchMissile),
	JF("_launchCountermeasure", "(JIZI)Z", launchCountermeasure),
	JF("_getNearestUnlockedMissileForTarget", "(J)I", getNearestUnlockedMissileForTarget),
	JF("_getAllUnlockedMissilesForTarget", "(J)[I", getAllUnlockedMissilesForTarget),
	JF("destroyMissile", "(I)Z", destroyMissile),
	JF("getTypeByMissile","(I)I", getTypeByMissile),
	JF("getWeaponIndexByMissile","(I)I",getWeaponIndexByMissile),
	JF("getTimeSinceFiredByMissile","(I)I",getTimeSinceFiredByMissile),
	JF("getTotalTimeByMissile","(I)I",getTotalTimeByMissile),
	JF("_getWhoFiredByMissile","(I)J",getWhoFiredByMissile),
	JF("_setLookAtTargetShipComponent", "(JI)Z", setLookAtTargetShipComponent),
	JF("_getLookAtTargetShipComponent", "(J)I", getLookAtTargetShipComponent),
	JF("_getObservedShips", "(JZ)[J", getObservedShips),
	JF("_getObservedPlayerShips", "(JZ)[J", getObservedPlayerShips),
	JF("_getObservedEnemyShips", "(J)[J", getObservedEnemyShips),
	JF("_getObservedEnemyPlayerShips", "(J)[J", getObservedEnemyPlayerShips),
	JF("_notifyShipDamage", "(JJF)V", notifyShipDamage),
	JF("_getShipChassisSpeedMaximumModifier", "(J)F", getShipChassisSpeedMaximumModifier),
	JF("_setShipChassisSpeedMaximumModifier", "(JF)V", setShipChassisSpeedMaximumModifier),
	JF("_getShipActualAccelerationRate",               "(J)F",      getShipActualAccelerationRate),
	JF("_getShipActualDecelerationRate",               "(J)F",      getShipActualDecelerationRate),
	JF("_getShipActualPitchAccelerationRateDegrees",   "(J)F",      getShipActualPitchAccelerationRateDegrees),
	JF("_getShipActualYawAccelerationRateDegrees",     "(J)F",      getShipActualYawAccelerationRateDegrees),
	JF("_getShipActualRollAccelerationRateDegrees",    "(J)F",      getShipActualRollAccelerationRateDegrees),
	JF("_getShipActualPitchRateMaximumDegrees",        "(J)F",      getShipActualPitchRateMaximumDegrees),
	JF("_getShipActualYawRateMaximumDegrees",          "(J)F",      getShipActualYawRateMaximumDegrees),
	JF("_getShipActualRollRateMaximumDegrees",         "(J)F",      getShipActualRollRateMaximumDegrees),
	JF("_getShipActualSpeedMaximum",                   "(J)F",      getShipActualSpeedMaximum),
	JF("_handleShipDestruction", "(JF)V", handleShipDestruction),
	JF("_handleShipComponentDestruction", "(JIF)V", handleShipComponentDestruction),
	JF("_setShipInternalDamageOverTime", "(JIFF)Z", setShipInternalDamageOverTime),
	JF("_removeShipInternalDamageOverTime", "(JI)Z", removeShipInternalDamageOverTime),
	JF("_getShipInternalDamageOverTimeDamageRate", "(JI)F", getShipInternalDamageOverTimeDamageRate),
	JF("_getShipInternalDamageOverTimeDamageThreshold", "(JI)F", getShipInternalDamageOverTimeDamageThreshold),
	JF("_hasShipInternalDamageOverTime", "(JI)Z", hasShipInternalDamageOverTime),
	JF("_shipGetSpaceFaction", "(J)I", shipGetSpaceFaction),
	JF("_shipGetSpaceFactionAllies", "(J)[I", shipGetSpaceFactionAllies),
	JF("_shipGetSpaceFactionEnemies", "(J)[I", shipGetSpaceFactionEnemies),
	JF("_shipGetSpaceFactionIsAggro", "(J)Z", shipGetSpaceFactionIsAggro),
	JF("_shipSetSpaceFaction", "(JI)V", shipSetSpaceFaction),
	JF("_shipSetSpaceFactionAllies", "(J[I)V", shipSetSpaceFactionAllies),
	JF("_shipSetSpaceFactionEnemies", "(J[I)V", shipSetSpaceFactionEnemies),
	JF("_shipSetSpaceFactionIsAggro", "(JZ)V", shipSetSpaceFactionIsAggro),
	JF("_getShipChassisType",                           "(J)Ljava/lang/String;",     getShipChassisType),
	JF("_getChassisComponentMassCurrent",               "(J)F",     getChassisComponentMassCurrent),
	JF("_getChassisComponentMassMaximum",               "(J)F",     getChassisComponentMassMaximum),
	JF("_setChassisComponentMassMaximum",               "(JF)Z",    setChassisComponentMassMaximum),
	JF("_isShipSlotInstalled",                          "(JI)Z",     isShipSlotInstalled),
	JF("_isShipComponentDisabled",                      "(JI)Z",     isShipComponentDisabled),
	JF("_isShipComponentLowPower",                      "(JI)Z",     isShipComponentLowPower),
	JF("_isShipComponentDisabledNeedsPower",            "(JI)Z",     isShipComponentDisabledNeedsPower),
	JF("_getShipComponentArmorHitpointsMaximum",        "(JI)F",     getShipComponentArmorHitpointsMaximum),
	JF("_getShipComponentArmorHitpointsCurrent",        "(JI)F",     getShipComponentArmorHitpointsCurrent),
	JF("_getShipComponentEfficiencyGeneral",            "(JI)F",     getShipComponentEfficiencyGeneral),
	JF("_getShipComponentEfficiencyEnergy",             "(JI)F",     getShipComponentEfficiencyEnergy),
	JF("_getShipComponentEnergyMaintenanceRequirement", "(JI)F",     getShipComponentEnergyMaintenanceRequirement),
	JF("_getShipComponentMass",                         "(JI)F",     getShipComponentMass),
	JF("_getShipComponentCrc",                          "(JI)I",     getShipComponentCrc),
	JF("_getShipComponentHitpointsCurrent",             "(JI)F",     getShipComponentHitpointsCurrent),
	JF("_getShipComponentHitpointsMaximum",             "(JI)F",     getShipComponentHitpointsMaximum),
	JF("_getShipComponentFlags",                        "(JI)I",     getShipComponentFlags),
	JF("_getShipComponentName",                         "(JI)Ljava/lang/String;",     getShipComponentName),
	JF("_getShipWeaponDamageMaximum",                   "(JI)F",     getShipWeaponDamageMaximum),
	JF("_getShipWeaponDamageMinimum",                   "(JI)F",     getShipWeaponDamageMinimum),
	JF("_getShipWeaponEffectivenessShields",            "(JI)F",     getShipWeaponEffectivenessShields),
	JF("_getShipWeaponEffectivenessArmor",              "(JI)F",     getShipWeaponEffectivenessArmor),
	JF("_getShipWeaponEnergyPerShot",                   "(JI)F",     getShipWeaponEnergyPerShot),
	JF("_getShipWeaponRefireRate",                      "(JI)F",     getShipWeaponRefireRate),
	JF("_getShipWeaponActualRefireRate",                "(JI)F",     getShipWeaponActualRefireRate),
	JF("_getShipWeaponEfficiencyRefireRate",            "(JI)F",     getShipWeaponEfficiencyRefireRate),
	JF("_getShipWeaponAmmoCurrent",                     "(JI)I",     getShipWeaponAmmoCurrent),
	JF("_getShipWeaponAmmoMaximum",                     "(JI)I",     getShipWeaponAmmoMaximum),
	JF("_getShipWeaponAmmoType",                        "(JI)I",     getShipWeaponAmmoType),
	JF("_getShipShieldHitpointsFrontCurrent",           "(J)F",      getShipShieldHitpointsFrontCurrent),
	JF("_getShipShieldHitpointsFrontMaximum",           "(J)F",      getShipShieldHitpointsFrontMaximum),
	JF("_getShipShieldHitpointsBackCurrent",            "(J)F",      getShipShieldHitpointsBackCurrent),
	JF("_getShipShieldHitpointsBackMaximum",            "(J)F",      getShipShieldHitpointsBackMaximum),
	JF("_getShipShieldRechargeRate",                    "(J)F",      getShipShieldRechargeRate),
	JF("_getShipCapacitorEnergyCurrent",                "(J)F",      getShipCapacitorEnergyCurrent),
	JF("_getShipCapacitorEnergyMaximum",                "(J)F",      getShipCapacitorEnergyMaximum),
	JF("_getShipCapacitorEnergyRechargeRate",           "(J)F",      getShipCapacitorEnergyRechargeRate),
	JF("_getShipEngineAccelerationRate",                "(J)F",      getShipEngineAccelerationRate),
	JF("_getShipEngineDecelerationRate",                "(J)F",      getShipEngineDecelerationRate),
	JF("_getShipEnginePitchAccelerationRateDegrees",    "(J)F",      getShipEnginePitchAccelerationRateDegrees),
	JF("_getShipEngineYawAccelerationRateDegrees",      "(J)F",      getShipEngineYawAccelerationRateDegrees),
	JF("_getShipEngineRollAccelerationRateDegrees",     "(J)F",      getShipEngineRollAccelerationRateDegrees),
	JF("_getShipEnginePitchRateMaximumDegrees",         "(J)F",      getShipEnginePitchRateMaximumDegrees),
	JF("_getShipEngineYawRateMaximumDegrees",           "(J)F",      getShipEngineYawRateMaximumDegrees),
	JF("_getShipEngineRollRateMaximumDegrees",          "(J)F",      getShipEngineRollRateMaximumDegrees),
	JF("_getShipEngineSpeedMaximum",                    "(J)F",      getShipEngineSpeedMaximum),
	JF("_getShipEngineSpeedRotationFactorMaximum",      "(J)F",      getShipEngineSpeedRotationFactorMaximum),
	JF("_getShipEngineSpeedRotationFactorMinimum",      "(J)F",      getShipEngineSpeedRotationFactorMinimum),
	JF("_getShipEngineSpeedRotationFactorOptimal",      "(J)F",      getShipEngineSpeedRotationFactorOptimal),
	JF("_getShipReactorEnergyGenerationRate",           "(J)F",      getShipReactorEnergyGenerationRate),
	JF("_getShipBoosterEnergyCurrent",                  "(J)F",      getShipBoosterEnergyCurrent),
	JF("_getShipBoosterEnergyMaximum",                  "(J)F",      getShipBoosterEnergyMaximum),
	JF("_getShipBoosterEnergyRechargeRate",             "(J)F",      getShipBoosterEnergyRechargeRate),
	JF("_getShipBoosterEnergyConsumptionRate",          "(J)F",      getShipBoosterEnergyConsumptionRate),
	JF("_getShipBoosterAcceleration",                   "(J)F",      getShipBoosterAcceleration),
	JF("_getShipBoosterSpeedMaximum",                   "(J)F",      getShipBoosterSpeedMaximum),
	JF("_isShipBoosterActive",                          "(J)Z",      isShipBoosterActive),
	JF("_getShipDroidInterfaceCommandSpeed",            "(J)F",      getShipDroidInterfaceCommandSpeed),
	JF("_getShipCargoHoldContentsMaximum",              "(J)I",      getShipCargoHoldContentsMaximum),
	JF("_getShipCargoHoldContentsCurrent",              "(J)I",      getShipCargoHoldContentsCurrent),
	JF("_setShipComponentArmorHitpointsMaximum",        "(JIF)V",    setShipComponentArmorHitpointsMaximum),
	JF("_setShipComponentArmorHitpointsCurrent",        "(JIF)V",    setShipComponentArmorHitpointsCurrent),
	JF("_setShipComponentEfficiencyGeneral",            "(JIF)V",    setShipComponentEfficiencyGeneral),
	JF("_setShipComponentEfficiencyEnergy",             "(JIF)V",    setShipComponentEfficiencyEnergy),
	JF("_setShipComponentEnergyMaintenanceRequirement", "(JIF)V",    setShipComponentEnergyMaintenanceRequirement),
	JF("_setShipComponentMass",                         "(JIF)V",    setShipComponentMass),
	JF("_setShipComponentHitpointsCurrent",             "(JIF)V",    setShipComponentHitpointsCurrent),
	JF("_setShipComponentHitpointsMaximum",             "(JIF)V",    setShipComponentHitpointsMaximum),
	JF("_setShipComponentFlags",                        "(JII)V",    setShipComponentFlags),
	JF("_setShipComponentName",                         "(JILjava/lang/String;)V",    setShipComponentName),
	JF("_setShipComponentDisabled",                     "(JIZ)V",    setShipComponentDisabled),
	JF("_setShipComponentLowPower",                     "(JIZ)V",    setShipComponentLowPower),
	JF("_setShipComponentDisabledNeedsPower",           "(JIZ)V",    setShipComponentDisabledNeedsPower),
	JF("_setShipWeaponDamageMaximum",                   "(JIF)V",    setShipWeaponDamageMaximum),
	JF("_setShipWeaponDamageMinimum",                   "(JIF)V",    setShipWeaponDamageMinimum),
	JF("_setShipWeaponEffectivenessShields",            "(JIF)V",    setShipWeaponEffectivenessShields),
	JF("_setShipWeaponEffectivenessArmor",              "(JIF)V",    setShipWeaponEffectivenessArmor),
	JF("_setShipWeaponEnergyPerShot",                   "(JIF)V",    setShipWeaponEnergyPerShot),
	JF("_setShipWeaponRefireRate",                      "(JIF)V",    setShipWeaponRefireRate),
	JF("_setShipWeaponEfficiencyRefireRate",            "(JIF)V",    setShipWeaponEfficiencyRefireRate),
	JF("_setShipWeaponAmmoCurrent",                     "(JII)V",    setShipWeaponAmmoCurrent),
	JF("_setShipWeaponAmmoMaximum",                     "(JII)V",    setShipWeaponAmmoMaximum),
	JF("_setShipWeaponAmmoType",                        "(JII)V",    setShipWeaponAmmoType),
	JF("_setShipShieldHitpointsFrontCurrent",           "(JF)V",     setShipShieldHitpointsFrontCurrent),
	JF("_setShipShieldHitpointsFrontMaximum",           "(JF)V",     setShipShieldHitpointsFrontMaximum),
	JF("_setShipShieldHitpointsBackCurrent",            "(JF)V",     setShipShieldHitpointsBackCurrent),
	JF("_setShipShieldHitpointsBackMaximum",            "(JF)V",     setShipShieldHitpointsBackMaximum),
	JF("_setShipShieldRechargeRate",                    "(JF)V",     setShipShieldRechargeRate),
	JF("_setShipCapacitorEnergyCurrent",                "(JF)V",     setShipCapacitorEnergyCurrent),
	JF("_setShipCapacitorEnergyMaximum",                "(JF)V",     setShipCapacitorEnergyMaximum),
	JF("_setShipCapacitorEnergyRechargeRate",           "(JF)V",     setShipCapacitorEnergyRechargeRate),
	JF("_setShipEngineAccelerationRate",                "(JF)V",     setShipEngineAccelerationRate),
	JF("_setShipEngineDecelerationRate",                "(JF)V",     setShipEngineDecelerationRate),
	JF("_setShipEnginePitchAccelerationRateDegrees",    "(JF)V",     setShipEnginePitchAccelerationRateDegrees),
	JF("_setShipEngineYawAccelerationRateDegrees",      "(JF)V",     setShipEngineYawAccelerationRateDegrees),
	JF("_setShipEngineRollAccelerationRateDegrees",     "(JF)V",     setShipEngineRollAccelerationRateDegrees),
	JF("_setShipEnginePitchRateMaximumDegrees",         "(JF)V",     setShipEnginePitchRateMaximumDegrees),
	JF("_setShipEngineYawRateMaximumDegrees",           "(JF)V",     setShipEngineYawRateMaximumDegrees),
	JF("_setShipEngineRollRateMaximumDegrees",          "(JF)V",     setShipEngineRollRateMaximumDegrees),
	JF("_setShipEngineSpeedMaximum",                    "(JF)V",     setShipEngineSpeedMaximum),
	JF("_setShipEngineSpeedRotationFactorMaximum",      "(JF)V",     setShipEngineSpeedRotationFactorMaximum),
	JF("_setShipEngineSpeedRotationFactorMinimum",      "(JF)V",     setShipEngineSpeedRotationFactorMinimum),
	JF("_setShipEngineSpeedRotationFactorOptimal",      "(JF)V",     setShipEngineSpeedRotationFactorOptimal),
	JF("_setShipReactorEnergyGenerationRate",           "(JF)V",     setShipReactorEnergyGenerationRate),
	JF("_setShipBoosterEnergyCurrent",                  "(JF)V",     setShipBoosterEnergyCurrent),
	JF("_setShipBoosterEnergyMaximum",                  "(JF)V",     setShipBoosterEnergyMaximum),
	JF("_setShipBoosterEnergyRechargeRate",             "(JF)V",     setShipBoosterEnergyRechargeRate),
	JF("_setShipBoosterEnergyConsumptionRate",          "(JF)V",     setShipBoosterEnergyConsumptionRate),
	JF("_setShipBoosterAcceleration",                   "(JF)V",     setShipBoosterAcceleration),
	JF("_setShipBoosterSpeedMaximum",                   "(JF)V",     setShipBoosterSpeedMaximum),
	JF("_setShipDroidInterfaceCommandSpeed",            "(JF)V",     setShipDroidInterfaceCommandSpeed),
	JF("_setShipCargoHoldContentsMaximum",              "(JI)V",     setShipCargoHoldContentsMaximum),
	JF("_setShipCargoHoldContent",                      "(JJI)V",   setShipCargoHoldContent),
	JF("_getShipCargoHoldContent",                      "(JJ)I",    getShipCargoHoldContent),
	JF("_getShipCargoHoldContentsResourceTypes",        "(J)[J",    getShipCargoHoldContentsResourceTypes),
	JF("_getShipCargoHoldContentsAmounts",              "(J)[I",     getShipCargoHoldContentsAmounts),
	JF("_shipCanInstallComponent",                      "(JIJ)Z",                shipCanInstallComponent),
	JF("_shipPseudoInstallComponent",                   "(JII)Z",                              shipPseudoInstallComponent),
	JF("_shipInstallComponent",                         "(JJIJ)Z",                shipInstallComponent),
	JF("_shipUninstallComponent",                       "(JJIJ)J",  shipUninstallComponent),
	JF("_shipUninstallComponentAllowOverload",          "(JJIJ)J",  shipUninstallComponentAllowOverload),
	JF("_getShipChassisSlots",                          "(J)[I",                               getShipChassisSlots),
	JF("_setShipSlotTargetable",                        "(JIZ)V",                              setShipSlotTargetable),
	JF("_isShipSlotTargetable",                         "(JI)Z",                               isShipSlotTargetable),
	JF("_getShipComponentDescriptorType",                  "(J)I",                  getShipComponentDescriptorType),
	JF("getShipComponentDescriptorTypeName",              "(I)Ljava/lang/String;",               getShipComponentDescriptorTypeName),
	JF("getShipComponentDescriptorTypeByName",            "(Ljava/lang/String;)I",               getShipComponentDescriptorTypeByName),
	JF("_getShipComponentDescriptorCrc",                   "(J)I",                  getShipComponentDescriptorCrc),
	JF("getShipComponentDescriptorCrcName",               "(I)Ljava/lang/String;",               getShipComponentDescriptorCrcName),
	JF("getShipComponentDescriptorCompatibility",         "(I)Ljava/lang/String;",               getShipComponentDescriptorCompatibility),
	JF("getShipComponentDescriptorWeaponIsAmmoConsuming", "(I)Z",                                getShipComponentDescriptorWeaponIsAmmoConsuming),
	JF("getShipComponentDescriptorWeaponIsMissile",       "(I)Z",                                getShipComponentDescriptorWeaponIsMissile),
	JF("getShipComponentDescriptorWeaponIsCountermeasure","(I)Z",                                getShipComponentDescriptorWeaponIsCountermeasure),
	JF("getShipComponentDescriptorWeaponIsMining",        "(I)Z",                                getShipComponentDescriptorWeaponIsMining),
	JF("getShipComponentDescriptorWeaponIsTractor",       "(I)Z",                                getShipComponentDescriptorWeaponIsTractor),
	JF("getShipComponentDescriptorWeaponRange",           "(I)F",                                getShipComponentDescriptorWeaponRange),
	JF("getShipComponentDescriptorWeaponProjectileSpeed", "(I)F",                                getShipComponentDescriptorWeaponProjectileSpeed),
	JF("_getDroidControlDeviceForShip",                 "(J)J",                  getDroidControlDeviceForShip),
	JF("_associateDroidControlDeviceWithShip",          "(JJ)Z",                 associateDroidControlDeviceWithShip),
	JF("_removeDroidControlDeviceFromShip",             "(J)Z",                                removeDroidControlDeviceFromShip),
	JF("_launchShipFromHangar",                         "(JLjava/lang/String;Lscript/transform;)J", launchShipFromHangar),
	JF("__commPlayers",                                 "(JLjava/lang/String;Ljava/lang/String;F[JLjava/lang/String;Z)V", commPlayers),
	JF("_shipIsInNebula",                               "(JLjava/lang/String;)Z", shipIsInNebula),
	JF("_setShipWingName",                              "(JLjava/lang/String;)Z", setShipWingName),
	JF("_getShipWingName",                              "(J)Ljava/lang/String;", getShipWingName),
	JF("_setShipTypeName",                              "(JLjava/lang/String;)Z", setShipTypeName),
	JF("_getShipTypeName",                              "(J)Ljava/lang/String;", getShipTypeName),
	JF("_setShipDifficulty",                            "(JLjava/lang/String;)Z", setShipDifficulty),
	JF("_getShipDifficulty",                            "(J)Ljava/lang/String;", getShipDifficulty),
	JF("_setShipFaction",                               "(JLjava/lang/String;)Z", setShipFaction),
	JF("_getShipFaction",                               "(J)Ljava/lang/String;", getShipFaction),
	JF("_addMissionCriticalObject", "(JJ)Z", addMissionCriticalObject),
	JF("_removeMissionCriticalObject", "(JJ)Z", removeMissionCriticalObject),
	JF("_clearMissionCriticalObjects", "(J)Z", clearMissionCriticalObjects),
	JF("_isMissionCriticalObject", "(JJ)Z", isMissionCriticalObject),
	JF("_getShipComponentDebugString",                  "(JI)Ljava/lang/String;", getShipComponentDebugString),
	JF("_notifyShipHit",                                "(JLscript/vector;Lscript/vector;IFF)V", notifyShipHit),
	JF("_setDynamicMiningAsteroidVelocity",             "(JLscript/vector;)V", setDynamicMiningAsteroidVelocity),
	JF("_getDynamicMiningAsteroidVelocity",             "(J)Lscript/vector;", getDynamicMiningAsteroidVelocity),
	JF("_openSpaceMiningUi",                            "(JJLjava/lang/String;)V", openSpaceMiningUi),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jboolean JNICALL ScriptMethodsShipNamespace::pilotShip(JNIEnv *env, jobject /*self*/, jlong pilotId, jlong pilotSlotId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	//-- Get the pilot object.
	CreatureObject * const pilotObject = JavaLibrary::getCreatureThrow(env, pilotId, "pilotShip(): error in pilotId arg");
	if (!pilotObject)
		return JNI_FALSE;

	//-- Ensure that the pilot is not already piloting a ship.
	if (pilotObject->getPilotedShip())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::pilotShip(): pilot id=[%s] is already riding a ship.", pilotObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	//-- Get the pilot slot object.
	ServerObject * pilotSlotObject = 0;
	if (!JavaLibrary::getObject(pilotSlotId, pilotSlotObject))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::pilotShip: could not resolve pilotSlotObject"));
		return JNI_FALSE;
	}

	//-- Make sure the ship is a player controlled ship
	if (pilotObject->isPlayerControlled())
	{
		//-- Get the containing ship
		ShipObject * const shipObject = ShipObject::getContainingShipObject(pilotSlotObject);
		if (!shipObject)
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::pilotShip: pilotSlotObject [id=%s, template=%s] does not have a containing ship", pilotSlotObject->getNetworkId().getValueString().c_str(), pilotSlotObject->getObjectTemplateName()));
			return JNI_FALSE;
		}

		if (!shipObject->isPlayerShip())
			return JNI_FALSE;
	}

	//-- Have the pilot mount the ship.
	if (!pilotObject->pilotShip(*pilotSlotObject))
		return JNI_FALSE;

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::unpilotShip(JNIEnv *env, jobject /*self*/, jlong pilotId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	//-- Get the pilot.
	CreatureObject * const pilotObject = JavaLibrary::getCreatureThrow(env, pilotId, "unpilotShip(): error in pilotId arg");
	if (!pilotObject)
		return JNI_FALSE;

	//-- Get the ship.
	ShipObject * const shipObject = pilotObject->getPilotedShip();
	if (!shipObject)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::unpilotShip(): pilot id=[%s] is not piloting a ship.", pilotObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	//-- Detach the pilot from the ship.
	if (!pilotObject->unpilotShip())
		return JNI_FALSE;

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsShipNamespace::getShipPilot(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0;

	//-- Get the ship object.
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipPilot(): obj_id shipId did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0;

	//-- Get the pilot object
	CreatureObject const * const pilotObject = shipObject->getPilot();
	NetworkId const pilotNetworkId = (pilotObject ? pilotObject->getNetworkId() : NetworkId::cms_invalid);

	return pilotNetworkId.getValue();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsShipNamespace::getPilotedShip(JNIEnv * env, jobject /*self*/, jlong jobject_pilotId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0;

	//-- Get the pilot object.
	CreatureObject const * const creatureObject = JavaLibrary::getCreatureThrow(env, jobject_pilotId, "getPilotedShip(): obj_id pilotId did not resolve to a ShipObject", false);
	if (!creatureObject)
		return 0;

	ShipObject const * const shipObject = creatureObject->getPilotedShip();
	NetworkId const shipNetworkId = (shipObject ? shipObject->getNetworkId() : NetworkId::cms_invalid);

	return shipNetworkId.getValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::getShipHasWings(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipHasWings(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	if (!shipObject->hasWings())
		return JNI_FALSE;

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipCurrentSpeed(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipCurrentSpeed(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getCurrentSpeed();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipSlideDampener(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat jfloat_slideDampener)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "setShipSlideDampener(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	bool const result = shipObject->setSlideDampener(jfloat_slideDampener);
	if (!result)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipSlideDampener(): could not set value %1.2f (possibly out of range)", jfloat_slideDampener));
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipSlideDampener(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipSlideDampener(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getSlideDampener();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipCurrentYaw(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat jfloat_currentYaw)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "setShipCurrentYaw(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	bool const result = shipObject->setCurrentYaw(jfloat_currentYaw);
	if (!result)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipCurrentYaw(): could not set value %1.2f (possibly out of range)", jfloat_currentYaw));
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipCurrentYaw(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipCurrentYaw(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getCurrentYaw();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipCurrentPitch(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat jfloat_currentPitch)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "setShipCurrentPitch(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	bool const result = shipObject->setCurrentPitch(jfloat_currentPitch);
	if (!result)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipCurrentPitch(): could not set value %1.2f (possibly out of range)", jfloat_currentPitch));
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipCurrentPitch(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipCurrentPitch(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getCurrentPitch();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipCurrentRoll(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat jfloat_currentRoll)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "setShipCurrentRoll(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	bool const result = shipObject->setCurrentRoll(jfloat_currentRoll);
	if (!result)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipCurrentRoll(): could not set value %1.2f (possibly out of range)", jfloat_currentRoll));
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipCurrentRoll(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipCurrentRoll(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getCurrentRoll();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipCurrentChassisHitPoints(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat jfloat_currentChassisHitPoints)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "setShipCurrentChassisHitPoints(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	bool const result = shipObject->setCurrentChassisHitPoints(jfloat_currentChassisHitPoints);
	if (!result)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipCurrentChassisHitPoints(): could not set value %1.2f (possibly out of range)", jfloat_currentChassisHitPoints));
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipCurrentChassisHitPoints(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipCurrentChassisHitPoints(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getCurrentChassisHitPoints();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipMaximumChassisHitPoints(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat jfloat_currentChassisHitPoints)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "setShipMaximumChassisHitPoints(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	bool const result = shipObject->setMaximumChassisHitPoints(jfloat_currentChassisHitPoints);
	if (!result)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipMaximumChassisHitPoints(): could not set value %1.2f (possibly out of range)", jfloat_currentChassisHitPoints));
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipMaximumChassisHitPoints(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipMaximumChassisHitPoints(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getMaximumChassisHitPoints();
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//-- component system
//----------------------------------------------------------------------
//----------------------------------------------------------------------

//-- ship component methods

jfloat JNICALL ScriptMethodsShipNamespace::getShipActualAccelerationRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipActualAccelerationRate(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getShipActualAccelerationRate();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipActualDecelerationRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipActualDecelerationRate(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getShipActualDecelerationRate();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipActualPitchAccelerationRateDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipActualPitchAccelerationRate(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return convertRadiansToDegrees(shipObject->getShipActualPitchAccelerationRate());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipActualYawAccelerationRateDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipActualYawAccelerationRate(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return convertRadiansToDegrees(shipObject->getShipActualYawAccelerationRate());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipActualRollAccelerationRateDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipActualRollAccelerationRate(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return convertRadiansToDegrees(shipObject->getShipActualRollAccelerationRate());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipActualPitchRateMaximumDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipActualPitchRateMaximum(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return convertRadiansToDegrees(shipObject->getShipActualPitchRateMaximum());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipActualYawRateMaximumDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipActualYawRateMaximum(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return convertRadiansToDegrees(shipObject->getShipActualYawRateMaximum());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipActualRollRateMaximumDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipActualRollRateMaximum(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return convertRadiansToDegrees(shipObject->getShipActualRollRateMaximum());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipActualSpeedMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipActualSpeedMaximum(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.f;

	return shipObject->getShipActualSpeedMaximum();
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsShipNamespace::getShipChassisType(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return nullptr;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipChassisType(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return nullptr;

	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (shipObject->getChassisType());
	if (shipChassis)
		return JavaString(shipChassis->getName ().getString ()).getReturnValue();

	return nullptr;
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getChassisComponentMassCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.0f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipChassisType(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.0f;

	return shipObject->getChassisComponentMassCurrent();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getChassisComponentMassMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.0f;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipChassisType(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0.0f;

	return shipObject->getChassisComponentMassMaximum();
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setChassisComponentMassMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, float chassisComponentMassMaximum)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipChassisType(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	shipObject->setChassisComponentMassMaximum(chassisComponentMassMaximum);

	return JNI_TRUE;
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::isShipSlotInstalled(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return false;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "JavaLibrary::isShipSlotInstalled ship obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return false;

	return shipObject->isSlotInstalled(chassisSlot);
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::isShipComponentDisabled(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "isShipComponentDisabled() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->isComponentDisabled(chassisSlot);
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::isShipComponentDisabledNeedsPower(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "isShipComponentDisabledNeedsPower() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->isComponentDisabledNeedsPower(chassisSlot);
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::isShipComponentLowPower(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "isShipComponentLowPower() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->isComponentLowPower(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipComponentArmorHitpointsMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentArmorHitpointsMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getComponentArmorHitpointsMaximum(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipComponentArmorHitpointsCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentArmorHitpointsCurrent() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getComponentArmorHitpointsCurrent(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipComponentEfficiencyGeneral(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentEfficiencyGeneral() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getComponentEfficiencyGeneral(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipComponentEfficiencyEnergy(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentEfficiencyEnergy() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getComponentEfficiencyEnergy(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipComponentEnergyMaintenanceRequirement(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentEnergyMaintenanceRequirement() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getComponentEnergyMaintenanceRequirement(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipComponentMass(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentMass() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getComponentMass(chassisSlot);
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getShipComponentCrc(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentCrc() invalid ship", false);
	if (!shipObject)
		return false;

	return static_cast<int>(shipObject->getComponentCrc(chassisSlot));
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipComponentHitpointsCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentHitpointsCurrent() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getComponentHitpointsCurrent(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipComponentHitpointsMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentHitpointsMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getComponentHitpointsMaximum(chassisSlot);
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getShipComponentFlags(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentFlags() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getComponentFlags(chassisSlot);
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsShipNamespace::getShipComponentName(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentName() invalid ship", false);
	if (!shipObject)
		return nullptr;

	Unicode::String const & name = shipObject->getComponentName(chassisSlot);
	if (!name.empty())
		return JavaString(name).getReturnValue();

	ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc(shipObject->getComponentCrc(chassisSlot));
	if (shipComponentDescriptor)
	{
		ObjectTemplate const * const t = shipComponentDescriptor->getSharedObjectTemplate();
		if (t)
		{
			SharedObjectTemplate const * const sharedTemplate = t->asSharedObjectTemplate();
			if (sharedTemplate)
				return JavaString("@"+sharedTemplate->getObjectName().getCanonicalRepresentation()).getReturnValue();
		}
	}
	return JavaString("").getReturnValue();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipWeaponDamageMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponDamageMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponDamageMaximum(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipWeaponDamageMinimum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponDamageMinimum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponDamageMinimum(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipWeaponEffectivenessShields(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponEffectivenessShields() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponEffectivenessShields(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipWeaponEffectivenessArmor(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponEffectivenessArmor() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponEffectivenessArmor(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipWeaponEnergyPerShot(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponEnergyPerShot() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponEnergyPerShot(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipWeaponRefireRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponRefireRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponRefireRate(chassisSlot);
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipWeaponActualRefireRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponActualRefireRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponActualRefireRate(chassisSlot);
}
//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipWeaponEfficiencyRefireRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponEfficiencyRefireRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponEfficiencyRefireRate(chassisSlot);
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getShipWeaponAmmoCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponAmmoCurrent() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponAmmoCurrent(chassisSlot);
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getShipWeaponAmmoMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponAmmoMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getWeaponAmmoMaximum(chassisSlot);
}


//----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getShipWeaponAmmoType(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipWeaponAmmoType() invalid ship", false);
	if (!shipObject)
		return false;

	return static_cast<int>(shipObject->getWeaponAmmoType(chassisSlot));
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipShieldHitpointsFrontCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipShieldHitpointsFrontCurrent() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getShieldHitpointsFrontCurrent();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipShieldHitpointsFrontMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipShieldHitpointsFrontMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getShieldHitpointsFrontMaximum();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipShieldHitpointsBackCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipShieldHitpointsBackCurrent() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getShieldHitpointsBackCurrent();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipShieldHitpointsBackMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipShieldHitpointsBackMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getShieldHitpointsBackMaximum();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipShieldRechargeRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipShieldRechargeRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getShieldRechargeRate();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipCapacitorEnergyCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipCapacitorEnergyCurrent() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getCapacitorEnergyCurrent();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipCapacitorEnergyMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipCapacitorEnergyMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getCapacitorEnergyEffectiveMaximum();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipCapacitorEnergyRechargeRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipCapacitorEnergyRechargeRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getCapacitorEnergyRechargeRate();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineAccelerationRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineAccelerationRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getEngineAccelerationRate();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineDecelerationRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineDecelerationRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getEngineDecelerationRate();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEnginePitchAccelerationRateDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEnginePitchAccelerationRateDegrees() invalid ship", false);
	if (!shipObject)
		return false;

	return convertRadiansToDegrees(shipObject->getEnginePitchAccelerationRate());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineYawAccelerationRateDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineYawAccelerationRateDegrees() invalid ship", false);
	if (!shipObject)
		return false;

	return convertRadiansToDegrees(shipObject->getEngineYawAccelerationRate());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineRollAccelerationRateDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineRollAccelerationRateDegrees() invalid ship", false);
	if (!shipObject)
		return false;

	return convertRadiansToDegrees(shipObject->getEngineRollAccelerationRate());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEnginePitchRateMaximumDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEnginePitchRateMaximumDegrees() invalid ship", false);
	if (!shipObject)
		return false;

	return convertRadiansToDegrees(shipObject->getEnginePitchRateMaximum());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineYawRateMaximumDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineYawRateMaximumDegrees() invalid ship", false);
	if (!shipObject)
		return false;

	return convertRadiansToDegrees(shipObject->getEngineYawRateMaximum());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineRollRateMaximumDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineRollRateMaximumDegrees() invalid ship", false);
	if (!shipObject)
		return false;

	return convertRadiansToDegrees(shipObject->getEngineRollRateMaximum());
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineSpeedMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineSpeedMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getEngineSpeedMaximum();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineSpeedRotationFactorMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineSpeedRotationFactorMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getEngineSpeedRotationFactorMaximum();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineSpeedRotationFactorMinimum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineSpeedRotationFactorMinimum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getEngineSpeedRotationFactorMinimum();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipEngineSpeedRotationFactorOptimal(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipEngineSpeedRotationFactorOptimal() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getEngineSpeedRotationFactorOptimal();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipReactorEnergyGenerationRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipReactorEnergyGenerationRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getReactorEnergyGenerationRate();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipBoosterEnergyCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipBoosterEnergyCurrent() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getBoosterEnergyCurrent();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipBoosterEnergyMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipBoosterEnergyMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getBoosterEnergyMaximum();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipBoosterEnergyRechargeRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipBoosterEnergyRechargeRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getBoosterEnergyRechargeRate();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipBoosterEnergyConsumptionRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipBoosterEnergyConsumptionRate() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getBoosterEnergyConsumptionRate();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipBoosterAcceleration(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipBoosterAcceleration() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getBoosterAcceleration();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipBoosterSpeedMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipBoosterSpeedMaximum() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getBoosterSpeedMaximum();
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::isShipBoosterActive(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "isShipBoosterActive() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->isBoosterActive();
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipDroidInterfaceCommandSpeed(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipDroidInterfaceCommandSpeed() invalid ship", false);
	if (!shipObject)
		return false;

	return shipObject->getDroidInterfaceCommandSpeed();
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// component setters
//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentArmorHitpointsMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat componentArmorHitpointsMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentArmorHitpointsMaximum()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentArmorHitpointsMaximum(chassisSlot, componentArmorHitpointsMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentArmorHitpointsMaximum (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentArmorHitpointsCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat componentArmorHitpointsCurrent)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentArmorHitpointsCurrent()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentArmorHitpointsCurrent(chassisSlot, componentArmorHitpointsCurrent))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentArmorHitpointsCurrent (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentEfficiencyGeneral(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat componentEfficiencyGeneral)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentEfficiencyGeneral()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentEfficiencyGeneral(chassisSlot, componentEfficiencyGeneral))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentEfficiencyGeneral (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentEfficiencyEnergy(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat componentEfficiencyEnergy)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentEfficiencyEnergy()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentEfficiencyEnergy(chassisSlot, componentEfficiencyEnergy))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentEfficiencyEnergy (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentEnergyMaintenanceRequirement(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat componentEnergyMaintenanceRequirement)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentEnergyMaintenanceRequirement()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentEnergyMaintenanceRequirement(chassisSlot, componentEnergyMaintenanceRequirement))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentEnergyMaintenanceRequirement (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentMass(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat componentMass)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentMass()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentMass(chassisSlot, componentMass))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentMass (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentHitpointsCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat componentHitpointsCurrent)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentHitpointsCurrent()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentHitpointsCurrent(chassisSlot, componentHitpointsCurrent))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentHitpointsCurrent (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentHitpointsMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat componentHitpointsMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentHitpointsMaximum()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentHitpointsMaximum(chassisSlot, componentHitpointsMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentHitpointsMaximum (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentFlags(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jint componentFlags)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentFlags()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentFlags(chassisSlot, componentFlags))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentFlags (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentName(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jstring componentName)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentName()", false, chassisSlot);
	if (!shipObject)
		return;

	if (componentName == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentName (): nullptr name"));
		return;
	}

	JavaStringParam const jsp(componentName);
	Unicode::String nameString;
	if (!JavaLibrary::convert(jsp, nameString))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentName (): failed to convert name string"));
	else
	{
		if (!shipObject->setComponentName(chassisSlot, nameString))
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentName (): component at slot [%d] error", chassisSlot));
	}
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentDisabled(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jboolean componentDisabled)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentDisabled()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentDisabled(chassisSlot, componentDisabled == JNI_TRUE))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentDisabled                     (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentDisabledNeedsPower(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jboolean componentDisabledNeedsPower)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentDisabledNeedsPower()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentDisabledNeedsPower(chassisSlot, componentDisabledNeedsPower == JNI_TRUE))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentDisabledNeedsPower(): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipComponentLowPower(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jboolean componentLowPower)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipComponentLowPower()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setComponentLowPower(chassisSlot, componentLowPower == JNI_TRUE))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentLowPower                     (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponDamageMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat weaponDamageMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponDamageMaximum()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponDamageMaximum(chassisSlot, weaponDamageMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponDamageMaximum (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponDamageMinimum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat weaponDamageMinimum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponDamageMinimum()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponDamageMinimum(chassisSlot, weaponDamageMinimum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponDamageMinimum (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponEffectivenessShields(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat weaponEffectivenessShields)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponEffectivenessShields()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponEffectivenessShields(chassisSlot, weaponEffectivenessShields))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponEffectivenessShields (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponEffectivenessArmor(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat weaponEffectivenessArmor)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponEffectivenessArmor()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponEffectivenessArmor(chassisSlot, weaponEffectivenessArmor))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponEffectivenessArmor (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponEnergyPerShot(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat weaponEnergyPerShot)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponEnergyPerShot()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponEnergyPerShot(chassisSlot, weaponEnergyPerShot))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponEnergyPerShot (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponRefireRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat weaponRefireRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponRefireRate()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponRefireRate(chassisSlot, weaponRefireRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponRefireRate (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponEfficiencyRefireRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jfloat weaponEfficiencyRefireRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponEfficiencyRefireRate()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponEfficiencyRefireRate(chassisSlot, weaponEfficiencyRefireRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponEfficiencyRefireRate (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponAmmoCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jint weaponAmmoCurrent)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponAmmoCurrent()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponAmmoCurrent(chassisSlot, weaponAmmoCurrent))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponAmmoCurrent (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponAmmoMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jint weaponAmmoMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponAmmoMaximum()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponAmmoMaximum(chassisSlot, weaponAmmoMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponAmmoMaximum (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipWeaponAmmoType(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jint weaponAmmoType)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipWeaponAmmoType()", false, chassisSlot);
	if (!shipObject)
		return;

	if (!shipObject->setWeaponAmmoType(chassisSlot, static_cast<uint32>(weaponAmmoType)))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWeaponAmmoType (): component at slot [%d] error", chassisSlot));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipShieldHitpointsFrontCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat shieldHitpointsFrontCurrent)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipShieldHitpointsFrontCurrent()", false, ShipChassisSlotType::SCST_shield_0);
	if (!shipObject)
		return;

	if (!shipObject->setShieldHitpointsFrontCurrent(shieldHitpointsFrontCurrent))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipShieldHitpointsFrontCurrent (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipShieldHitpointsFrontMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat shieldHitpointsFrontMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipShieldHitpointsFrontMaximum()", false, ShipChassisSlotType::SCST_shield_0);
	if (!shipObject)
		return;

	if (!shipObject->setShieldHitpointsFrontMaximum(shieldHitpointsFrontMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipShieldHitpointsFrontMaximum (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipShieldHitpointsBackCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat shieldHitpointsBackCurrent)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipShieldHitpointsBackCurrent()", false, ShipChassisSlotType::SCST_shield_0);
	if (!shipObject)
		return;

	if (!shipObject->setShieldHitpointsBackCurrent(shieldHitpointsBackCurrent))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipShieldHitpointsBackCurrent (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipShieldHitpointsBackMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat shieldHitpointsBackMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipShieldHitpointsBackMaximum()", false, ShipChassisSlotType::SCST_shield_0);
	if (!shipObject)
		return;

	if (!shipObject->setShieldHitpointsBackMaximum(shieldHitpointsBackMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipShieldHitpointsBackMaximum (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipShieldRechargeRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat shieldRechargeRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipShieldRechargeRate()", false, ShipChassisSlotType::SCST_shield_0);
	if (!shipObject)
		return;

	if (!shipObject->setShieldRechargeRate(shieldRechargeRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipShieldRechargeRate (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipCapacitorEnergyCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat capacitorEnergyCurrent)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipCapacitorEnergyCurrent()", false, ShipChassisSlotType::SCST_capacitor);
	if (!shipObject)
		return;

	if (!shipObject->setCapacitorEnergyCurrent(capacitorEnergyCurrent))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipCapacitorEnergyCurrent (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipCapacitorEnergyMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat capacitorEnergyMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipCapacitorEnergyMaximum()", false, ShipChassisSlotType::SCST_capacitor);
	if (!shipObject)
		return;

	if (!shipObject->setCapacitorEnergyMaximum(capacitorEnergyMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipCapacitorEnergyMaximum (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipCapacitorEnergyRechargeRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat capacitorEnergyRechargeRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipCapacitorEnergyRechargeRate()", false, ShipChassisSlotType::SCST_capacitor);
	if (!shipObject)
		return;

	if (!shipObject->setCapacitorEnergyRechargeRate(capacitorEnergyRechargeRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipCapacitorEnergyRechargeRate (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineAccelerationRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineAccelerationRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineAccelerationRate()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineAccelerationRate(engineAccelerationRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineAccelerationRate (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineDecelerationRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineDecelerationRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineDecelerationRate()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineDecelerationRate(engineDecelerationRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineDecelerationRate (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEnginePitchAccelerationRateDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat enginePitchAccelerationRateDegrees)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEnginePitchAccelerationRateDegrees()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEnginePitchAccelerationRate(convertDegreesToRadians(enginePitchAccelerationRateDegrees)))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEnginePitchAccelerationRateDegrees (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineYawAccelerationRateDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineYawAccelerationRateDegrees)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineYawAccelerationRateDegrees()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineYawAccelerationRate(convertDegreesToRadians(engineYawAccelerationRateDegrees)))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineYawAccelerationRateDegrees (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineRollAccelerationRateDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineRollAccelerationRateDegrees)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineRollAccelerationRateDegrees()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineRollAccelerationRate(convertDegreesToRadians(engineRollAccelerationRateDegrees)))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineRollAccelerationRateDegrees (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEnginePitchRateMaximumDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat enginePitchRateMaximumDegrees)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEnginePitchRateMaximumDegrees()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEnginePitchRateMaximum(convertDegreesToRadians(enginePitchRateMaximumDegrees)))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEnginePitchRateMaximumDegrees (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineYawRateMaximumDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineYawRateMaximumDegrees)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineYawRateMaximumDegrees()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineYawRateMaximum(convertDegreesToRadians(engineYawRateMaximumDegrees)))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineYawRateMaximumDegrees (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineRollRateMaximumDegrees(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineRollRateMaximumDegrees)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineRollRateMaximumDegrees()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineRollRateMaximum(convertDegreesToRadians(engineRollRateMaximumDegrees)))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineRollRateMaximumDegrees (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineSpeedMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineSpeedMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineSpeedMaximum()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineSpeedMaximum(engineSpeedMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineSpeedMaximum (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineSpeedRotationFactorMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineSpeedRotationRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineSpeedRotationFactorMaximum()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineSpeedRotationFactorMaximum(engineSpeedRotationRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineSpeedRotationFactorMaximum(): component error %s", shipObject->getDebugInformation(false).c_str()));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineSpeedRotationFactorMinimum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineSpeedRotationRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineSpeedRotationFactorMinimum()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineSpeedRotationFactorMinimum(engineSpeedRotationRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineSpeedRotationFactorMinimum(): component error %s", shipObject->getDebugInformation(false).c_str()));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipEngineSpeedRotationFactorOptimal(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat engineSpeedRotationRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipEngineSpeedRotationFactorOptimal()", false, ShipChassisSlotType::SCST_engine);
	if (!shipObject)
		return;

	if (!shipObject->setEngineSpeedRotationFactorOptimal(engineSpeedRotationRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipEngineSpeedRotationFactorOptimal(): component error %s", shipObject->getDebugInformation(false).c_str()));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipReactorEnergyGenerationRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat reactorEnergyGenerationRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipReactorEnergyGenerationRate()", false, ShipChassisSlotType::SCST_reactor);
	if (!shipObject)
		return;

	if (!shipObject->setReactorEnergyGenerationRate(reactorEnergyGenerationRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipReactorEnergyGenerationRate (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipBoosterEnergyCurrent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat boosterEnergyCurrent)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipBoosterEnergyCurrent()", false, ShipChassisSlotType::SCST_booster);
	if (!shipObject)
		return;

	if (!shipObject->setBoosterEnergyCurrent(boosterEnergyCurrent))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipBoosterEnergyCurrent (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipBoosterEnergyMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat boosterEnergyMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipBoosterEnergyMaximum()", false, ShipChassisSlotType::SCST_booster);
	if (!shipObject)
		return;

	if (!shipObject->setBoosterEnergyMaximum(boosterEnergyMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipBoosterEnergyMaximum (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipBoosterEnergyRechargeRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat boosterEnergyRechargeRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipBoosterEnergyRechargeRate()", false, ShipChassisSlotType::SCST_booster);
	if (!shipObject)
		return;

	if (!shipObject->setBoosterEnergyRechargeRate(boosterEnergyRechargeRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipBoosterEnergyRechargeRate (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipBoosterEnergyConsumptionRate(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat boosterEnergyConsumptionRate)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipBoosterEnergyConsumptionRate()", false, ShipChassisSlotType::SCST_booster);
	if (!shipObject)
		return;

	if (!shipObject->setBoosterEnergyConsumptionRate(boosterEnergyConsumptionRate))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipBoosterEnergyConsumptionRate (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipBoosterAcceleration(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat boosterAcceleration)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipBoosterAcceleration()", false, ShipChassisSlotType::SCST_booster);
	if (!shipObject)
		return;

	if (!shipObject->setBoosterAcceleration(boosterAcceleration))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipBoosterAcceleration (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipBoosterSpeedMaximum(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat boosterSpeedMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "isShipSlotInstalledActual()", false, ShipChassisSlotType::SCST_booster);
	if (!shipObject)
		return;

	if (!shipObject->setBoosterSpeedMaximum(boosterSpeedMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipBoosterSpeedMaximum (): component error"));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipDroidInterfaceCommandSpeed(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jfloat droidInterfaceCommandSpeed)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipDroidInterfaceCommandSpeed()", false, ShipChassisSlotType::SCST_droid_interface);
	if (!shipObject)
		return;

	if (!shipObject->setDroidInterfaceCommandSpeed(droidInterfaceCommandSpeed))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipDroidInterfaceCommandSpeed (): component error"));
}

//----------------------------------------------------------------------

//-- component support

jboolean JNICALL ScriptMethodsShipNamespace::shipCanInstallComponent(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot, jlong component)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return false;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "shipCanInstallComponent(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return false;

	TangibleObject * tangibleComponent = 0;
	if (JavaLibrary::getObject (component, tangibleComponent) && tangibleComponent != nullptr)
		return shipObject->canInstallComponent(chassisSlot, *tangibleComponent);
	else
	{
		JAVA_THROW_SCRIPT_EXCEPTION (true, ("shipCanInstallComponent(): invalid component"));
		return false;
	}
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::shipPseudoInstallComponent(JNIEnv *env, jobject, jlong jship, jint chassisSlot, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return false;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jship, "shipInstallComponent(): ship obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return false;

	return shipObject->pseudoInstallComponent(static_cast<int>(chassisSlot), static_cast<uint32>(componentCrc));
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::shipInstallComponent(JNIEnv * env, jobject /*self*/, jlong jobject_installerId, jlong jobject_shipId, jint chassisSlot, jlong component)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return false;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "shipInstallComponent(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return false;

	NetworkId installerId(jobject_installerId);

	TangibleObject * tangibleComponent = 0;
	if (JavaLibrary::getObject (component, tangibleComponent) && tangibleComponent != nullptr)
		return shipObject->installComponent(installerId, chassisSlot, *tangibleComponent);
	else
	{
		JAVA_THROW_SCRIPT_EXCEPTION (true, ("shipInstallComponent(): invalid component"));
		return false;
	}
}

//----------------------------------------------------------------------
jlong JNICALL ScriptMethodsShipNamespace::shipUninstallComponentAllowOverload(JNIEnv * env, jobject self, jlong jobject_uninstallerId, jlong jobject_shipId, jint chassisSlot, jlong containerTarget)
{
	return shipUninstallComponentBase(env, self, jobject_uninstallerId, jobject_shipId, chassisSlot, containerTarget, true);
}

jlong JNICALL ScriptMethodsShipNamespace::shipUninstallComponent(JNIEnv * env, jobject self, jlong jobject_uninstallerId, jlong jobject_shipId, jint chassisSlot, jlong containerTarget)
{
	return shipUninstallComponentBase(env, self, jobject_uninstallerId, jobject_shipId, chassisSlot, containerTarget, false);
}

jlong JNICALL ScriptMethodsShipNamespace::shipUninstallComponentBase(JNIEnv * env, jobject /*self*/, jlong jobject_uninstallerId, jlong jobject_shipId, jint chassisSlot, jlong containerTarget, bool allowOverload)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "shipUninstallComponent(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return 0;

	ServerObject * containerTargetObject = 0;

	if (containerTarget)
	{
		if (!JavaLibrary::getObject (containerTarget, containerTargetObject))
		{
			JAVA_THROW_SCRIPT_EXCEPTION (true, ("shipUninstallComponent(): containerTarget did not resolve"));
			return 0;
		}

		NetworkId uninstallerId(jobject_uninstallerId);

		TangibleObject * const component = shipObject->uninstallComponent(uninstallerId, chassisSlot, *containerTargetObject, allowOverload);
		if (component)
			return (component->getNetworkId()).getValue();
		else
		{
			JAVA_THROW_SCRIPT_EXCEPTION (true, ("shipUninstallComponent(): invalid result component"));
			return 0;
		}
	}
	else
	{
		shipObject->purgeComponent (chassisSlot);
		return 0;
	}
}


//----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsShipNamespace::getShipChassisSlots(JNIEnv * env, jobject /*self*/, jlong jobject_shipId)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return nullptr;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipChassisSlots(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return nullptr;

	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (shipObject->getChassisType());
	if (shipChassis)
	{
		ShipChassis::SlotVector const & slots = shipChassis->getSlots ();

		LocalIntArrayRefPtr jslots = createNewIntArray(static_cast<int>(slots.size ()));
		if (jslots != LocalIntArrayRef::cms_nullPtr)
		{
			int index = 0;
			for (ShipChassis::SlotVector::const_iterator it = slots.begin (); it != slots.end (); ++it, ++index)
			{
				ShipChassisSlot const & slot = *it;
				jint value = static_cast<jint>(slot.getSlotType ());
				setIntArrayRegion(*jslots, index, 1, &value);
			}
			return jslots->getReturnValue();
		}
	}

	return nullptr;
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorType(JNIEnv * /*env*/, jobject /*self*/, jlong jobject_component)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return static_cast<int>(ShipComponentType::SCT_num_types);

	TangibleObject * tangibleComponent = 0;
	if (JavaLibrary::getObject (jobject_component, tangibleComponent))
	{
		if (tangibleComponent != nullptr && tangibleComponent->getObjectTemplate () != nullptr)
		{
			ShipComponentDescriptor const * const shipComponentDescriptor =
				ShipComponentDescriptor::findShipComponentDescriptorByObjectTemplate (tangibleComponent->getObjectTemplate ()->getCrcName ().getCrc ());
			if (shipComponentDescriptor)
				return static_cast<int>(shipComponentDescriptor->getComponentType ());
		}
	}

	return static_cast<int>(ShipComponentType::SCT_num_types);
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorTypeByName(JNIEnv * /*env*/, jobject /*self*/, jstring typeName)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return static_cast<int>(ShipComponentType::SCT_num_types);

	JavaStringParam const jsp(typeName);
	std::string nameString;
	if (!JavaLibrary::convert(jsp, nameString))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipComponentDescriptorTypeByName (): failed to convert typeName string"));
	else
	{
		return ShipComponentType::getTypeFromName(nameString);
	}

	return static_cast<int>(ShipComponentType::SCT_num_types);
}

//----------------------------------------------------------------------

jstring    JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorTypeName(JNIEnv * /*env*/, jobject /*self*/, jint componentType)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return nullptr;

	std::string const & typeName = ShipComponentType::getNameFromType(static_cast<ShipComponentType::Type>(componentType));

	return JavaString(typeName).getReturnValue();
}

//----------------------------------------------------------------------

jint       JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorCrc(JNIEnv * /*env*/, jobject /*self*/, jlong jobject_component)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return static_cast<int>(ShipComponentType::SCT_num_types);

	TangibleObject * tangibleComponent = 0;
	if (JavaLibrary::getObject (jobject_component, tangibleComponent))
	{
		if (tangibleComponent != nullptr && tangibleComponent->getObjectTemplate () != nullptr)
		{
			ShipComponentDescriptor const * const shipComponentDescriptor =
				ShipComponentDescriptor::findShipComponentDescriptorByObjectTemplate (tangibleComponent->getObjectTemplate ()->getCrcName ().getCrc ());
			if (shipComponentDescriptor)
				return static_cast<int>(shipComponentDescriptor->getCrc ());
		}
	}

	return 0;
}

//----------------------------------------------------------------------

jstring    JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorCrcName(JNIEnv * /*env*/, jobject /*self*/, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return nullptr;

	ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc(static_cast<uint32>(componentCrc));
	if (shipComponentDescriptor == nullptr)
		return nullptr;

	return JavaString(shipComponentDescriptor->getName().getString()).getReturnValue();
}

//----------------------------------------------------------------------

jstring    JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorCompatibility(JNIEnv * /*env*/, jobject /*self*/, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return nullptr;

	ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc(static_cast<uint32>(componentCrc));
	if (shipComponentDescriptor == nullptr)
		return nullptr;

	return JavaString(shipComponentDescriptor->getCompatibility().getString()).getReturnValue();
}

//----------------------------------------------------------------------

jboolean   JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorWeaponIsAmmoConsuming(JNIEnv * /*env*/, jobject /*self*/, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return false;

	return ShipComponentWeaponManager::isAmmoConsuming(static_cast<uint32>(componentCrc));
}

//----------------------------------------------------------------------

jboolean   JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorWeaponIsMissile(JNIEnv * /*env*/, jobject /*self*/, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return false;

	return ShipComponentWeaponManager::isMissile(static_cast<uint32>(componentCrc));
}

//----------------------------------------------------------------------

jboolean   JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorWeaponIsCountermeasure(JNIEnv * /*env*/, jobject /*self*/, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return false;

	return ShipComponentWeaponManager::isCountermeasure(static_cast<uint32>(componentCrc));
}

//----------------------------------------------------------------------

jboolean   JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorWeaponIsMining(JNIEnv * /*env*/, jobject /*self*/, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return false;

	return ShipComponentWeaponManager::isMining(static_cast<uint32>(componentCrc));
}

//----------------------------------------------------------------------

jboolean   JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorWeaponIsTractor(JNIEnv * /*env*/, jobject /*self*/, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return false;

	return ShipComponentWeaponManager::isTractor(static_cast<uint32>(componentCrc));
}

//----------------------------------------------------------------------

jfloat     JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorWeaponRange(JNIEnv * /*env*/, jobject /*self*/, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.0f;

	return ShipComponentWeaponManager::getRange(static_cast<uint32>(componentCrc));
}

//----------------------------------------------------------------------

jfloat     JNICALL ScriptMethodsShipNamespace::getShipComponentDescriptorWeaponProjectileSpeed(JNIEnv * /*env*/, jobject /*self*/, jint componentCrc)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return 0.0f;

	return ShipComponentWeaponManager::getProjectileSpeed(static_cast<uint32>(componentCrc));
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::notifyShipDamage(JNIEnv * env, jobject /*self*/, jlong jobject_victim, jlong jobject_attacker, jfloat totalDamage)
{
	//-- Make sure ships are enabled
	if (!verifyShipsEnabled())
		return;

	//-- Get the victim ship object.
	ShipObject * const victimShipObject = JavaLibrary::getShipThrow(env, jobject_victim, "notifyShipDamage(): jobject_victim did not resolve to a ShipObject", false);
	if (victimShipObject)
	{
		//-- Ensure we have a player controlled ship object.
		if (victimShipObject->isPlayerShip())
		{
			//-- Get the attacker object.  It can be any tangible object.
			TangibleObject const * attackerTangibleObject = nullptr;
			if (jobject_attacker)
				IGNORE_RETURN(JavaLibrary::getObject(jobject_attacker, attackerTangibleObject));

			Controller * const victimShipController = victimShipObject->getController();
			if (victimShipController)
			{
				ShipDamageMessage shipDamage(attackerTangibleObject != nullptr ? attackerTangibleObject->getNetworkId() : NetworkId::cms_invalid,
					attackerTangibleObject != nullptr ? attackerTangibleObject->getPosition_w() : victimShipObject->getPosition_w(),
					totalDamage );

				//lint -esym(429, damageMessage)

				MessageQueueGenericValueType<ShipDamageMessage> * const damageMessage = new MessageQueueGenericValueType<ShipDamageMessage>(shipDamage);

				victimShipController->appendMessage(CM_shipDamageMessage, 0.0f, damageMessage,
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
	}
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::launchMissile(JNIEnv * /*env*/, jobject /*self*/, jlong jplayer, jlong jship, jlong jtarget, jint missileInfo, jint targetedComponent)
{
	if (!verifyShipsEnabled())
		return false;

	const TangibleObject * player = 0;
	if (!JavaLibrary::getObject (jplayer, player))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::launchMissile: could not resolve player"));
		return JNI_FALSE;
	}

	const TangibleObject * ship = 0;
	if (!JavaLibrary::getObject (jship, ship))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::launchMissile: could not resolve ship"));
		return JNI_FALSE;
	}

	const TangibleObject * target = 0;
	if (!JavaLibrary::getObject (jtarget, target))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::launchMissile: could not resolve target"));
		return JNI_FALSE;
	}

	if (player && ship && target)
		return MissileManager::getInstance().requestFireMissile(player->getNetworkId(), ship->getNetworkId(), target->getNetworkId(), -1, missileInfo, getSlotType(targetedComponent));
	else
		return JNI_FALSE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::launchCountermeasure(JNIEnv * /*env*/, jobject /*self*/, jlong ship, jint targettedMissile, jboolean successful, jint countermeasureType)
{
	if (!verifyShipsEnabled())
		return false;

	NetworkId const shipId(ship);

	bool result= MissileManager::getInstance().fireCountermeasure(shipId, targettedMissile, (successful==static_cast<jboolean>(JNI_TRUE)), countermeasureType);
	return result ? static_cast<jboolean>(JNI_TRUE) : static_cast<jboolean>(JNI_FALSE);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getNearestUnlockedMissileForTarget(JNIEnv * /*env*/, jobject /*self*/, jlong jtarget)
{
	return MissileManager::getInstance().getNearestUnlockedMissileForTarget(NetworkId(jtarget));
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsShipNamespace::getAllUnlockedMissilesForTarget(JNIEnv * env, jobject /*self*/, jlong jtarget)
{
	std::vector<int> missiles;
	MissileManager::getInstance().getAllUnlockedMissilesForTarget(NetworkId(jtarget),missiles);

	LocalIntArrayRefPtr results = createNewIntArray(static_cast<int>(missiles.size()));

	setIntArrayRegion(*results, 0 , static_cast<int>(missiles.size()), reinterpret_cast<jint*>(&(missiles[0])));
	return results->getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::destroyMissile(JNIEnv * /*env*/, jobject /*self*/, jint missileId)
{
	return (MissileManager::getInstance().destroyMissile(missileId)) ? static_cast<jboolean>(JNI_TRUE) : static_cast<jboolean>(JNI_FALSE);
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getTypeByMissile(JNIEnv * /*env*/, jobject /*self*/, jint missileId)
{
	const Missile* missile = MissileManager::getInstance().getConstMissile(missileId);
	if (missile)
		return missile->getType();
	else
		return -1;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getWeaponIndexByMissile(JNIEnv * /*env*/, jobject /*self*/, jint missileId)
{
	const Missile* missile = MissileManager::getInstance().getConstMissile(missileId);
	if (missile)
		return missile->getWeaponIndex();
	else
		return -1;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getTimeSinceFiredByMissile(JNIEnv * /*env*/, jobject /*self*/, jint missileId)
{
	const Missile* missile = MissileManager::getInstance().getConstMissile(missileId);
	if (missile)
		return missile->getTimeSinceFired();
	else
		return -1;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getTotalTimeByMissile(JNIEnv * /*env*/, jobject /*self*/, jint missileId)
{
	const Missile* missile = MissileManager::getInstance().getConstMissile(missileId);
	if (missile)
		return missile->getTotalTime();
	else
		return -1;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsShipNamespace::getWhoFiredByMissile(JNIEnv * /*env*/, jobject /*self*/, jint missileId)
{
	const Missile* missile = MissileManager::getInstance().getConstMissile(missileId);
	if (missile)
		return (missile->getSource()).getValue();
	else
		return 0;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setLookAtTargetShipComponent(JNIEnv * env, jobject /*self*/, jlong jship, jint targetedComponentChassisSlot)
{
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jship, "setLookAtTargetShipComponent(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	shipObject->setPilotLookAtTargetSlot(getSlotType(targetedComponentChassisSlot));
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getLookAtTargetShipComponent(JNIEnv * env, jobject /*self*/, jlong jship)
{
	if (!verifyShipsEnabled())
		return static_cast<int>(ShipChassisSlotType::SCST_num_types);

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jship, "getLookAtTargetShipComponent(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return static_cast<int>(ShipChassisSlotType::SCST_num_types);

	return static_cast<int>(shipObject->getPilotLookAtTargetSlot());
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::associateDroidControlDeviceWithShip(JNIEnv * env, jobject /*self*/, jlong jship, jlong jdroidControlDevice)
{
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jship, "associateDroidControlDeviceWithShip(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	const ServerObject * droidControlDevice = 0;
	if (!JavaLibrary::getObject (jdroidControlDevice, droidControlDevice))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::associateDroidControlDeviceWithShip: could not resolve droid control device"));
		return JNI_FALSE;
	}

	shipObject->setInstalledDroidControlDevice(droidControlDevice->getNetworkId());
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::removeDroidControlDeviceFromShip(JNIEnv * env, jobject /*self*/, jlong jship)
{
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jship, "removeDroidControlDeviceFromShip(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	shipObject->setInstalledDroidControlDevice(NetworkId::cms_invalid);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsShipNamespace::getDroidControlDeviceForShip(JNIEnv * env, jobject /*self*/, jlong jship)
{
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jship, "getDroidControlDeviceForShip(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	NetworkId const & droidControlDeviceId = shipObject->getInstalledDroidControlDevice();
	Object const * const droidControlDevice = NetworkIdManager::getObjectById(droidControlDeviceId);
	ServerObject const * const serverDroidControlDevice = droidControlDevice ? droidControlDevice->asServerObject() : nullptr;
	if(!serverDroidControlDevice)
		return 0;
	else
		return (serverDroidControlDevice->getNetworkId()).getValue();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::commPlayers(JNIEnv *env, jobject /*self*/, jlong jtaunter, jstring jappearanceOverloadServerTemplate, jstring jSoundEffect, jfloat jDuration, jlongArray jplayers, jstring jtauntOob, jboolean chronicles)
{
	ServerObject const *taunter = 0;
	if (!JavaLibrary::getObject(jtaunter, taunter))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::commPlayers: could not resolve taunter"));
		return;
	}

	Unicode::String oob;
	if (jtauntOob)
	{
		JavaStringParam localTauntOob(jtauntOob);
		if (!JavaLibrary::convert(localTauntOob, oob))
		{
			JAVA_THROW_SCRIPT_EXCEPTION (true, ("JavaLibrary::commPlayers unable to convert tauntOob"));
			return;
		}
	}

	Unicode::String appearanceOverloadServerTemplateWide;
	if (jappearanceOverloadServerTemplate)
	{
		JavaStringParam localAppearanceOverloadServerTemplate(jappearanceOverloadServerTemplate);
		if (!JavaLibrary::convert(localAppearanceOverloadServerTemplate, appearanceOverloadServerTemplateWide))
		{
			JAVA_THROW_SCRIPT_EXCEPTION (true, ("JavaLibrary::commPlayers unable to convert jappearanceOverloadServerTemplate to a string"));
			return;
		}
	}

	uint32 appearanceOverloadSharedTemplateCrc = 0;
	ObjectTemplate const * const ot = ObjectTemplateList::fetch(Unicode::wideToNarrow(appearanceOverloadServerTemplateWide));
	ServerObjectTemplate const * const sot = ot ? ot->asServerObjectTemplate() : nullptr;
	if(sot)
	{
		std::string const & sharedTemplateName = sot->getSharedTemplate();
		appearanceOverloadSharedTemplateCrc = Crc::calculate(sharedTemplateName.c_str());
	}

	typedef std::map<ConnectionServerConnection *, std::vector<NetworkId> > DistributionList;
	DistributionList distributionList;

	jsize const count = env->GetArrayLength(jplayers);
	jlong jlongTmp;
	for (jsize i = 0; i < count; ++i)
	{
		ServerObject const *player = 0;
		env->GetLongArrayRegion(jplayers, i, 1, &jlongTmp);
		if (JavaLibrary::getObject(jlongTmp, player))
		{
			Client * const client = player->getClient();
			if (client)
				distributionList[client->getConnection()].push_back(client->getCharacterObjectId());
		}
	}

	std::string soundEffect;
	if (jSoundEffect)
	{
		JavaStringParam soundEffectParam(jSoundEffect);
		if (!JavaLibrary::convert(soundEffectParam, soundEffect))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::commPlayers unable to convert jSoundEffect to a string."));
			return;
		}
	}

	bool chroniclesMessage = (chronicles == JNI_TRUE ? true : false);
	if (!distributionList.empty())
	{
		GenericValueTypeMessage<std::pair<bool, std::pair<NetworkId, std::pair<Unicode::String, std::pair<uint32, std::pair<std::string, float> > > > > > const message("CommPlayerMessage", std::make_pair(chroniclesMessage, std::make_pair(taunter->getNetworkId(), std::make_pair(oob, std::make_pair(appearanceOverloadSharedTemplateCrc, std::make_pair(soundEffect, jDuration))))));
		for (DistributionList::const_iterator iter = distributionList.begin(); iter != distributionList.end(); ++iter)
		{
			GameClientMessage const gcm(
				(*iter).second,
				true,
				message);
			(*iter).first->send(gcm, true);
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipSlotTargetable(JNIEnv * env, jobject /*self*/, jlong jshipId, jint slot, jboolean targetable)
{
	if (!verifyShipsEnabled())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentTargetable ships not enabled"));
		return;
	}

	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jshipId, "setShipComponentTargetable(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return;

	if(targetable)
		shipObject->setSlotTargetable(getSlotType(slot));
	else
		shipObject->clearSlotTargetable(getSlotType(slot));
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::isShipSlotTargetable(JNIEnv * env, jobject /*self*/, jlong jshipId, jint slot)
{
	if (!verifyShipsEnabled())
		return JNI_FALSE;

	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, jshipId, "getShipComponentTargetable(): shipId obj_id did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	bool const result = shipObject->getSlotTargetable(getSlotType(slot));
	if(result)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

//--------------------------------------------------------------------------------

jlong JNICALL ScriptMethodsShipNamespace::launchShipFromHangar(JNIEnv *env, jobject /*self*/, jlong jlaunchingShipId, jstring templateToSpawn, jobject deltaFromHangarHardpoint)
{
	if (!verifyShipsEnabled())
		return 0;

	//convert ship
	ShipObject * const launchingShip = JavaLibrary::getShipThrow(env, jlaunchingShipId, "getShipComponentTargetable(): shipId obj_id did not resolve to a ShipObject", false);
	if (!launchingShip)
		return 0;

	//convert transform
	Transform deltaFromHangarTransform;
	if (!ScriptConversion::convert(deltaFromHangarHardpoint, deltaFromHangarTransform))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("[designer bug] JavaLibrary::spawnShipFromHangar called with bad transform"));
		return 0;
	}

	// get the template name into an std::string
	JavaStringParam const localSource(templateToSpawn);
	std::string templateName;
	IGNORE_RETURN(JavaLibrary::convert(localSource, templateName));

	ConstCharCrcString const crcName(ObjectTemplateList::lookUp(templateName.c_str()));
	if (crcName.getCrc() == 0)

	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("[designer bug] JavaLibrary::spawnShipFromHangar called with unknown template %s", templateName.c_str()));
		return 0;
	}

	//put spawned ship in same cell as the launching ship
	ServerObject *cell = 0;
	CellProperty * const cellProperty = launchingShip->getParentCell();
	if (cellProperty && cellProperty != CellProperty::getWorldCellProperty())
		cell = safe_cast<ServerObject *>(&cellProperty->getOwner());

	//get the hardpoint from the launching ship
	Transform hangarHardpointDelta;
	Appearance const * const launchingShipAppearance = launchingShip->getAppearance();
	if(launchingShipAppearance)
	{
		static char const * const hangarHardpointName = "hangar";
		if (!launchingShipAppearance->findHardpoint(ConstCharCrcLowerString(hangarHardpointName), hangarHardpointDelta))
			WARNING(true, ("JavaLibrary::launchShipFromHangar - could not find hangar hardpoint [%s] for %s, launching from center of ship", hangarHardpointName, launchingShip->getNetworkId().getValueString().c_str()));
	}
	else
	{
		WARNING(true, ("JavaLibrary::launchShipFromHangar - could not find hangar %s, which has no appearance.", launchingShip->getNetworkId().getValueString().c_str()));
	}

	//build the final hangar offset from the hangar hardpoint transform and the delta transform from script
	Transform finalHangarDelta_o(Transform::IF_none);
	finalHangarDelta_o.multiply(hangarHardpointDelta, deltaFromHangarTransform);
	Transform const & finalCreateTransform = launchingShip->getTransform_o2p().rotateTranslate_l2p(finalHangarDelta_o);

	ServerObject * const newShip = ServerWorld::createNewObject(crcName.getCrc(), finalCreateTransform, cell, false);
	if (newShip == nullptr)
		return 0;

	// create an objId to return
	NetworkId netId = newShip->getNetworkId();
	if (netId == NetworkId::cms_invalid)
	{
		delete newShip;
		return 0;
	}

	if (!cell)
		newShip->addToWorld();

	return netId.getValue();
}

//----------------------------------------------------------------------

void       JNICALL ScriptMethodsShipNamespace::handleShipDestruction(JNIEnv * env, jobject /*self*/, jlong jship, jfloat severity)
{
	if (!verifyShipsEnabled())
		return;

	//convert ship
	ShipObject * const ship = JavaLibrary::getShipThrow(env, jship, "handleShipDestruction(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship == nullptr)
		return;

	DestroyShipMessage const msg(ship->getNetworkId(), severity);
	sendGameNetworkMessageToObservers(*ship, msg, true);

	if (ship->isPlayerShip())
		Pvp::removeDuelEnemyFlags(*ship, NetworkId::cms_invalid);
	else
		IGNORE_RETURN(ship->permanentlyDestroy(DeleteReasons::Script));

	//@todo: we must handle player ship destruction differently
}

//----------------------------------------------------------------------

void       JNICALL ScriptMethodsShipNamespace::handleShipComponentDestruction(JNIEnv * env, jobject /*self*/, jlong jship, jint chassisSlot, jfloat severity)
{
	ShipObject * const ship = getShipThrowTestSlot(env, jship, "handleShipComponentDestruction()", false, chassisSlot);
	if (!ship)
		return;

	DestroyShipComponentMessage const msg(ship->getNetworkId(), chassisSlot, severity);
	sendGameNetworkMessageToObservers(*ship, msg, true);

	ship->purgeComponent(chassisSlot);
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsShipNamespace::getObservedShips(JNIEnv *, jobject, jlong jsource, jboolean excludeSource)
{
	ServerObject const *source = 0;
	if (JavaLibrary::getObject(jsource, source))
	{
		std::vector<ServerObject *> visibleObjects;
		SpaceVisibilityManager::getObjectsVisibleFromLocation(source->getPosition_w(), visibleObjects);
		if (!visibleObjects.empty())
		{
			std::vector<ServerObject *> results(visibleObjects.size());
			for (std::vector<ServerObject *>::const_iterator i = visibleObjects.begin(); i != visibleObjects.end(); ++i)
				if (   (*i)->asShipObject()
				    && ((*i) != source || !excludeSource))
					results.push_back(*i);
			if (!results.empty())
			{
				LocalLongArrayRefPtr rv;
				if (ScriptConversion::convert(results, rv))
					return rv->getReturnValue();
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsShipNamespace::getObservedPlayerShips(JNIEnv *, jobject, jlong jsource, jboolean excludeSource)
{
	ServerObject const *source = 0;
	if (JavaLibrary::getObject(jsource, source))
	{
		std::vector<ServerObject *> visibleObjects;
		SpaceVisibilityManager::getObjectsVisibleFromLocation(source->getPosition_w(), visibleObjects);
		if (!visibleObjects.empty())
		{
			std::vector<ServerObject *> results(visibleObjects.size());
			for (std::vector<ServerObject *>::const_iterator i = visibleObjects.begin(); i != visibleObjects.end(); ++i)
			{
				ShipObject * const ship = (*i)->asShipObject();
				if (   ship
				    && ship->isPlayerShip()
				    && ((*i) != source || !excludeSource))
					results.push_back(*i);
			}
			if (!results.empty())
			{
				LocalLongArrayRefPtr rv;
				if (ScriptConversion::convert(results, rv))
					return rv->getReturnValue();
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsShipNamespace::getObservedEnemyShips(JNIEnv *, jobject, jlong jsource)
{
	ShipObject const *source = 0;
	if (JavaLibrary::getObject(jsource, source))
	{
		std::vector<ServerObject *> visibleObjects;
		SpaceVisibilityManager::getObjectsVisibleFromLocation(source->getPosition_w(), visibleObjects);
		if (!visibleObjects.empty())
		{
			std::vector<ServerObject *> results(visibleObjects.size());
			for (std::vector<ServerObject *>::const_iterator i = visibleObjects.begin(); i != visibleObjects.end(); ++i)
			{
				ShipObject * const ship = (*i)->asShipObject();
				if (   ship
				    && Pvp::isEnemy(*source, *ship))
					results.push_back(*i);
			}
			if (!results.empty())
			{
				LocalLongArrayRefPtr rv;
				if (ScriptConversion::convert(results, rv))
					return rv->getReturnValue();
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsShipNamespace::getObservedEnemyPlayerShips(JNIEnv *, jobject, jlong jsource)
{
	ShipObject const *source = 0;
	if (JavaLibrary::getObject(jsource, source))
	{
		std::vector<ServerObject *> visibleObjects;
		SpaceVisibilityManager::getObjectsVisibleFromLocation(source->getPosition_w(), visibleObjects);
		if (!visibleObjects.empty())
		{
			std::vector<ServerObject *> results(visibleObjects.size());
			for (std::vector<ServerObject *>::const_iterator i = visibleObjects.begin(); i != visibleObjects.end(); ++i)
			{
				ShipObject * const ship = (*i)->asShipObject();
				if (   ship
				    && ship->isPlayerShip()
				    && Pvp::isEnemy(*source, *ship))
					results.push_back(*i);
			}
			if (!results.empty())
			{
				LocalLongArrayRefPtr rv;
				if (ScriptConversion::convert(results, rv))
					return rv->getReturnValue();
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipInternalDamageOverTime(JNIEnv * env, jobject, jlong jship, jint chassisSlot, jfloat damageRate, jfloat damageThreshold)
{
	ShipObject * const ship = getShipThrowTestSlot(env, jship, "setShipInternalDamageOverTime()", false, chassisSlot);
	if (!ship)
		return false;

	return ShipInternalDamageOverTimeManager::setEntry(*ship, chassisSlot, damageRate, damageThreshold);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::removeShipInternalDamageOverTime(JNIEnv * env, jobject, jlong jship, jint chassisSlot)
{
	ShipObject * const ship = getShipThrowTestSlot(env, jship, "removeShipInternalDamageOverTime()", false, chassisSlot);
	if (!ship)
		return false;

	return ShipInternalDamageOverTimeManager::removeEntry(*ship, chassisSlot);
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipInternalDamageOverTimeDamageRate(JNIEnv * env, jobject, jlong jship, jint chassisSlot)
{
	ShipObject * const ship = getShipThrowTestSlot(env, jship, "getShipInternalDamageOverTimeDamageThreshold()", false, chassisSlot);
	if (!ship)
		return 0.0f;

	static ShipInternalDamageOverTime const * const idot = ShipInternalDamageOverTimeManager::findEntry(*ship, chassisSlot);
	if (idot == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipInternalDamageOverTimeDamageThreshold idot for [%s] slot [%d] not found", ship->getNetworkId().getValueString().c_str(), chassisSlot));
		return 0.0f;
	}

	return idot->getDamageRate();
}

// ----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipInternalDamageOverTimeDamageThreshold(JNIEnv * env, jobject, jlong jship, jint chassisSlot)
{
	ShipObject * const ship = getShipThrowTestSlot(env, jship, "getShipInternalDamageOverTimeDamageThreshold()", false, chassisSlot);
	if (!ship)
		return 0.0f;

	static ShipInternalDamageOverTime const * const idot = ShipInternalDamageOverTimeManager::findEntry(*ship, chassisSlot);
	if (idot == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipInternalDamageOverTimeDamageThreshold idot for [%s] slot [%d] not found", ship->getNetworkId().getValueString().c_str(), chassisSlot));
		return 0.0f;
	}

	return idot->getDamageThreshold();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::hasShipInternalDamageOverTime(JNIEnv * env, jobject, jlong jship, jint chassisSlot)
{
	if (!verifyShipsEnabled())
		return false;

	//convert ship
	ShipObject * const ship = JavaLibrary::getShipThrow(env, jship, "hasShipInternalDamageOverTime(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship == nullptr)
		return false;

	static ShipInternalDamageOverTime const * const idot = ShipInternalDamageOverTimeManager::findEntry(*ship, chassisSlot);
	return idot != nullptr;
}

// ----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::shipGetSpaceFaction(JNIEnv *env, jobject, jlong jship)
{
	ShipObject const * const ship = JavaLibrary::getShipThrow(env, jship, "shipGetSpaceFaction(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship)
		return static_cast<jint>(ship->getSpaceFaction());
	return 0;
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsShipNamespace::shipGetSpaceFactionAllies(JNIEnv *env, jobject, jlong jship)
{
	ShipObject const * const ship = JavaLibrary::getShipThrow(env, jship, "shipGetSpaceFactionAllies(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship)
	{
		std::vector<uint32> const &factionList = ship->getSpaceFactionAllies();
		if (!factionList.empty())
		{
			LocalIntArrayRefPtr ret = createNewIntArray(static_cast<jsize>(factionList.size()));
			setIntArrayRegion(*ret, 0, static_cast<jsize>(factionList.size()), reinterpret_cast<jint *>(const_cast<uint32 *>(&factionList[0])));
			return ret->getReturnValue();
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsShipNamespace::shipGetSpaceFactionEnemies(JNIEnv *env, jobject, jlong jship)
{
	ShipObject const * const ship = JavaLibrary::getShipThrow(env, jship, "shipGetSpaceFactionEnemies(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship)
	{
		std::vector<uint32> const &factionList = ship->getSpaceFactionEnemies();
		if (!factionList.empty())
		{
			LocalIntArrayRefPtr ret = createNewIntArray(static_cast<jsize>(factionList.size()));
			setIntArrayRegion(*ret, 0, static_cast<jsize>(factionList.size()), reinterpret_cast<jint *>(const_cast<uint32 *>(&factionList[0])));
			return ret->getReturnValue();
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::shipGetSpaceFactionIsAggro(JNIEnv *env, jobject, jlong jship)
{
	ShipObject const * const ship = JavaLibrary::getShipThrow(env, jship, "shipGetSpaceFactionIsAggro(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship)
		return static_cast<jboolean>(ship->getSpaceFactionIsAggro());
	return JNI_FALSE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::shipSetSpaceFaction(JNIEnv *env, jobject, jlong jship, jint spaceFaction)
{
	ShipObject * const ship = JavaLibrary::getShipThrow(env, jship, "shipSetSpaceFaction(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship)
		ship->setSpaceFaction(static_cast<uint32>(spaceFaction));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::shipSetSpaceFactionAllies(JNIEnv *env, jobject, jlong jship, jintArray jfactionList)
{
	ShipObject * const ship = JavaLibrary::getShipThrow(env, jship, "shipSetSpaceFactionAllies(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship)
	{
		std::vector<uint32> factionList;
		if (jfactionList)
		{
			unsigned int const factionCount = static_cast<unsigned int>(env->GetArrayLength(jfactionList));
			if (factionCount > 0)
			{
				factionList.reserve(factionCount);
				jint * const factionArray = env->GetIntArrayElements(jfactionList, 0);
				for (unsigned int i = 0; i < factionCount; ++i)
					factionList.push_back(static_cast<uint32>(factionArray[i]));
				env->ReleaseIntArrayElements(jfactionList, factionArray, JNI_ABORT);
			}
		}
		ship->setSpaceFactionAllies(factionList);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::shipSetSpaceFactionEnemies(JNIEnv *env, jobject, jlong jship, jintArray jfactionList)
{
	ShipObject * const ship = JavaLibrary::getShipThrow(env, jship, "shipSetSpaceFactionEnemies(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship)
	{
		std::vector<uint32> factionList;
		if (jfactionList)
		{
			unsigned int const factionCount = static_cast<unsigned int>(env->GetArrayLength(jfactionList));
			if (factionCount > 0)
			{
				factionList.reserve(factionCount);
				jint * const factionArray = env->GetIntArrayElements(jfactionList, 0);
				for (unsigned int i = 0; i < factionCount; ++i)
					factionList.push_back(static_cast<uint32>(factionArray[i]));
				env->ReleaseIntArrayElements(jfactionList, factionArray, JNI_ABORT);
			}
		}
		ship->setSpaceFactionEnemies(factionList);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::shipSetSpaceFactionIsAggro(JNIEnv *env, jobject, jlong jship, jboolean isAggro)
{
	ShipObject * const ship = JavaLibrary::getShipThrow(env, jship, "shipSetSpaceFactionIsAggro(): shipId obj_id did not resolve to a ShipObject", false);
	if (ship)
		ship->setSpaceFactionIsAggro(isAggro ? true : false);
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::shipIsInNebula(JNIEnv *env, jobject, jlong jship, jstring j_nebulaName)
{
	ShipObject * const ship = JavaLibrary::getShipThrow(env, jship, "shipIsInNebula(): shipId obj_id did not resolve to a ShipObject", false);
	if (!ship)
		return JNI_FALSE;

	std::string nebulaName;

	if (nullptr != j_nebulaName)
	{
		JavaStringParam const jsp(j_nebulaName);
		if (!JavaLibrary::convert(jsp, nebulaName))
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::shipIsInNebula (): failed to convert nebulaName string"));
	}

	ShipObject::IntVector const & nebulas = ship->getNebulas();

	//-- empty nebula name matches any nebula
	if (nebulaName.empty() && !nebulas.empty())
		return JNI_TRUE;

	for (ShipObject::IntVector::const_iterator it = nebulas.begin(); it != nebulas.end(); ++it)
	{
		int const id = *it;
		Nebula const * const nebula = NebulaManager::getNebulaById(id);
		if (nullptr != nebula)
		{
			if (nebula->getName() == nebulaName)
				return JNI_TRUE;
		}
	}

	return JNI_FALSE;
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipWingName(JNIEnv * /*env*/, jobject /*self*/, jlong target, jstring jstr)
{
	ServerObject * object = 0;
	if(!JavaLibrary::getObject(target, object))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWingName (): failed to convert target"));
		return JNI_FALSE;
	}
	ShipObject * const ship = object->asShipObject();
	if(!ship)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWingName (): target not a ship"));
		return JNI_FALSE;
	}

	JavaStringParam const jsp(jstr);
	std::string wingName;
	if (!JavaLibrary::convert(jsp, wingName))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWingName (): failed to convert mobileName string"));
		return JNI_FALSE;
	}

	ship->setWingName(wingName);

	return JNI_TRUE;
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsShipNamespace::getShipWingName(JNIEnv * /*env*/, jobject /*self*/, jlong target)
{
	ServerObject const * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipWingName (): could not convert the target"));
		return 0;
	}
	ShipObject const * const ship = object->asShipObject();
	if(!ship)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipWingName (): target is not a ship"));
		return 0;
	}

	return JavaString(ship->getWingName()).getReturnValue();
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipTypeName(JNIEnv * /*env*/, jobject /*self*/, jlong target, jstring jstr)
{
	ServerObject * object = 0;
	if(!JavaLibrary::getObject(target, object))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipTypeName (): failed to convert target"));
		return JNI_FALSE;
	}
	ShipObject * const ship = object->asShipObject();
	if(!ship)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipTypeName (): target not a ship"));
		return JNI_FALSE;
	}

	JavaStringParam const jsp(jstr);
	std::string typeName;
	if (!JavaLibrary::convert(jsp, typeName))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWingName (): failed to convert mobileName string"));
		return JNI_FALSE;
	}
	ship->setTypeName(typeName);

	return JNI_TRUE;
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsShipNamespace::getShipTypeName(JNIEnv * /*env*/, jobject /*self*/, jlong target)
{
	ServerObject const * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipTypeName (): could not convert the target"));
		return 0;
	}
	ShipObject const * const ship = object->asShipObject();
	if(!ship)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipTypeName (): target is not a ship"));
		return 0;
	}

	return JavaString(ship->getTypeName()).getReturnValue();
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipDifficulty(JNIEnv * /*env*/, jobject /*self*/, jlong target, jstring jstr)
{
	ServerObject * object = 0;
	if(!JavaLibrary::getObject(target, object))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipDifficultyName (): failed to convert target"));
		return JNI_FALSE;
	}
	ShipObject * const ship = object->asShipObject();
	if(!ship)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipDifficultyName (): target not a ship"));
		return JNI_FALSE;
	}

	JavaStringParam const jsp(jstr);
	std::string difficulty;
	if (!JavaLibrary::convert(jsp, difficulty))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWingName (): failed to convert mobileName string"));
		return JNI_FALSE;
	}
	ship->setDifficulty(difficulty);

	return JNI_TRUE;
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsShipNamespace::getShipDifficulty(JNIEnv * /*env*/, jobject /*self*/, jlong target)
{
	ServerObject const * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipDifficulty (): could not convert the target"));
		return 0;
	}
	ShipObject const * const ship = object->asShipObject();
	if(!ship)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipDifficulty (): target is not a ship"));
		return 0;
	}
	return JavaString(ship->getDifficulty()).getReturnValue();
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::setShipFaction(JNIEnv * /*env*/, jobject /*self*/, jlong target, jstring jstr)
{
	ServerObject * object = 0;
	if(!JavaLibrary::getObject(target, object))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipFactionName (): failed to convert target"));
		return JNI_FALSE;
	}
	ShipObject * const ship = object->asShipObject();
	if(!ship)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipFactionName (): target not a ship"));
		return JNI_FALSE;
	}

	JavaStringParam const jsp(jstr);
	std::string faction;
	if (!JavaLibrary::convert(jsp, faction))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipWingName (): failed to convert mobileName string"));
		return JNI_FALSE;
	}
	ship->setFaction(faction);

	return JNI_TRUE;
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsShipNamespace::getShipFaction(JNIEnv * /*env*/, jobject /*self*/, jlong target)
{
	ServerObject const * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipFaction (): could not convert the target"));
		return 0;
	}
	ShipObject const * const ship = object->asShipObject();
	if(!ship)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::getShipFaction (): target is not a ship"));
		return 0;
	}
	return JavaString(ship->getFaction()).getReturnValue();
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::addMissionCriticalObject(JNIEnv * env, jobject /*self*/, jlong playerId, jlong missionCriticalObject)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::addMissionCriticalObject");

	CreatureObject * const creatureObject = JavaLibrary::getCreatureThrow(env, playerId, "addMissionCriticalObject(): playerId did not resolve to a CreatureObject", false);
	if (!creatureObject)
		return JNI_FALSE;

	NetworkId missionCriticalObjectId(missionCriticalObject);

	if (missionCriticalObject)
	{
		if(missionCriticalObjectId == NetworkId::cms_invalid)
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("addMissionCriticalObject(): missionCriticalObject is not an obj_id"));
			return JNI_FALSE;
		}

		if (missionCriticalObjectId != NetworkId::cms_invalid)
			creatureObject->addMissionCriticalObject(missionCriticalObjectId);
	}
	return JNI_TRUE;
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::removeMissionCriticalObject(JNIEnv * env, jobject /*self*/, jlong playerId, jlong missionCriticalObject)
{
	CreatureObject * const creatureObject = JavaLibrary::getCreatureThrow(env, playerId, "removeMissionCriticalObject(): playerId did not resolve to a CreatureObject", false);
	if (!creatureObject)
		return JNI_FALSE;

	NetworkId missionCriticalObjectId(missionCriticalObject);

	if (missionCriticalObject)
	{
		if (missionCriticalObjectId == NetworkId::cms_invalid)
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("removeMissionCriticalObject(): missionCriticalObject is not an obj_id"));
			return JNI_FALSE;
		}

		if (missionCriticalObjectId != NetworkId::cms_invalid)
			creatureObject->removeMissionCriticalObject(missionCriticalObjectId);
	}
	return JNI_TRUE;
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::clearMissionCriticalObjects(JNIEnv * env, jobject /*self*/, jlong playerId)
{
	CreatureObject * const creatureObject = JavaLibrary::getCreatureThrow(env, playerId, "clearMissionCriticalObjects(): playerId did not resolve to a CreatureObject", false);
	if (!creatureObject)
		return JNI_FALSE;

	creatureObject->clearMissionCriticalObjects();

	return JNI_TRUE;
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsShipNamespace::isMissionCriticalObject(JNIEnv * env, jobject /*self*/, jlong playerId, jlong ship)
{
	CreatureObject * const creatureObject = JavaLibrary::getCreatureThrow(env, playerId, "isMissionCriticalObject(): playerId did not resolve to a CreatureObject", false);
	if (!creatureObject)
		return JNI_FALSE;

	if (!ship)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("isMissionCriticalObject(): playerId ship is nullptr"));
		return JNI_FALSE;
	}

	NetworkId shipId(ship);
	if (shipId == NetworkId::cms_invalid)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("isMissionCriticalObject(): playerId is not an obj_id"));
		return JNI_FALSE;
	}

	return creatureObject->isMissionCriticalObject(shipId);
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsShipNamespace::getShipComponentDebugString(JNIEnv * env, jobject /*self*/, jlong jobject_shipId, jint chassisSlot)
{
	ShipObject const * const ship = JavaLibrary::getShipThrow(env, jobject_shipId, "getShipComponentDebugString(): obj_id shipId did not resolve to a ShipObject", false);
	if (ship && ship->isSlotInstalled(chassisSlot))
	{
		ShipComponentData * const shipComponentData = ship->createShipComponentData(chassisSlot);
		if (shipComponentData)
		{
			Unicode::String result;
			shipComponentData->printDebugString(result, Unicode::emptyString);
			delete shipComponentData;
			if (!result.empty())
				return JavaString(result).getReturnValue();
		}
	}
	return 0;
}

//----------------------------------------------------------------------

jfloat JNICALL ScriptMethodsShipNamespace::getShipChassisSpeedMaximumModifier(JNIEnv * env, jobject self, jlong shipId)
{
	ShipObject const * const shipObject = JavaLibrary::getShipThrow(env, shipId, "getShipChassisMaximumSpeedModifier(): shipId did not resolve to a ShipObject", false);
	if (!shipObject)
		return JNI_FALSE;

	return shipObject->getChassisSpeedMaximumModifier();
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipChassisSpeedMaximumModifier(JNIEnv * env, jobject self, jlong shipId, jfloat value)
{
	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, shipId, "setShipChassisMaximumSpeedModifier(): shipId did not resolve to a ShipObject", false);
	if (!shipObject)
		return;

	shipObject->setChassisSpeedMaximumModifier(value);
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::notifyShipHit(JNIEnv * env, jobject self, jlong jobject_shipId, jobject jobject_vectorUp_w, jobject jobject_vectorHitLocation_o, jint type, jfloat integrity, jfloat previousIntegrity)
{
	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_shipId, "notifyShipHit(): shipId did not resolve to a ShipObject", false);
	if (!shipObject)
		return;

	Vector up_w;
	if (!ScriptConversion::convert(jobject_vectorUp_w, up_w))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("notifyShipHit(): unable to convert up_w vector"));
		return;
	}

	Vector hitLocation_o;
	if (!ScriptConversion::convert(jobject_vectorHitLocation_o, hitLocation_o))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("notifyShipHit(): unable to convert hitLocation_o vector"));
		return;
	}

	MessageQueueShipHit * const msg = new MessageQueueShipHit(up_w, hitLocation_o, type, integrity, previousIntegrity);
	shipObject->getController()->appendMessage(
		CM_spaceShipHit, 0.0f, msg,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_ALL_CLIENT);
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setDynamicMiningAsteroidVelocity(JNIEnv * env, jobject self, jlong jobject_asteroidId, jobject jobject_velocity_w)
{
	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_asteroidId, "setDynamicMiningAsteroidVelocity(): shipId did not resolve to a ShipObject", false);
	if (!shipObject)
		return;

	Vector velocity_w;
	if (!ScriptConversion::convert(jobject_velocity_w, velocity_w))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("setDynamicMiningAsteroidVelocity(): unable to convert velocity_w vector"));
		return;
	}

	MiningAsteroidController * const miningAsteroidController = dynamic_cast<MiningAsteroidController * >(shipObject->getController());
	if (nullptr == miningAsteroidController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("setDynamicMiningAsteroidVelocity(): called on a ship which is not a dynamic mining asteroid"));
		return;
	}

	miningAsteroidController->setVelocity_w(velocity_w);
}

//----------------------------------------------------------------------

jobject JNICALL ScriptMethodsShipNamespace::getDynamicMiningAsteroidVelocity(JNIEnv * env, jobject self, jlong jobject_asteroidId)
{
	ShipObject * const shipObject = JavaLibrary::getShipThrow(env, jobject_asteroidId, "getDynamicMiningAsteroidVelocity(): shipId did not resolve to a ShipObject", false);
	if (!shipObject)
		return nullptr;
	MiningAsteroidController * const miningAsteroidController = dynamic_cast<MiningAsteroidController * >(shipObject->getController());
	if (nullptr == miningAsteroidController)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("setDynamicMiningAsteroidVelocity(): called on a ship which is not a dynamic mining asteroid"));
		return nullptr;
	}

	Vector const & velocity_w = miningAsteroidController->getVelocity_w();

	LocalRefPtr vec;
	if (!ScriptConversion::convert(velocity_w, vec))
		return 0;

	return vec->getReturnValue();
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipCargoHoldContentsMaximum (JNIEnv * env, jobject self, jlong jobject_shipId, jint contentsMaximum)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipCargoHoldContentsMaximum()", false, static_cast<int>(ShipChassisSlotType::SCST_cargo_hold));
	if (!shipObject)
		return;

	if (!shipObject->setCargoHoldContentsMaximum(contentsMaximum))
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipCargoHoldContentsMaximum (): component at slot [%d] error", static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)));
}

//----------------------------------------------------------------------

int JNICALL ScriptMethodsShipNamespace::getShipCargoHoldContentsMaximum (JNIEnv * env, jobject self, jlong jobject_shipId)
{
	ShipObject const * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "getShipCargoHoldContentsMaximum()", false, static_cast<int>(ShipChassisSlotType::SCST_cargo_hold));
	if (!shipObject)
		return false;

	return shipObject->getCargoHoldContentsMaximum();
}

//----------------------------------------------------------------------

int JNICALL ScriptMethodsShipNamespace::getShipCargoHoldContentsCurrent (JNIEnv * env, jobject self, jlong jobject_shipId)
{
	ShipObject const * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "getShipCargoHoldContentsCurrent()", false, static_cast<int>(ShipChassisSlotType::SCST_cargo_hold));
	if (!shipObject)
		return false;

	return shipObject->getCargoHoldContentsCurrent();
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::setShipCargoHoldContent(JNIEnv * env, jobject self, jlong jobject_shipId, jlong jobject_resourceTypeId, jint amount)
{
	ShipObject * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "setShipCargoHoldContent()", false, static_cast<int>(ShipChassisSlotType::SCST_cargo_hold));
	if (!shipObject)
		return;

	NetworkId const & resourceTypeId = NetworkId(jobject_resourceTypeId);

	//-- check existence if amount != 0

	shipObject->setCargoHoldContent(resourceTypeId, amount);
}

//----------------------------------------------------------------------

jint JNICALL ScriptMethodsShipNamespace::getShipCargoHoldContent(JNIEnv * env, jobject self, jlong jobject_shipId, jlong jobject_resourceTypeId)
{
	ShipObject const * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "getShipCargoHoldContent()", false, static_cast<int>(ShipChassisSlotType::SCST_cargo_hold));
	if (!shipObject)
		return false;

	NetworkId const & resourceTypeId = NetworkId(jobject_resourceTypeId);

	return shipObject->getCargoHoldContent(resourceTypeId);
}

//----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsShipNamespace::getShipCargoHoldContentsResourceTypes(JNIEnv * env, jobject self, jlong jobject_shipId)
{
	ShipObject const * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "getShipCargoHoldContents()", false, static_cast<int>(ShipChassisSlotType::SCST_cargo_hold));
	if (!shipObject)
		return nullptr;

	ShipObject::NetworkIdIntMap const & contents = shipObject->getCargoHoldContents();

	LocalLongArrayRefPtr jids = createNewLongArray(contents.size());

	if (jids != LocalLongArrayRef::cms_nullPtr)
	{
		int index = 0;
		for (ShipObject::NetworkIdIntMap::const_iterator it = contents.begin(); it != contents.end(); ++it, ++index)
		{
			jlong jlongTmp = ((*it).first).getValue();
			setLongArrayRegion(*jids, index, 1, &jlongTmp);
		}
		return jids->getReturnValue();
	}

	return nullptr;
}

//----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsShipNamespace::getShipCargoHoldContentsAmounts(JNIEnv * env, jobject self, jlong jobject_shipId)
{
	ShipObject const * const shipObject = getShipThrowTestSlot(env, jobject_shipId, "getShipCargoHoldContents()", false, static_cast<int>(ShipChassisSlotType::SCST_cargo_hold));
	if (!shipObject)
		return nullptr;

	ShipObject::NetworkIdIntMap const & contents = shipObject->getCargoHoldContents();

	LocalIntArrayRefPtr jamounts = createNewIntArray(static_cast<int>(contents.size ()));
	if (jamounts != LocalIntArrayRef::cms_nullPtr)
	{
		int index = 0;
		for (ShipObject::NetworkIdIntMap::const_iterator it = contents.begin(); it != contents.end(); ++it)
		{
			jint amount = (*it).second;
			setIntArrayRegion(*jamounts, index, 1, &amount);
		}
		return jamounts->getReturnValue();
	}

	return nullptr;
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsShipNamespace::openSpaceMiningUi(JNIEnv * env, jobject self, jlong jobject_player, jlong jobject_spaceStation, jstring jstring_spaceStationName)
{
	NetworkId const & playerId = NetworkId(jobject_player);
	NetworkId const & spaceStationId = NetworkId(jobject_spaceStation);

	if (jstring_spaceStationName == nullptr)
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentName (): nullptr name"));
		return;
	}

	JavaStringParam const jsp(jstring_spaceStationName);
	std::string spaceStationName;
	if (!JavaLibrary::convert(jsp, spaceStationName))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::setShipComponentName (): failed to convert name string"));
		return;
	}

	Object * const player = NetworkIdManager::getObjectById(playerId);
	Controller * const controller = player ? player->getController(): nullptr;

	if (!controller)
	{
		WARNING(true, ("JavaLibrary::openSpaceMiningUi failed, no player or controller"));
		return;
	}

	typedef std::pair<NetworkId /*spaceStationId*/, std::string /*spaceStationName*/> MessageData;
	typedef MessageQueueGenericValueType<MessageData> MessageType;
	MessageType * const msg = new MessageType(MessageData(spaceStationId, spaceStationName));

	controller->appendMessage(CM_spaceMiningSaleOpen, 0.0f, msg,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ======================================================================

