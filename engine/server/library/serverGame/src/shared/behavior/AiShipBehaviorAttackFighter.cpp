// ======================================================================
//
// AiShipBehaviorAttackFighter.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorAttackFighter.h"

#include "serverGame/AiPilotManager.h"
#include "serverGame/AiShipAttackTargetList.h"
#include "serverGame/AiShipBehaviorAttackFighter_Maneuver.h"
#include "serverGame/AiShipBehaviorAttackFighter_Maneuver_Path.h"
#include "serverGame/AiShipController.h"
#include "serverGame/AiShipPilotData.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceSquad.h"
#include "serverGame/SpaceAttackSquad.h"
#include "serverGame/MissileManager.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/AiDebugString.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedLog/Log.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Range.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================
//
// AiShipBehaviorAttackFighterNamespace
//
// ======================================================================

namespace AiShipBehaviorAttackFighterNamespace
{
	// ----------------------------------------------------------------------

	//-- Map the weapon priority to weapon slot.
	typedef std::map<float /*priority*/, int /*chassis slot*/> WeaponPriorityMap;

	bool isTargetInConeOfFire(Object const & attackerObject, Sphere const & targetSphere_w, float const coneAngle);
}

using namespace AiShipBehaviorAttackFighterNamespace;

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighterNamespace::isTargetInConeOfFire(Object const & attackerObject, Sphere const & targetSphere_w, float const coneAngle)
{
	return targetSphere_w.intersectsCone(attackerObject.getPosition_w(), attackerObject.getObjectFrameK_w(), coneAngle);
}

// ======================================================================
//
// AiShipBehaviorAttackFighter
//
// ======================================================================

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::AiAttackTargetInformation::AiAttackTargetInformation()
 : m_playerControlled(false)
 , m_distanceToTarget(0.0f)
 , m_speed(0.0f)
 , m_weaponRange(0.0f)
 , m_radius(0.0f)
 , m_facingTarget(false)
 , m_behindTarget(false)
 , m_position_w()
 , m_orientation_w()
 , m_directionFromShip_w()
 , m_offset()
 , m_maneuverPosition_w()
 , m_offsetUpdate(0.0f)
 , m_positionUpdate(0.0f)
 , m_isBoosting(false)
 , m_targetInProjectileConeOfFire(false)
 , m_targetWasInProjectileConeOfFire(false)
 , m_targetInMissileLockOnConeOfFire(false)
 , m_collisionCourse(false)
 , m_requestPositionUpdate(false)
 , m_inTargetsProjectileConeOfFire(false)
{
}

// ----------------------------------------------------------------------

Vector const AiShipBehaviorAttackFighter::calculateEvadePositionWithinLeashDistance_w(AiShipBehaviorAttack const & aiShipBehaviorAttack)
{
	Vector evadePosition_w;

	float const leashRadius = aiShipBehaviorAttack.getLeashRadius();
	float const weaponRange = aiShipBehaviorAttack.getAiShipController().getShipOwner()->getApproximateAttackRange();
	Vector const & ownerPosition_w = aiShipBehaviorAttack.getAiShipController().getOwnerPosition_w();
	Vector const & leashAnchorPosition_w = aiShipBehaviorAttack.getAiShipController().getSquad().getLeashAnchorPosition_w();
	Vector directionFromOwnerToLeashAnchor(leashAnchorPosition_w - ownerPosition_w);
	IGNORE_RETURN(directionFromOwnerToLeashAnchor.normalize());
	Vector const farPosition(ownerPosition_w + -directionFromOwnerToLeashAnchor * weaponRange);
	float const distanceToFarPosition = leashAnchorPosition_w.magnitudeBetween(farPosition);
	Vector randomUnit(Vector::randomUnit());
	float const turnRadiusOfAi = aiShipBehaviorAttack.getAiShipController().getLargestTurnRadius();
	float const evadeDistance = std::min(weaponRange + turnRadiusOfAi, leashRadius) * 0.999f;

	// If there is plenty of room around the unit to evade in any direction, then do so.

	if (distanceToFarPosition < leashRadius)
	{
		evadePosition_w = ownerPosition_w + randomUnit * evadeDistance;
	}
	else
	{
		// We must be cautious when picking the evade position so it is in the leash sphere
	
		if (directionFromOwnerToLeashAnchor.dot(randomUnit) < 0.0f)
		{
			randomUnit = -randomUnit;
		}
		
		evadePosition_w = (leashAnchorPosition_w + (randomUnit * evadeDistance));
	}

	aiShipBehaviorAttack.getAiShipController().clampPositionToZone(evadePosition_w);

	// Verify the evade position is NOT outside the leash radius
	
#ifdef _DEBUG
	float const distanceToNewEvadePosition = leashAnchorPosition_w.magnitudeBetween(evadePosition_w);

	if (distanceToNewEvadePosition > aiShipBehaviorAttack.getLeashRadius())
	{
		DEBUG_WARNING(true, ("debug_ai: ERROR: New evade position (distance from anchor: %.2f) is outside the leash radius(%.2f).", distanceToNewEvadePosition, aiShipBehaviorAttack.getLeashRadius()));
	}
#endif // _DEBUG

	return evadePosition_w;
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::AiShipBehaviorAttackFighter(AiShipController & aiShipController)
 : AiShipBehaviorAttack(aiShipController)
 , m_currentManeuver(nullptr)
 , m_timeDelta(0.0f)
 , m_projectileTimer()
 , m_missileLockOnTimer()
 , m_missileFireDelayTimer()
 , m_hitDecayTimer()
 , m_stateFlags(0)
 , m_targetInfo(new AiAttackTargetInformation)
 , m_missRadians()
 , m_lastManeuverState(static_cast<int>(Maneuver::FM_none))
 , m_maneuverFlags(0)
 , m_lastManeuverFlags(0)
 , m_lastEvadeHealthPercent(1.0f)
 , m_evadeHealthPercent(0.0f)
 , m_timeSinceLastShot(0.0f)
 , m_projectileSpeed(0.0f)
 , m_nextProjectileShotPerfect(false)
 , m_shotErrorPosition_l()
 , m_nextShotPosition_w()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorAttackFighter::AiShipBehaviorAttackFighter() unit(%s)", (aiShipController.getOwner() != nullptr) ? aiShipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner"));
	initializeTimers();
	calculateEvadeHealthPercent();
	calculateNextProjectileShotPerfect();
	calculateWeaponProjectileSpeed();

	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorAttackFighter::AiShipBehaviorAttackFighter() unit(%s) m_projectileSpeed(%.0f)", getAiShipController().getOwner()->getNetworkId().getValueString().c_str(), m_projectileSpeed));
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::AiShipBehaviorAttackFighter(AiShipBehaviorAttack const & sourceBehavior)
 : AiShipBehaviorAttack(sourceBehavior)
 , m_currentManeuver(nullptr)
 , m_timeDelta(0.0f)
 , m_projectileTimer()
 , m_missileLockOnTimer()
 , m_missileFireDelayTimer()
 , m_hitDecayTimer()
 , m_stateFlags(0)
 , m_targetInfo(new AiAttackTargetInformation)
 , m_missRadians()
 , m_lastManeuverState(static_cast<int>(Maneuver::FM_none))
 , m_maneuverFlags(0)
 , m_lastManeuverFlags(0)
 , m_lastEvadeHealthPercent(1.0f)
 , m_evadeHealthPercent(0.0f)
 , m_timeSinceLastShot(0.0f)
 , m_projectileSpeed(0.0f)
 , m_nextProjectileShotPerfect(false)
 , m_shotErrorPosition_l()
 , m_nextShotPosition_w()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorAttackFighter::AiShipBehaviorAttackFighter() unit(%s)", (getAiShipController().getOwner() != nullptr) ? getAiShipController().getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner"));
	initializeTimers();
	calculateEvadeHealthPercent();
	calculateNextProjectileShotPerfect();
	calculateWeaponProjectileSpeed();
	
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorAttackFighter::AiShipBehaviorAttackFighter(AiShipBehaviorAttack const &) unit(%s) m_projectileSpeed(%.0f)" , getAiShipController().getOwner()->getNetworkId().getValueString().c_str(), m_projectileSpeed));
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackFighter::~AiShipBehaviorAttackFighter()
{
	delete m_targetInfo;

	delete m_currentManeuver;
	m_currentManeuver = nullptr;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::alter(float const timeDelta)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipBehaviorAttackFighter::alter");

	calculateNextShotPosition_w();

	AiShipBehaviorAttack::alter(timeDelta);
	
	setTimeDelta(timeDelta);

	// Update the behavior.
	alterTargetInformation();
	alterManeuverFlags();
	alterManeuver();
	alterWeapons();
	alterBooster();
	alterAttackBehavior();
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::setManeuver(Maneuver * const maneuver)
{
	delete m_currentManeuver;
	m_currentManeuver = maneuver;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::alterManeuver()
{
	m_timeSinceLastShot += getTimeDelta();

	bool isManeuverRunning = false;
	float const overallHealthPercent = getAiShipController().getShipOwner()->getOverallHealthWithShieldsAndArmor();

	if(m_currentManeuver)
	{
		ShipObject const * const targetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();

		if (targetShipObject)
		{
			isManeuverRunning = m_currentManeuver->alter(getTimeDelta(), m_stateFlags);
		}
		else
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorAttackFighter::alterManeuver() owner(%s) Why does this ship in the attack state with a nullptr attack target?", getAiShipController().getOwner()->getNetworkId().getValueString().c_str()));
		}

		if (overallHealthPercent > m_lastEvadeHealthPercent)
		{
			m_lastEvadeHealthPercent = overallHealthPercent;
		}
	}

	//-- StateMachine --
	// "Intelligently pick a new maneuver."
	bool const flagsChanged = m_maneuverFlags != m_lastManeuverFlags;

	if (!isManeuverRunning || flagsChanged)
	{
		Maneuver::FighterManeuver newManeuver = Maneuver::FM_leash;

		//-- Check to see what the ship should do next.
		if(shouldStartToLeash())
		{
			newManeuver = Maneuver::FM_leash;

			getAiShipController().getAttackSquad().clearShotsFired();
		}
		else if(shouldStartToEvade())
		{
			newManeuver = Maneuver::FM_evade;

			// This keeps us from continually evading
			
			m_lastEvadeHealthPercent = overallHealthPercent;
			
			calculateEvadeHealthPercent();
			getAiShipController().getAttackSquad().clearShotsFired();
		}
		else 
		{
			newManeuver = Maneuver::FM_chase;
			m_timeSinceLastShot = 0.0f;
		}

		if (getAiShipController().isAttackSquadLeader())
		{
			bool const inFormation = (newManeuver != Maneuver::FM_chase);
			getAiShipController().getAttackSquad().setInFormation(inFormation);
		}
		else
		{
			// Non-squad leaders need to always be chasing when they are released to attack

			newManeuver = Maneuver::FM_chase;
		}

		// Set the last state.
		m_lastManeuverState = static_cast<int>(newManeuver);

		setManeuver(Maneuver::createManeuver(newManeuver, *this, *m_targetInfo));
	}
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::setTimeDelta(float const timeDelta)
{
	m_timeDelta = timeDelta;
}

// ----------------------------------------------------------------------

float AiShipBehaviorAttackFighter::getTimeDelta() const
{
	return m_timeDelta;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::alterWeapons()
{
	ShipObject const * const targetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();

	if (!targetShipObject)
	{
		DEBUG_WARNING(true, ("debug_ai: unit(%s) ERROR: Why is the primary attack target nullptr?", getAiShipController().getShipOwner()->getDebugInformation().c_str()));
		return;
	}

	bool missileReady = false;

	IGNORE_RETURN(m_missileFireDelayTimer.updateNoReset(getTimeDelta()));

	if (m_targetInfo->m_targetInMissileLockOnConeOfFire)
	{
		// Make sure we have a successful missile lock

		if (   m_missileFireDelayTimer.isExpired()
			&& m_missileLockOnTimer.updateZero(getTimeDelta()))
		{
			// See if this missile chance succeeds to fire

			AiShipPilotData const & pilotData = *NON_NULL(getAiShipController().getPilotData());

			missileReady = (Random::randomReal() <= pilotData.m_fighterMissileChanceToFirePercent);

			if (!missileReady)
			{
				m_missileLockOnTimer.reset();
			}
		}
	}
	else
	{
		m_missileLockOnTimer.reset();
	}

	bool projectileReady = false;

	if (   m_projectileTimer.updateNoReset(getTimeDelta())
	    && m_targetInfo->m_targetInProjectileConeOfFire)
	{
		// Make sure enough time has past since the last projectile was shot

		m_projectileTimer.reset();
		projectileReady = true;
	}

	if (   missileReady
	    || projectileReady)
	{
		int const weaponIndex = selectWeaponToFire(missileReady);

		if (weaponIndex != -1)
		{
			ShipObject & ownerShipObject = *NON_NULL(getAiShipController().getShipOwner());

			ownerShipObject.fireShotNonTurretServer(weaponIndex, targetShipObject->getNetworkId(), ownerShipObject.getPilotLookAtTargetSlot(), 0.0f, 0.0f);

			getAiShipController().getAttackSquad().increaseShotsFired();

			m_timeSinceLastShot = 0.0f;
			
			if (ownerShipObject.isMissile(weaponIndex))
			{
				// Missile fired

				m_missileFireDelayTimer.reset();
				m_missileLockOnTimer.reset();
			}
			else
			{
				// Projectile fired

				calculateNextProjectileShotPerfect();
			}

			// Keep my target in my attack target list, this resets the decay timer for the unit we are attacking, so it is not purged from the attack target list
			
			NetworkId const & attackingUnit = targetShipObject->getNetworkId();
			float const damage = 0.0001f;
			bool const verifyAttacker = false;
			IGNORE_RETURN(getAiShipController().addDamageTaken(attackingUnit, damage, verifyAttacker));
		}
	}
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::calculateNextProjectileShotPerfect()
{
	AiShipPilotData const & pilotData = *NON_NULL(getAiShipController().getPilotData());

	m_nextProjectileShotPerfect = (Random::randomReal() > pilotData.m_projectileMissChance);

	Vector const directionToTarget(m_targetInfo->m_position_w - getAiShipController().getOwnerPosition_w());
	float const approximateDistanceToTarget = directionToTarget.approximateMagnitude();
	float const missHalfAngle = pilotData.m_projectileMissAngle / 2.0f;
	Transform transform;
	transform.roll_l(Random::randomReal() * PI_TIMES_2);
	m_shotErrorPosition_l = transform.rotate_l2p(Vector(0.0f, m_targetInfo->m_radius + approximateDistanceToTarget * missHalfAngle, 0.0f));
}

// ----------------------------------------------------------------------

int AiShipBehaviorAttackFighter::selectWeaponToFire(bool const includeMissles)
{
	int result = -1;
	WeaponPriorityMap weaponPriorityMap;
	ShipObject * const ownerShipObject = getAiShipController().getShipOwner();

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		if (   !ownerShipObject->isTurret(weaponIndex)
		    && !ownerShipObject->isCountermeasure(weaponIndex))
		{
			if (ownerShipObject->canFireShot(weaponIndex))
			{
				// -- Get the weapon priority and add it to the list of weapons.
				bool const canFireProjectiles = true;
				float const weaponPriority = getWeaponPriority(includeMissles, canFireProjectiles, weaponIndex);
				
				if (weaponPriority > FLT_MIN)
				{
					IGNORE_RETURN(weaponPriorityMap.insert(std::make_pair(weaponPriority, weaponIndex)));
				}
			}
		}
	}
	
	//-- Shoot based upon weapon priority & skill.

	if (!weaponPriorityMap.empty())
	{
		//-- First get the best weapon.
		WeaponPriorityMap::reverse_iterator itWeapon = weaponPriorityMap.rbegin();
		
		//-- Randomize the weapon selection based on skill level.
		if(Random::randomReal() > getPilotManagerInfo().m_weaponSelectionSkill)
		{
			size_t const randumbWeapon = static_cast<size_t>(Random::random(0, static_cast<int>(weaponPriorityMap.size() - 1)));
			std::advance(itWeapon, randumbWeapon);
		}

		result = itWeapon->second;
	}

	return result;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::alterManeuverFlags()
{
	ShipObject * const shipObject = NON_NULL(safe_cast<ShipObject *>(getAiShipController().getOwner()));

	// Track state transitions.
	m_lastManeuverFlags = m_maneuverFlags;

	// ------------------------------
	// Set Maneuver Flags in this block ONLY.
		// Begin
		//setManeuverFlag(Maneuver::MF_targetInAttackRange, m_targetInfo->m_distanceToTarget <= shipObject->getApproximateAttackRange(false));
		//setManeuverFlag(Maneuver::MF_targetIsTooClose, m_targetInfo->m_distanceToTarget <= getAiShipController().getLargestTurnRadius());
		setManeuverFlag(static_cast<int>(Maneuver::MF_missileLockedOnMe), MissileManager::getInstance().isTargetedByMissile(shipObject->getNetworkId()));
		// End
	// ------------------------------
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::alterBooster()
{
	AiShipPilotData const & pilotData = *NON_NULL(getAiShipController().getPilotData());
	bool activateBooster = false;

	float const boosterRange = std::max(getAiShipController().getShipOwner()->getApproximateAttackRange(true) * pilotData.m_aggression, getAiShipController().getLargestTurnRadius());
	if (m_targetInfo->m_isBoosting || 
		(m_targetInfo->m_distanceToTarget > boosterRange) || 
		(m_targetInfo->m_behindTarget && !m_targetInfo->m_targetWasInProjectileConeOfFire))
	{
		activateBooster = true; 
	}

	getAiShipController().getShipOwner()->setBoosterActive(activateBooster);
}

// ----------------------------------------------------------------------

float AiShipBehaviorAttackFighter::getWeaponPriority(bool const missilesReady, bool const projectileReady, int const weaponIndex)
{
	ShipObject * const shipObject = NON_NULL(safe_cast<ShipObject *>(getAiShipController().getOwner()));
	
	float weaponPriority = -FLT_MAX;

	ShipChassisSlotType::Type const weaponSlot = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);
	uint32 const componentCrc = shipObject->getComponentCrc(weaponSlot);
	bool const isMissile = ShipComponentWeaponManager::isMissile(componentCrc);
	
	if (ShipComponentWeaponManager::isCountermeasure(componentCrc))
	{
		// We don't shoot countermeasures here, they are handled in the AiShipController
	}
	else if((projectileReady && !isMissile) || (missilesReady && isMissile))
	{
		//-- Consider the distance to the target over the weapon range.
		float const range = shipObject->getWeaponRange(weaponIndex);

		if(m_targetInfo->m_distanceToTarget < range)
		{	
			//-- Consider the amount of damage inflicted by this weapons.
			float const weaponDamage = shipObject->getWeaponDamageMaximum(weaponSlot) + shipObject->getWeaponDamageMinimum(weaponSlot);
			
			//-- How effective?
			float const weaponEffectiveness = shipObject->getWeaponEffectivenessShields(weaponSlot) + shipObject->getWeaponEffectivenessArmor(weaponSlot);
			
			//-- How much energy do we use?
			float const energyPerShot = shipObject->getWeaponActualEnergyPerShot(weaponSlot);
			float const weaponEnergy  = (energyPerShot > FLT_MIN) ? shipObject->computeWeaponProjectileTimeToLive(weaponIndex) / energyPerShot : 1.0f;;
			
			//-- Are we running out of ammo for this weapon?
			float const ammoPerShot = static_cast<float>(shipObject->getWeaponAmmoMaximum(weaponSlot));
			float const weaponAmmo = ammoPerShot > FLT_MIN ? static_cast<float>(shipObject->getWeaponAmmoCurrent(weaponSlot)) / ammoPerShot : 1.0f;

			//-- Range.
			float const weaponRange = (range > FLT_MIN) ? m_targetInfo->m_distanceToTarget / range : 1.0f;
			
			//-- Compute a "fuzzy" priority.
			weaponPriority = (weaponDamage * weaponEffectiveness * weaponEnergy * weaponAmmo  * weaponRange);
		}
	}
		
	return weaponPriority;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::alterTargetInformation()
{
	ShipObject & ownerShipObject = *NON_NULL(getAiShipController().getShipOwner());
	ShipObject const * const targetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();

	if (targetShipObject != nullptr)
	{
		//-- Set pilot data here.
		AiShipPilotData const * pilotData = NON_NULL(getAiShipController().getPilotData());
		float const timeDelta = getTimeDelta();

		//-- Set look at target to activate turret tracking.
		if (ownerShipObject.hasTurrets())
		{
			NetworkId const & targetObjectId = targetShipObject->getNetworkId();
			if(targetObjectId != ownerShipObject.getPilotLookAtTarget())
				ownerShipObject.setPilotLookAtTarget(targetObjectId);
		}

		CreatureObject const * const targetPilot = targetShipObject->getPilot();

		m_targetInfo->m_playerControlled = (targetPilot != nullptr) ? targetPilot->isPlayerControlled() : false;

		// Cached target info.
		bool const includeMissiles = true;
		m_targetInfo->m_weaponRange = targetShipObject->getApproximateAttackRange(includeMissiles);
		m_targetInfo->m_speed = targetShipObject->getCurrentSpeed();
		m_targetInfo->m_orientation_w = targetShipObject->getObjectFrameK_w();

		// Target position.
		//float const targetRadius = targetShipObject->getRadius();
		if (m_targetInfo->m_offsetUpdate.updateZero(timeDelta))
		{
			m_targetInfo->m_targetWasInProjectileConeOfFire = false;
			
			// We can't have bad shot due to this approximation. We need a better solution before enabling this.

			m_targetInfo->m_offset = Vector::zero;
			//m_targetInfo->m_offset = Vector::randomUnit();
			//if (m_targetInfo->m_offset.approximateNormalize())
			//	m_targetInfo->m_offset *= targetRadius;
		}

		//RLS TODO: Add ShipObject::getInterceptPosition(timeDelta);
		if (m_targetInfo->m_positionUpdate.updateZero(timeDelta) || m_targetInfo->m_requestPositionUpdate)
		{
			m_targetInfo->m_requestPositionUpdate = false;
			m_targetInfo->m_distanceToTarget = ownerShipObject.getPosition_w().magnitudeBetween(m_targetInfo->m_position_w);
			m_targetInfo->m_position_w = targetShipObject->getInterceptPosition(timeDelta * 2.0f) + m_targetInfo->m_offset;
		}

		Vector const & targetPosition_o = ownerShipObject.rotateTranslate_w2o(m_targetInfo->m_position_w);
		Vector normalizedTargetPosition_o(targetPosition_o);
		if(normalizedTargetPosition_o.normalize())
			m_targetInfo->m_directionFromShip_w = normalizedTargetPosition_o;

		m_targetInfo->m_isBoosting = targetShipObject->isBoosterActive();
		
		// Note: Using target orientation (not position) vs ship orientation.
		// RLS TODO: mUST BE IN THE TURN RADIUS TOO
		m_targetInfo->m_collisionCourse = m_targetInfo->m_distanceToTarget < getAiShipController().getLargestTurnRadius() && acos(m_targetInfo->m_orientation_w.dot(ownerShipObject.getObjectFrameK_w())) < getPilotManagerInfo().m_targetCollisionCourse; 

		// TODO: The projectile cone of fire check needs to use the lead position of the target, not the current position!!!!!!!
		
		// Calculate if our target's lead position is in our _projectile_ cone of fire
		{
			ShipObject const & attackerShipObject = ownerShipObject;
			Sphere const targetSphere_w(m_nextShotPosition_w, targetShipObject->getRadius());
			float const coneAngle = pilotData->m_projectileFireAngle * 0.5f;

			m_targetInfo->m_targetInProjectileConeOfFire = isTargetInConeOfFire(attackerShipObject, targetSphere_w, coneAngle);
			m_targetInfo->m_targetWasInProjectileConeOfFire |= m_targetInfo->m_targetInProjectileConeOfFire;
		}

		// Calculate if our target's lead position is in our _missile_ cone of fire
		{
			ShipObject const & attackerShipObject = ownerShipObject;
			Sphere const targetSphere_w(m_nextShotPosition_w, targetShipObject->getRadius());
			float const coneAngle = pilotData->m_fighterMissileLockOnAngle * 0.5f;

			m_targetInfo->m_targetInMissileLockOnConeOfFire = isTargetInConeOfFire(attackerShipObject, targetSphere_w, coneAngle);
		}
		
		// Calculate if our lead position is in our target's cone of fire
		{
			ShipObject const & attackerShipObject = *targetShipObject;
			float const attackerProjectileSpeed = attackerShipObject.getFastestWeaponProjectileSpeed();

			if (attackerProjectileSpeed > 0.0f)
			{
				// Attacker has weapons

				Vector const & attackerShipPosition_w = attackerShipObject.getPosition_w();
				Vector const & targetLeadPosition_w = ownerShipObject.getTargetLead_p(attackerShipPosition_w, attackerProjectileSpeed);
				Sphere const targetSphere_w(targetLeadPosition_w, ownerShipObject.getRadius());
				float const coneAngle = convertDegreesToRadians(30.0f) * 0.5f;

				m_targetInfo->m_inTargetsProjectileConeOfFire = isTargetInConeOfFire(attackerShipObject, targetSphere_w, coneAngle);
			}
			else
			{
				// Attacker has no weapons

				m_targetInfo->m_inTargetsProjectileConeOfFire = false;
			}
		}

		// Am I facing my target?
		{
			Vector const directionToTarget(targetShipObject->getPosition_w() - ownerShipObject.getPosition_w());
			float const dotProduct = directionToTarget.dot(ownerShipObject.getObjectFrameK_w());

			m_targetInfo->m_facingTarget = (dotProduct >= 0.0f);
		}

		// Am I behind my target?
		{
			Vector const & ownerShipPosition_w = ownerShipObject.getPosition_w();
			Vector const ownerShipPosition_l(targetShipObject->rotateTranslate_w2o(ownerShipPosition_w));

			m_targetInfo->m_behindTarget = (ownerShipPosition_l.z < 0.0f);
		}

		// Get the size of the target.
		m_targetInfo->m_radius = targetShipObject->getRadius();
	}
	else
	{
		DEBUG_WARNING(true, ("debug_ai: unit(%s) ERROR: The primary attack target is nullptr.", ownerShipObject.getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::isFacingTarget() const
{
	return m_targetInfo->m_facingTarget;
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::isBehindTarget() const
{
	return m_targetInfo->m_behindTarget;
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::isTargetInProjectileConeOfFire() const
{
	return m_targetInfo->m_targetInProjectileConeOfFire;
}

// ----------------------------------------------------------------------

float AiShipBehaviorAttackFighter::getDistanceToTargetSquared() const
{
	return sqr(m_targetInfo->m_distanceToTarget);
}

// ----------------------------------------------------------------------

float AiShipBehaviorAttackFighter::getTimeSinceLastShot() const
{
	return m_timeSinceLastShot;
}

// ----------------------------------------------------------------------

float const AiShipBehaviorAttackFighter::getProjectileSpeed() const
{
	return m_projectileSpeed;
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::shouldStartToLeash() const
{
	bool leash = false;

	if (getAiShipController().isAttackSquadLeader())
	{
		// Don't evade twice in a row

		if (   (m_lastManeuverState != static_cast<int>(Maneuver::FM_none))
			&& (m_lastManeuverState != static_cast<int>(Maneuver::FM_evade)))
		{
			if (!isWithinLeashDistance())
			{
				leash = true;
			}
		}
	}

	return leash;
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::shouldStartToEvade() const
{
	bool evade = false;

	if (getAiShipController().isAttackSquadLeader())
	{
		if (m_lastManeuverState != static_cast<int>(Maneuver::FM_evade))
		{
			// If we lose too much of our health, lets evade

			float const overallHealthPercent = getAiShipController().getShipOwner()->getOverallHealthWithShieldsAndArmor();
			float const healthLossPercent = (m_lastEvadeHealthPercent - overallHealthPercent);

			if (healthLossPercent > m_evadeHealthPercent)
			{
				evade = true;
			}

			// If we fire enough shots, then evade

			if (!evade)
			{
				int const totalShotsFired = getAiShipController().getAttackSquad().getTotalShotsFired();
				int const numberOfShotsToFire = getAiShipController().getAttackSquad().getNumberOfShotsToFire();

				if (totalShotsFired > numberOfShotsToFire)
				{
					evade = true;
				}
			}

			//evade |= hasManeuverFlag(Maneuver::MF_targetIsTooClose);

			// Are we to close to the target

			if (!evade)
			{
				if (getAiShipController().getAttackSquad().isTooCloseToTarget())
				{
					evade = true;
				}
			}
		}
	}

	return evade;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::initializeTimers()
{
	AiShipPilotData const & pilotData = *NON_NULL(getAiShipController().getPilotData());

	m_projectileTimer.setExpireTime(pilotData.m_projectileFireDelay);
	m_projectileTimer.setElapsedTime(Random::randomReal(m_projectileTimer.getExpireTime()));

	m_missileLockOnTimer.setExpireTime(pilotData.m_fighterMissileLockOnTime);
	m_missileLockOnTimer.reset();

	m_missileFireDelayTimer.setExpireTime(pilotData.m_fighterMissileFireDelay);
	m_missileFireDelayTimer.setElapsedTime(Random::randomReal(m_missileFireDelayTimer.getExpireTime()));

	m_targetInfo->m_offsetUpdate.setExpireTime(getPilotManagerInfo().m_targetAimZoneUpdate);
	m_targetInfo->m_offsetUpdate.setElapsedTime(Random::randomReal(m_targetInfo->m_offsetUpdate.getExpireTime()));

	m_targetInfo->m_positionUpdate.setExpireTime(getPilotManagerInfo().m_targetReactionTime);
	m_targetInfo->m_positionUpdate.setElapsedTime(Random::randomReal(m_targetInfo->m_positionUpdate.getExpireTime()));

	m_hitDecayTimer.setExpireTime(getPilotManagerInfo().m_hitsDecayTimer);
	m_hitDecayTimer.setElapsedTime(Random::randomReal(m_hitDecayTimer.getExpireTime()));
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::alterAttackBehavior()
{
	ShipObject const * const targetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();
	if (targetShipObject && targetShipObject->isCapitalShip())
	{
		getAiShipController().switchToBomberAttack();
	}
}

// ----------------------------------------------------------------------

AiPilotManager const & AiShipBehaviorAttackFighter::getPilotManagerInfo() const
{
	return getAiShipController().getPilotManagerInfo();
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::setManeuverFlag(int const activeFlags, bool const setFlag)
{
	if (setFlag)
	{
		m_maneuverFlags |= static_cast<int>(activeFlags);
	}
	else
	{
		m_maneuverFlags &= ~static_cast<int>(activeFlags);
	}
}

// ----------------------------------------------------------------------


bool AiShipBehaviorAttackFighter::hasManeuverFlag(int const activeFlags) const
{
	return m_maneuverFlags & static_cast<int>(activeFlags);
}


// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::calculateEvadeHealthPercent()
{
	float const pilotAggression = getAiShipController().getPilotAggression();

	m_evadeHealthPercent = 0.1f + (Random::randomReal() * 0.15f * pilotAggression);
}

// ----------------------------------------------------------------------

bool AiShipBehaviorAttackFighter::isNextProjectileShotPerfect() const
{
	return m_nextProjectileShotPerfect;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::calculateWeaponProjectileSpeed()
{
	ShipObject const & ownerShipObject = *NON_NULL(getAiShipController().getShipOwner());

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		if (ownerShipObject.isProjectile(weaponIndex))
		{
			float const projectileSpeed = ownerShipObject.getWeaponProjectileSpeed(weaponIndex);

#ifdef _DEBUG
			if (   (projectileSpeed > 0.0f)
			    && (m_projectileSpeed > 0.0f)
			    && (!WithinEpsilonInclusive(projectileSpeed, m_projectileSpeed, 0.0001f)))
			{
				DEBUG_WARNING(true, ("debug_ai: unit(%s) ERROR: This AI ship has projectiles with differing speeds. current(%.2f) new(%.2f) weaponIndex(%d) The code does not handle this optimally.", ownerShipObject.getDebugInformation().c_str(), m_projectileSpeed, projectileSpeed, weaponIndex));
			}
#endif // _DEBUG

			m_projectileSpeed = std::max(m_projectileSpeed, projectileSpeed);
		}
	}

	if (m_projectileSpeed <= 0.0f)
	{
		//DEBUG_WARNING(true, ("debug_ai: unit(%s) ERROR: This ship has a projectile speed of %.2f.", ownerShipObject.getDebugInformation().c_str(), m_projectileSpeed));

		m_projectileSpeed = 1.0f;
	}
}

// ----------------------------------------------------------------------

Vector const & AiShipBehaviorAttackFighter::getNextShotPosition_w() const
{
	return m_nextShotPosition_w;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::calculateNextShotPosition_w()
{
	ShipObject const * const targetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();

	if (!targetShipObject)
	{
		DEBUG_WARNING(true, ("debug_ai: unit(%s) ERROR: Why is the primary attack target nullptr?", getAiShipController().getShipOwner()->getDebugInformation().c_str()));
		return;
	}

	if (isNextProjectileShotPerfect())
	{
		// Perfect shot

		float const projectileSpeed = getProjectileSpeed();
		Vector const & ownerShipPosition_w = getAiShipController().getOwnerPosition_w();
		Vector const & targetLeadPosition_w = targetShipObject->getTargetLead_p(ownerShipPosition_w, projectileSpeed);

		m_nextShotPosition_w = targetLeadPosition_w;
	}
	else
	{
		// Bad shot

		Vector const & targetPosition_w = targetShipObject->getPosition_w();
		Vector const & ownerShipPosition_w = getAiShipController().getOwnerPosition_w();
		Vector directionToTargetPosition_w(targetPosition_w - ownerShipPosition_w);
		float const distanceToTargetPosition = directionToTargetPosition_w.magnitude();

		if (distanceToTargetPosition < Vector::NORMALIZE_THRESHOLD)
		{
			// We are on top of the target, just shoot toward the target

			m_nextShotPosition_w = targetPosition_w;
		}
		else
		{
			IGNORE_RETURN(directionToTargetPosition_w.normalize());
			Transform transform;
			transform.setLocalFrameKJ_p(directionToTargetPosition_w, Vector::perpendicular(directionToTargetPosition_w));
			transform.setPosition_p(ownerShipPosition_w);

			float const x = m_shotErrorPosition_l.x;
			float const y = m_shotErrorPosition_l.y;
			float const z = distanceToTargetPosition;

			m_nextShotPosition_w = transform.rotateTranslate_l2p(Vector(x, y, z));
		}
	}
}

#ifdef _DEBUG
// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::addDebug(AiDebugString & aiDebugString)
{
	AiShipBehaviorAttack::addDebug(aiDebugString);

	// Show the path size

	NOT_NULL(m_currentManeuver);
	AiShipBehaviorAttackFighter::Maneuver::Path const * path = m_currentManeuver->getCurrentPath();

	char pathSizeText[256];
	if (path != nullptr)
	{
		snprintf(pathSizeText, sizeof(pathSizeText) - 1, "PATH(%d)", path->getLength());
	}
	else
	{
		snprintf(pathSizeText, sizeof(pathSizeText) - 1, "NO PATH");
	}
	pathSizeText[sizeof(pathSizeText) - 1] = '\0';

	// Show the current maneuver

	{
		char const * const text = FormattedString<512>().sprintf("attack[%s%s%s] %d %s\n", AiDebugString::getColorCode(PackedRgb::solidWhite).c_str(), (m_currentManeuver != nullptr) ? m_currentManeuver->getFighterManeuverString() : "nullptr", AiDebugString::getResetColorCode(), (m_currentManeuver != nullptr) ? m_currentManeuver->getSequenceId() : -1, pathSizeText);
		aiDebugString.addText(text, PackedRgb::solidCyan);
	}

	//{
	//	char const * const text = FormattedString<512>().sprintf("facingTarget(%s) behindTarget(%s)\n", isFacingTarget() ? "YES" : "NO", isBehindTarget() ? "YES" : "NO");
	//	aiDebugString.addText(text, PackedRgb::solidWhite);
	//}

	//{
	//	if (m_currentManeuver->isDesiredPositionBehindMe())
	//	{
	//		aiDebugString.addText(FormattedString<512>().sprintf("isDesiredPositionBehindMe(YES)\n"), PackedRgb::solidYellow);
	//	}
	//	else
	//	{
	//		aiDebugString.addText(FormattedString<512>().sprintf("isDesiredPositionBehindMe(NO)\n"), PackedRgb::solidYellow);
	//	}
	//}

	//{
	//	if (m_targetInfo->m_inTargetsProjectileConeOfFire)
	//	{
	//		aiDebugString.addText("inTargetsConeOfFire(YES)\n", PackedRgb::solidYellow);
	//	}
	//	else
	//	{
	//		aiDebugString.addText("inTargetsConeOfFire(NO)\n", PackedRgb::solidYellow);
	//	}
	//}

	// Show the maneuver path

	if (   (path != nullptr)
	    && !path->isEmpty())
	{
		AiDebugString::TransformList transformList;

		for (int index = 0; index < path->getLength(); ++index)
		{
			Vector position_w;
			IGNORE_RETURN(path->getNode(index, position_w));
			Transform transform;
			transform.setPosition_p(position_w.x, position_w.y, position_w.z);
			transformList.push_back(transform);
		}

		bool const cyclic = false;
		aiDebugString.addPath(transformList, cyclic);

		Vector currentPosition_w;
		if (path->getNode(m_currentManeuver->getCurrentPathIndex(), currentPosition_w))
		{
			aiDebugString.addLineToPosition(currentPosition_w, PackedRgb::solidCyan);
		}
	}
	else if (   (m_currentManeuver->getFighterManeuver() == Maneuver::FM_evade)
	         || (m_currentManeuver->getFighterManeuver() == Maneuver::FM_chase))
	{
		// Do not check this in enabled, it spams the client and will disconnect you with too many ships

		aiDebugString.addLineToPosition(m_currentManeuver->getDesiredPosition_w(), PackedRgb::solidCyan);
	}

	//{
	//	AiShipPilotData const * pilotData = NON_NULL(getAiShipController().getPilotData());
	//	
	//	{
	//		float const length = m_targetInfo->m_distanceToTarget;
	//		float const coneAngle = pilotData->m_projectileFireAngle * 0.5f;
	//		PackedRgb const & color = m_targetInfo->m_targetInProjectileConeOfFire ? PackedRgb::solidRed : PackedRgb::solidWhite;
	//
	//		aiDebugString.addCone(length, coneAngle, color);
	//	}
	//
	//	{
	//		float const length = m_targetInfo->m_distanceToTarget;
	//		float const coneAngle = pilotData->m_fighterMissileLockOnAngle * 0.5f;
	//		PackedRgb const & color = m_targetInfo->m_targetInMissileLockOnConeOfFire ? PackedRgb::solidRed : PackedRgb::solidWhite;
	//
	//		aiDebugString.addCone(length, coneAngle, color);
	//	}
	//}
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackFighter::debug_forceManeuver(int const maneuverType)
{
	Maneuver::FighterManeuver const forcedManeuverType = static_cast<Maneuver::FighterManeuver>(maneuverType);

	setManeuver(Maneuver::createManeuver(forcedManeuverType, *this, *m_targetInfo));
}

#endif // _DEBUG

// ======================================================================
