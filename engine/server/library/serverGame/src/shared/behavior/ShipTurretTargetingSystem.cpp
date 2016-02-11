// ======================================================================
//
// ShipTurretTargetingSystem.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipTurretTargetingSystem.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ShipController.h"
#include "serverGame/ShipObject.h"
#include "sharedLog/Log.h"

// ======================================================================

ShipTurretTargetingSystem::ShipTurretTargetingSystem(ShipController & shipController) :
		m_targetsDirty(false),
		m_shipController(shipController),
		m_turretsPerTarget(new TurretsPerTargetType),
		m_targetList(new AiShipAttackTargetList::SortedTargetList),
		m_idealTurretsPerTarget(0),
		m_retargetTimer(2.0f) //TODO: determine if we need really this.  It was done because SpaceTargetManager::buildTargetList is expensive and we don't want to call it often, but that may change
{
}

// ----------------------------------------------------------------------

ShipTurretTargetingSystem::~ShipTurretTargetingSystem()
{
	delete m_turretsPerTarget;
	delete m_targetList;
}

// ---------------------------------------------------------------------

void ShipTurretTargetingSystem::alter(float const deltaTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("ShipTurretTargetingSystem::alter");
	IGNORE_RETURN(m_retargetTimer.updateNoReset(deltaTime));

	ShipObject * const shipObject = NON_NULL(NON_NULL(NON_NULL(m_shipController.getOwner())->asServerObject())->asShipObject());
		
	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		if (shipObject->isTurret(weaponIndex)
			&& isTurretControlled(weaponIndex))
		{
			PROFILER_AUTO_BLOCK_DEFINE("turret");
			bool canFireShot = false;
			Object const * const target = shipObject->getTurretTarget(weaponIndex).getObject();
			
			if (target)
			{
				Vector const & turretPosition_w = shipObject->getTurretTransform(weaponIndex).getPosition_p();
				bool const targetInRange = (target->getPosition_w().magnitudeBetweenSquared(turretPosition_w) <= sqr(shipObject->getWeaponRange(weaponIndex)));

				if (targetInRange)
				{
					canFireShot = shipObject->canTurretFireTowardsLocation_p(weaponIndex, target->getPosition_w());

					if (canFireShot)
					{
						ShipChassisSlotType::Type const shipChassisSlotType = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);
						if (shipObject->isSlotInstalled(shipChassisSlotType))
						{
							if (shipObject->hasEnergyForAShot(weaponIndex) && shipObject->hasAmmoForAShot(weaponIndex))
							{
								PROFILER_AUTO_BLOCK_DEFINE("fireShot");
								doShot(weaponIndex, target->getNetworkId(), shipObject->getPilotLookAtTargetSlot(), (Random::randomReal(0.0f,1.0f) > m_shipController.getTurretMissChance()));

								// Keep my target in my attack target list, this resets the decay timer for the unit we are attacking, so it is not purged from the attack target list
								
								NetworkId const & attackingUnit = target->getNetworkId();
								float const damage = 0.0001f;
								bool const verifyAttacker = false;
								m_shipController.addDamageTaken(attackingUnit, damage, verifyAttacker);
							}
						}
					}
				}
			}
			
			if (!canFireShot)
			{
				shipObject->setTurretTarget(weaponIndex, onTurretLostTarget(weaponIndex, shipObject->getTurretTarget(weaponIndex)));
			}
		}
	}
}

// ----------------------------------------------------------------------

ShipController const & ShipTurretTargetingSystem::getShipController() const
{
	return m_shipController;
}
	
// ----------------------------------------------------------------------

ShipController & ShipTurretTargetingSystem::getShipController() 
{
	return m_shipController; //lint !e1536 // returning reference to member
}
	
// ----------------------------------------------------------------------

void ShipTurretTargetingSystem::onTargetChanged(NetworkId const & /*target*/)
{
	m_targetsDirty = true;
}

// ----------------------------------------------------------------------

void ShipTurretTargetingSystem::onTargetLost(NetworkId const & target)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipTurretTargetingSystem::onTargetLost() owner(%s) target(%s)", m_shipController.getOwner()->getNetworkId().getValueString().c_str(), target.getValueString().c_str()));

	m_targetsDirty = true;

	// If any turret was attacking this target, remove the target

	ShipObject * const shipObject = NON_NULL(NON_NULL(NON_NULL(m_shipController.getOwner())->asServerObject())->asShipObject());
	if (!shipObject)  // for release builds
	{
		WARNING(true,("Programmer bug:  got a nullptr ShipObject in ShipTurretTargetingSystem::onTargetLost().  May indicate that the ShipObject has already been partially deconstructed."));
		return;
	}

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		if (shipObject->isTurret(weaponIndex))
		{
			if (shipObject->getTurretTarget(weaponIndex) == target)
			{
				LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("ShipTurretTargetingSystem::onTargetLost() owner(%s) TARGET LOST(%s)", m_shipController.getOwner()->getNetworkId().getValueString().c_str(), target.getValueString().c_str()));

				// Remove the turrets assignment since it is no longer in the target list

				shipObject->setTurretTarget(weaponIndex, CachedNetworkId::cms_cachedInvalid);
			}
		}
	}

	// We must acquire a new target list here so the turrets stop shooting at invalid targets	
	IGNORE_RETURN(buildTargetList());
}

// ----------------------------------------------------------------------

ShipObject & ShipTurretTargetingSystem::getShipOwner()
{
	return (*NON_NULL(NON_NULL(NON_NULL(m_shipController.getOwner())->asServerObject())->asShipObject()));
}

// ----------------------------------------------------------------------

bool ShipTurretTargetingSystem::buildTargetList()
{
	m_retargetTimer.reset();
	m_targetsDirty = false;

	// Get a new list of attack targets
	
	ShipObject * const ownerShipObject = getShipController().getShipOwner();
	ShipController * const ownerShipController = ownerShipObject->getController()->asShipController();
	bool result = false;
	m_targetList->clear();
	
	if (ownerShipController != nullptr)
	{
		if (!ownerShipController->getAttackTargetList().isEmpty())
		{
			result = true;

			ownerShipController->getAttackTargetList().getSortedTargetList(*m_targetList);

			// Count the number of turrets currently available
			int numGuns=0;
			for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
			{
				if (ownerShipObject->isTurret(weaponIndex))
				{
					ShipChassisSlotType::Type const shipChassisSlotType = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);
					if (ownerShipObject->isSlotInstalled(shipChassisSlotType))
						++numGuns;
				}
			}				

			m_idealTurretsPerTarget = std::max(1, numGuns / static_cast<int>(m_targetList->size()));
		}
		else
		{
			m_idealTurretsPerTarget = 0;
		}
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("PlayerShipTurretTargetingSystem::alter() ERROR: Why does this owner(%s) not have an ShipController?", ownerShipObject->getNetworkId().getValueString().c_str()));
	}

	// Cleanup: remove anything from the turretsPerTarget that has a count of 0, so that turretsPerTarget doesn't grow indefinitely
	
	for (TurretsPerTargetType::iterator i=m_turretsPerTarget->begin(); i!=m_turretsPerTarget->end();)
	{
		if (i->second <= 0)
			m_turretsPerTarget->erase(i++);
		else
			++i;
	}

	return result;
}

// ----------------------------------------------------------------------

/**
 * Called when a turret has no target or can't shoot at its assigned target.
 * Figure out a new target for the turret.
 */
CachedNetworkId ShipTurretTargetingSystem::onTurretLostTarget(int const weaponIndex, CachedNetworkId const & oldTarget)
{
	// Only get new targets if some new targets have been added to the list while the timer was ticking

	if (   m_targetsDirty
	    && m_retargetTimer.isExpired()
	    && !buildTargetList())
	{
		return CachedNetworkId::cms_cachedInvalid;
	}

	if (oldTarget != CachedNetworkId::cms_cachedInvalid)
	{
		--((*m_turretsPerTarget)[oldTarget]);
		DEBUG_FATAL((*m_turretsPerTarget)[oldTarget] < 0,("Programmer bug:  m_turretsPerTarget went negative for target %s", oldTarget.getValueString().c_str()));
	}
		
	ShipObject * const ownerShipObject = getShipController().getShipOwner();

	CachedNetworkId bestTarget(CachedNetworkId::cms_cachedInvalid);
	CachedNetworkId bestTargetIgnoringLimits(CachedNetworkId::cms_cachedInvalid);
	
	for (AiShipAttackTargetList::SortedTargetList::const_iterator target = m_targetList->begin(); target != m_targetList->end(); ++target)
	{
		CachedNetworkId const & cachedNetworkId = target->first;
		Object const * const targetObject = NON_NULL(cachedNetworkId.getObject());

		if (targetObject)
		{
			if (ownerShipObject->canTurretFireTowardsLocation_p(weaponIndex, targetObject->getPosition_w()))
			{
				if ((*m_turretsPerTarget)[target->first] < m_idealTurretsPerTarget)
				{
					bestTarget = target->first;
					break;
				}
				else
					bestTargetIgnoringLimits = target->first;
			}
		}
	}

	if (bestTarget == CachedNetworkId::cms_cachedInvalid)
		bestTarget = bestTargetIgnoringLimits;

	if (bestTarget != CachedNetworkId::cms_cachedInvalid)
	{
		++((*m_turretsPerTarget)[bestTarget]);
		return bestTarget;
	}

	return CachedNetworkId::cms_cachedInvalid;
}

// ======================================================================
