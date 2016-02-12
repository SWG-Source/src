// ======================================================================
//
// AiShipBehaviorAttackBomber.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipBehaviorAttackBomber.h"

#include "serverGame/AiShipController.h"
#include "serverGame/AiShipPilotData.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ShipObject.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/AiDebugString.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace AiShipBehaviorAttackBomber_namespace
{
	float const ms_breakOffDistanceSquared = sqr(200.0f);
	float const ms_evadeOffsetMax = 200.0f; // constant for now, TODO:  hook this up to the pilot type data table

	struct TargetShipData
	{
		TargetShipData();

		typedef	std::map<ShipChassisSlotType::Type, int> BombersPerComponentType;
		
		int m_totalBombers;
		BombersPerComponentType m_bombersPerComponent;
	};
	
	typedef std::map<NetworkId, TargetShipData> TargetShipsType;
	TargetShipsType ms_targetShips;
}

using namespace AiShipBehaviorAttackBomber_namespace;

// ======================================================================

AiShipBehaviorAttackBomber::AiShipBehaviorAttackBomber(AiShipController & aiShipController) :
		AiShipBehaviorAttack(aiShipController),
		m_attackStage(AS_attack),
		m_targetId(NetworkId::cms_invalid),
		m_targetComponentSlot(ShipChassisSlotType::SCST_invalid),
		m_targetComponentPosition_o(Vector::zero),
		m_evadePosition_o(Vector::zero),
		m_evadeDistanceSquared(0.0f),
		m_evadeOffset(Vector::zero),
		m_evadeChangeDirectionTimer(0.5f),
		m_missilesFired(0)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorAttackBomber::AiShipBehaviorAttackBomber() unit(%s)", (aiShipController.getOwner() != nullptr) ? aiShipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner"));
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackBomber::AiShipBehaviorAttackBomber(AiShipBehaviorAttack const & sourceBehavior) :
		AiShipBehaviorAttack(sourceBehavior),
		m_attackStage(AS_attack),
		m_targetId(NetworkId::cms_invalid),
		m_targetComponentSlot(ShipChassisSlotType::SCST_invalid),
		m_targetComponentPosition_o(Vector::zero),
		m_evadePosition_o(Vector::zero),
		m_evadeDistanceSquared(0.0f),
		m_evadeOffset(Vector::zero),
		m_evadeChangeDirectionTimer(0.5f),
		m_missilesFired(0)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipBehaviorAttackBomber::AiShipBehaviorAttackBomber(AiShipBehaviorAttack &) unit(%s)", (m_aiShipController.getOwner() != nullptr) ? m_aiShipController.getOwner()->getNetworkId().getValueString().c_str() : "nullptr owner"));
}

// ----------------------------------------------------------------------

AiShipBehaviorAttackBomber::~AiShipBehaviorAttackBomber()
{
	changeTargetObject(NetworkId::cms_invalid);
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackBomber::alter(float const deltaTime)
{
	AiShipBehaviorAttack::alter(deltaTime);

	PROFILER_AUTO_BLOCK_DEFINE("AiShipBehaviorAttackBomber::alter");

	NetworkId const & newTargetId = getAiShipController().getPrimaryAttackTarget();

	if (newTargetId != m_targetId)
	{
		changeTargetObject(newTargetId);
	}
	
	if (getTargetCapitalShip())
	{
		if (m_targetComponentSlot == ShipChassisSlotType::SCST_invalid)
			selectTargetComponent();
		
		if (m_targetComponentSlot != ShipChassisSlotType::SCST_invalid)
		{
			switch(m_attackStage)
			{
				case AS_evade:
					doEvade(deltaTime);
					break;
				case AS_attack:
					doAttackRun(deltaTime);
					break;
				default:
					break;
			}
		}
	}
 	else
 	{
 		// switch to a fighter attack behavior
		m_aiShipController.switchToFighterAttack();
 	}
}

// ----------------------------------------------------------------------

/**
 * Helper function to pick a component on the targeted capital ship to attack.
 * Note:  success is not guaranteed.  A component may not be selected after
 * this function returns
 */
void AiShipBehaviorAttackBomber::selectTargetComponent()
{
	ShipObject const * const targetShipObject = getTargetCapitalShip();
	
	if (targetShipObject)
	{
		Vector const position_w=NON_NULL(m_aiShipController.getShipOwner())->getPosition_w();
		float const evadeDistance = targetShipObject->getApproximateAttackRange();
		m_evadeDistanceSquared = sqr(evadeDistance);
		ShipChassisSlotType::Type newTargetComponentSlot = ShipChassisSlotType::SCST_invalid;
		
		TargetShipData & targetShipData = ms_targetShips[targetShipObject->getNetworkId()];

		float closestSoFar = 0;
		int fewestBombersSoFar = 0;
		std::vector<Transform> transforms;
		for (int chassisSlot = ShipChassisSlotType::SCST_first;  chassisSlot < ShipChassisSlotType::SCST_num_types; ++chassisSlot)
		{
			if (targetShipObject->isValidTargetableSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot)))
			{
				targetShipObject->findTransformsForComponent(chassisSlot, transforms);
				for (std::vector<Transform>::const_iterator i=transforms.begin(); i!=transforms.end(); ++i)
				{
					// pick the component whose "evade" position, i.e. the start position for the attack run,
					// is closest to where we are now.
					// If coordinating with other bombers, prefer the components that have the fewest bombers
					// already attacking them.
					
					Vector const evadePosition_o = i->getPosition_p() + (i->getLocalFrameJ_p() * evadeDistance * 1.5f);
					Vector const evadePosition_w = targetShipObject->rotateTranslate_o2w(evadePosition_o);
					float const distanceSquared = (evadePosition_w - position_w).magnitudeSquared();

					bool preferThisOne = false;
					if (closestSoFar == 0)
						preferThisOne = true;
					if (targetShipData.m_bombersPerComponent[static_cast<ShipChassisSlotType::Type>(chassisSlot)] < fewestBombersSoFar)
						preferThisOne = true;
					if (targetShipData.m_bombersPerComponent[static_cast<ShipChassisSlotType::Type>(chassisSlot)] == fewestBombersSoFar &&
						distanceSquared < closestSoFar)
						preferThisOne = true;

					if (preferThisOne)
					{
						closestSoFar = distanceSquared;
						fewestBombersSoFar = targetShipData.m_bombersPerComponent[static_cast<ShipChassisSlotType::Type>(chassisSlot)];
						m_targetComponentPosition_o = i->getPosition_p();
						m_evadePosition_o = evadePosition_o;
						newTargetComponentSlot = static_cast<ShipChassisSlotType::Type>(chassisSlot);
					}
				}
			}
		}

		if (newTargetComponentSlot != ShipChassisSlotType::SCST_invalid)
		{
			DEBUG_REPORT_LOG(true,("Ship %s targetting slot %i\n",m_aiShipController.getShipOwner()->getNetworkId().getValueString().c_str(),newTargetComponentSlot));
			changeTargetComponentSlot(newTargetComponentSlot);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Fly towards a point in space near the capital ship but out of range of
 * its weapons
 */
void AiShipBehaviorAttackBomber::doEvade(float const deltaTime)
{
	ShipObject * const ownerShipObject = NON_NULL(m_aiShipController.getShipOwner());
	ownerShipObject->setBoosterActive(true);
		
	if (m_evadeChangeDirectionTimer.updateZero(deltaTime))
	{
		m_evadeOffset = Vector (Random::randomReal(-ms_evadeOffsetMax,ms_evadeOffsetMax),
							   Random::randomReal(-ms_evadeOffsetMax,ms_evadeOffsetMax),
							   Random::randomReal(-ms_evadeOffsetMax,ms_evadeOffsetMax));
	}

	Vector const goalPosition_w = NON_NULL(getTargetCapitalShip())->rotateTranslate_o2w(m_evadePosition_o) + m_evadeOffset;
	float const throttle = 1.0f;
	m_aiShipController.moveTo(goalPosition_w, throttle, deltaTime);

	if ((getTargetCapitalShip()->getPosition_w() - NON_NULL(m_aiShipController.getShipOwner())->getPosition_w()).magnitudeSquared() > m_evadeDistanceSquared)
		m_attackStage = AS_attack;
}

// ----------------------------------------------------------------------

/**
 * Make a run at the selected component of the capital ship.  Fire missiles &
 * guns when in range.
 */
void AiShipBehaviorAttackBomber::doAttackRun(float const deltaTime)
{
	ShipObject const * const targetShipObject = NON_NULL(getTargetCapitalShip());
	ShipObject * const ownerShipObject = NON_NULL(m_aiShipController.getShipOwner());
	AiShipPilotData const * pilotData = m_aiShipController.getPilotData();
	Vector const goalPosition_w = targetShipObject->rotateTranslate_o2w(m_targetComponentPosition_o);
	Vector const goalPosition_o = ownerShipObject->rotateTranslate_w2o(goalPosition_w); // target in the space of the owner ship

	// Move & do avoidance if necessary

	float const throttle = 1.0f;
	m_aiShipController.moveTo(goalPosition_w, throttle, deltaTime);
	
	// Fire weapons if possible

	Vector normalizedTargetPosition_o(goalPosition_o);
	if (normalizedTargetPosition_o.approximateNormalize() && (acos(normalizedTargetPosition_o.dot(Vector::unitZ)) < pilotData->m_projectileFireAngle))
	{
		for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
		{
			if (!ownerShipObject->isTurret(weaponIndex))
			{
				if (goalPosition_o.magnitudeSquared() < sqr(ownerShipObject->getWeaponRange(weaponIndex)))
				{
					ShipChassisSlotType::Type const chassisSlot = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);
					if (ownerShipObject->isSlotInstalled(chassisSlot))
					{
						if (ownerShipObject->canFireShot(weaponIndex))
						{
							if (ShipComponentWeaponManager::isMissile(ownerShipObject->getComponentCrc(chassisSlot)))
								++m_missilesFired;
							ownerShipObject->fireShotNonTurretServer(weaponIndex, targetShipObject->getNetworkId(), m_targetComponentSlot);
						}
					}
				}
			}
		}
	}

	// Don't fire booster while on attack run (more time to launch missiles & fire guns)
	
	ownerShipObject->setBoosterActive(false);
	
	// Check for state changes
	
	if (!targetShipObject->isValidTargetableSlot(static_cast<ShipChassisSlotType::Type>(m_targetComponentSlot)))
		changeTargetComponentSlot(ShipChassisSlotType::SCST_invalid);
	if (((ownerShipObject->getPosition_w() - goalPosition_w).magnitudeSquared() < ms_breakOffDistanceSquared) || m_missilesFired >= pilotData->m_bomberMissilesPerBombingRun)
	{
		m_attackStage = AS_evade;
		m_missilesFired = 0;
	}
}

// ----------------------------------------------------------------------

/**
 * Returns a pointer to the target object, if it is a capital ship
 */
ShipObject const * AiShipBehaviorAttackBomber::getTargetCapitalShip() const
{
	ShipObject const * const targetShipObject = getAiShipController().getPrimaryAttackTargetShipObject();

	if (targetShipObject && targetShipObject->isCapitalShip())
	{
		return targetShipObject;
	}

	return nullptr;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackBomber::changeTargetObject(NetworkId const & newTarget)
{
	changeTargetComponentSlot(ShipChassisSlotType::SCST_invalid);
	
	if (m_targetId != NetworkId::cms_invalid)
	{
		TargetShipData & data = ms_targetShips[m_targetId];
		if (--(data.m_totalBombers) <= 0)
			IGNORE_RETURN(ms_targetShips.erase(m_targetId));
	}

	if (newTarget != NetworkId::cms_invalid)
	{
		TargetShipData & data = ms_targetShips[newTarget];
		++(data.m_totalBombers);
	}
	
	m_targetId = newTarget;
}

// ----------------------------------------------------------------------

void AiShipBehaviorAttackBomber::changeTargetComponentSlot(ShipChassisSlotType::Type newTarget)
{
	if (m_targetId != NetworkId::cms_invalid)
	{
		TargetShipsType::iterator data=ms_targetShips.find(m_targetId);
		DEBUG_FATAL(data == ms_targetShips.end(),("Programmer bug:  changing target slot, but target %s is not in ms_targetShips.  It should have already been added",m_targetId.getValueString().c_str()));
	
		if (m_targetComponentSlot != ShipChassisSlotType::SCST_invalid)
			--(data->second.m_bombersPerComponent[m_targetComponentSlot]);
	
		if (newTarget != ShipChassisSlotType::SCST_invalid)
			++(data->second.m_bombersPerComponent[newTarget]);
	}

	DEBUG_FATAL(newTarget < ShipChassisSlotType::SCST_first || newTarget > ShipChassisSlotType::SCST_num_types,("newTarget out of range"));
	
	m_targetComponentSlot = newTarget;
}

// ----------------------------------------------------------------------

TargetShipData::TargetShipData() :
		m_totalBombers(0),
		m_bombersPerComponent()
{
}
		
#ifdef _DEBUG
// ----------------------------------------------------------------------

void AiShipBehaviorAttackBomber::addDebug(AiDebugString & aiDebugString)
{
	AiShipBehaviorAttack::addDebug(aiDebugString);

	// Show the current maneuver

	{
		char const * const text = FormattedString<512>().sprintf("BOMBER\n");
		aiDebugString.addText(text, PackedRgb::solidCyan);
	}
}
#endif // _DEBUG

// ======================================================================
