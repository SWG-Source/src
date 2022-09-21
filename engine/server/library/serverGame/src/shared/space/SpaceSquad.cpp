// ======================================================================
// 
// SpaceSquad.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SpaceSquad.h"

#include "serverGame/AiShipAttackTargetList.h"
#include "serverGame/AiShipController.h"
#include "serverGame/AiShipControllerInterface.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ShipObject.h"
#include "serverGame/SpaceAttackSquad.h"
#include "serverGame/SpacePath.h"
#include "serverGame/SpaceSquadManager.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedLog/Log.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"

#include <limits>
#include <list>
#include <map>
#include <set>

// ======================================================================
//
// SpaceSquadNamespace
//
// ======================================================================

namespace SpaceSquadNamespace
{
	typedef std::map<PersistentCrcString, int> FormationPriorityList;

	FormationPriorityList s_formationPriorityList;
	float s_flockingSeperationGain = 4.0f;
	float s_flockingComfortDistanceGain = 1.0f;
	float const s_defaultShipRadius = 7.0f;
}

using namespace SpaceSquadNamespace;

// ======================================================================
//
// SpaceSquad
//
// ======================================================================

// ----------------------------------------------------------------------
void SpaceSquad::install()
{
	// Create the data table

	Iff iff;

	if (iff.open("datatables/space_mobile/space_mobile.iff", true))
	{
		DataTable dataTable;
		dataTable.load(iff);

		int const rowCount = dataTable.getNumRows();

		for (int row = 0; row < rowCount; ++row)
		{
			PersistentCrcString const shipName(dataTable.getStringValue("strIndex", row), false);
			int const formationPriority = dataTable.getIntValue("formationPriority", row);
			
			IGNORE_RETURN(s_formationPriorityList.insert(std::make_pair(shipName, formationPriority)));

			LOGC((ConfigServerGame::isSpaceAiLoggingEnabled() && formationPriority  <= 0), "space_debug_ai", ("SpaceSquad::install() ERROR: Invalid formationPriority(%d) specified for shipName(%s)", formationPriority, shipName.getString()));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load space_mobile.iff to retrieve formation priorities!"));
	}

	ExitChain::add(&remove, "SpaceSquad::remove");
}

// ----------------------------------------------------------------------
void SpaceSquad::remove()
{
	s_formationPriorityList.clear();
}

// ----------------------------------------------------------------------
SpaceSquad::SpaceSquad()
 : Squad()
 , m_guardTarget(nullptr)
 , m_guardedByList(new SpaceSquadList)
 , m_attackSquadList(new AttackSquadList)
 , m_guarding(false)
 , m_leashAnchorPosition_w()
 , m_leashAnchorPositionTimer(4.0f)
{
}

// ----------------------------------------------------------------------
SpaceSquad::~SpaceSquad()
{
	// The the squad that I am guarding that I am no longer guarding it

	if (m_guardTarget != nullptr)
	{
		removeGuardTarget();
		m_guardTarget = nullptr;
	}

	// Tell all the squads guarding me that I am not longer guardable
    for (SpaceSquadList::iterator it = m_guardedByList->begin(), next_it = it; it != m_guardedByList->end(); it = next_it)
    {
        ++next_it;
        (*it)->removeGuardTarget();
    }

	delete m_guardedByList;

	// Clean up the attack squads

	{
		AttackSquadList::const_iterator iterAttackSquadList = m_attackSquadList->begin();

		for (; iterAttackSquadList != m_attackSquadList->end(); ++iterAttackSquadList)
		{
			SpaceAttackSquad * const attackSquad = NON_NULL(*iterAttackSquadList);

			delete attackSquad;
		}

		m_attackSquadList->clear();
		delete m_attackSquadList;
	}
}

// ----------------------------------------------------------------------
char const * SpaceSquad::getClassName() const
{
	return "SpaceSquad";
}

// ----------------------------------------------------------------------
SpacePath * SpaceSquad::getPath() const
{
	//-- Get the path of the squad leader

	AiShipController * const aiShipController = AiShipController::getAiShipController(getLeader());

	return nullptr != aiShipController ? aiShipController->getPath() : nullptr;
}

// ----------------------------------------------------------------------
void SpaceSquad::idle() const
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::idle() squadId(%d)", getId()));

	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;

		IGNORE_RETURN(AiShipControllerInterface::idle(unit));
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::track(Object const & target) const
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::track() squadId(%d) target(%s)", getId(), target.getNetworkId().getValueString().c_str()));

	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;

		IGNORE_RETURN(AiShipControllerInterface::track(unit, target));
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::moveTo(SpacePath * const path) const
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::moveTo() squadId(%d) path(0x%p) pathSize(%u)", getId(), path, (path != nullptr) ? path->getTransformList().size() : 0));

	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;

		IGNORE_RETURN(AiShipControllerInterface::moveTo(unit, path));
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::addPatrolPath(SpacePath * const path) const
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::addPatrolPath() squadId(%d) squadSize(%u) path(0x%p) pathSize(%u)", getId(), getUnitMap().size(), path, (path != nullptr) ? path->getTransformList().size() : 0));

	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;

		IGNORE_RETURN(AiShipControllerInterface::addPatrolPath(unit, path));
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::clearPatrolPath()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::clearPatrolPath() squadId(%d) squadSize(%u)", getId(), getUnitMap().size()));

	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;

		IGNORE_RETURN(AiShipControllerInterface::clearPatrolPath(unit));
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::follow(NetworkId const & followedUnit, Vector const & direction_l, float const distance) const
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::follow() squadId(%d) squadSize(%u) followedUnit(%s) direction_l(%.2f, %.2f, %.2f) distance(%.2f)", getId(), getUnitMap().size(), followedUnit.getValueString().c_str(), direction_l.x, direction_l.y, direction_l.z, distance));

	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;

		IGNORE_RETURN(AiShipControllerInterface::follow(unit, followedUnit, direction_l, distance));
	}
}

// ----------------------------------------------------------------------
bool SpaceSquad::setGuardTarget(int const squadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::setGuardTarget() squadId(%d) guardTarget(%d)", getId(), squadId));

	// A squad can't guard itself

	if (squadId == getId())
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("SpaceSquad::setGuardTarget() ERROR: The squad(%d) is trying to guard itself.", getId());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
		return false;
	}

	SpaceSquad * const desiredGuardTarget = SpaceSquadManager::getSquad(squadId);
	if (m_guardTarget != desiredGuardTarget)
	{
		//-- Tell the guard that I'm guarding that I'm no longer guarding it
		if (m_guardTarget)
			m_guardTarget->removeGuardedBy(*this);

		m_guardTarget = desiredGuardTarget;
		m_guardTarget->setGuardedBy(*this);
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("SpaceSquad::setGuardTarget() squadId(%d) ERROR: guardTarget(%d) does not exist", getId(), squadId);
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}

	return (m_guardTarget != nullptr);
}

// ----------------------------------------------------------------------
SpaceSquad * SpaceSquad::getGuardTarget()
{
	return m_guardTarget;
}

// ----------------------------------------------------------------------
void SpaceSquad::removeGuardTarget()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::removeGuardTarget() squadId(%d) guardedBySquad(%d)", getId(), (m_guardTarget != nullptr) ? m_guardTarget->getId() : 0));

	//-- Tell the guard target it's no longer being guarded

	if (m_guardTarget != nullptr)
	{
		m_guardTarget->removeGuardedBy(*this);
		m_guardTarget = nullptr;
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::addDamageTaken(NetworkId const & attackingUnit, float const damage) const
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::addDamageTaken() squadId(%d) attackingUnit(%s) damage(%.f)", getId(), attackingUnit.getValueString().c_str(), damage));

	AttackSquadList::iterator iterAttackSquadList = m_attackSquadList->begin();

	for (; iterAttackSquadList != m_attackSquadList->end(); ++iterAttackSquadList)
	{
		SpaceAttackSquad & attackSquad = *NON_NULL(*iterAttackSquadList);

		attackSquad.addDamageTaken(attackingUnit, damage);
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::setGuardedBy(SpaceSquad & squad)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::setGuardedBy() squadId(%d) guardedBySquad(%d)", getId(), squad.getId()));

	IGNORE_RETURN(m_guardedByList->insert(&squad));
}

// ----------------------------------------------------------------------

void SpaceSquad::removeGuardedBy(SpaceSquad & squad)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::removeGuardedBy() squadId(%d) guardedBySquad(%d)", getId(), squad.getId()));
	DEBUG_WARNING(m_guardedByList->find(&squad) == m_guardedByList->end(), ("The squad (%d) could not be found in our squad's (%d) guarded by list.", squad.getId(), getId()));

	IGNORE_RETURN(m_guardedByList->erase(&squad));
}

// ----------------------------------------------------------------------
SpaceSquad::SpaceSquadList & SpaceSquad::getGuardedByList()
{
	return *NON_NULL(m_guardedByList);
}

// ----------------------------------------------------------------------
void SpaceSquad::onAddUnit(NetworkId const & unit)
{
	AiShipController * unitAiShipController = AiShipController::getAiShipController(unit);
	
	if (unitAiShipController != nullptr)
	{
		unitAiShipController->setSquad(this);

		assignAttackSquad(*unitAiShipController);
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("SpaceSquad::onAddUnit() ERROR: squadId(%d) Trying to add a unit(%s) that can't resolve to an AiShipController.", getId(), unit.getValueString().c_str());
		WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}

	refreshPathInfo();
}

// ----------------------------------------------------------------------
void SpaceSquad::onRemoveUnit()
{
	refreshPathInfo();
}


// ----------------------------------------------------------------------
void SpaceSquad::onNewLeader(NetworkId const & oldLeader)
{
	AiShipController * const newLeaderAiShipController = AiShipController::getAiShipController(getLeader());

	if (newLeaderAiShipController != nullptr)
	{
		AiShipController * const oldLeaderAiShipController = AiShipController::getAiShipController(oldLeader);
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled() && (oldLeaderAiShipController == nullptr) && (getUnitCount() > 1), "space_debug_ai", ("Squad::onNewLeader() ERROR: The old leader(%s) could not resolve to an AiShipController.", oldLeader.getValueString().c_str()));

		if (oldLeaderAiShipController != nullptr)
		{
			newLeaderAiShipController->setCurrentPathIndex(oldLeaderAiShipController->getCurrentPathIndex());
		}

		m_leaderOffsetPosition_l = -newLeaderAiShipController->getFormationPosition_l();
		m_leashAnchorPosition_w = newLeaderAiShipController->getOwner()->getPosition_w();
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("SpaceSquad::onNewLeader() ERROR: The new leader(%s) could not be resolved to an AiShipController.", getLeader().getValueString().c_str());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::onSetUnitFormationPosition_l(NetworkId const & unit, Vector const & position_l)
{
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		aiShipController->setFormationPosition_l(position_l);
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("SpaceSquad::onSetUnitFormationPosition_l() ERROR: Unable to resolve AiShipController for unit(%s)", unit.getValueString().c_str());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::alter(float const deltaSeconds)
{
	Squad::alter(deltaSeconds);

	AttackSquadList::iterator iterAttackSquadList = m_attackSquadList->begin();

	for (; iterAttackSquadList != m_attackSquadList->end(); ++iterAttackSquadList)
	{
		SpaceAttackSquad & attackSquad = *NON_NULL(*iterAttackSquadList);

		attackSquad.alter(deltaSeconds);
	}

	if (m_leashAnchorPositionTimer.updateNoReset(deltaSeconds))
	{
		m_leashAnchorPositionTimer.reset();

		// If the leader is not attacking, then set a new leash position to keep all the squad units near the non-attacking units

		AiShipController const * const leaderAiShipController = AiShipController::getAiShipController(getLeader());

		if (leaderAiShipController != nullptr)
		{
			if (isGuarding())
			{
				SpaceSquad * const guardTarget = getGuardTarget();

				if (guardTarget != nullptr)
				{
					m_leashAnchorPosition_w = guardTarget->getSquadPosition_w();
				}
			}
			else if (!leaderAiShipController->isAttacking())
			{
				m_leashAnchorPosition_w = leaderAiShipController->getOwner()->getPosition_w();
			}

			leaderAiShipController->clampPositionToZone(m_leashAnchorPosition_w);
		}
	}

#ifdef _DEBUG
	verifyAttackSquads();
#endif // _DEBUG
}

// ----------------------------------------------------------------------
void SpaceSquad::assignNewLeader()
{
	UnitMap const & unitMap = getUnitMap();

	if (unitMap.size() == 1)
	{
		return;
	}

	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		CachedNetworkId const & unit = iterUnitMap->first;
		AiShipController const * const unitAiShipController = AiShipController::getAiShipController(unit);

		if (unitAiShipController != nullptr)
		{
			if (   unitAiShipController->getShipOwner()->isComponentFunctional(ShipChassisSlotType::SCST_engine)
			    && !unitAiShipController->isAttacking()
			    && !unitAiShipController->isDocking())
			{
				bool const rebuildFormation = false;

				if (!setLeader(unit, rebuildFormation))
				{
#ifdef _DEBUG
					FormattedString<1024> fs;
					char const * const text = fs.sprintf("SpaceSquad::assignNewLeader() squadId(%d) ERROR: Failure to set the new squad leader(%s)", getId(), unit.getValueString().c_str());
					DEBUG_WARNING(true, (text));
					LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
				}
				else
				{
					break;
				}
			}
		}
		else
		{
#ifdef _DEBUG
			FormattedString<1024> fs;
			char const * const text = fs.sprintf("SpaceSquad::assignNewLeader() ERROR: Could not convert unit(%s) to a ShipObject.", unit.getValueString().c_str());
			DEBUG_WARNING(true, (text));
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
		}
	}
}

// ----------------------------------------------------------------------
void SpaceSquad::setGuarding(bool const guarding)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquad::setGuarding() guarding(%s)", guarding ? "yes" : "no"));

	m_guarding = guarding;
}

// ----------------------------------------------------------------------
bool SpaceSquad::isGuarding() const
{
#ifdef _DEBUG
	if (   m_guarding
	    && !m_guardTarget)
	{
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("SpaceSquad::isGuarding() squadId(%d) ERROR: Why are we guarding a nullptr guard target?", getId());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
	}
#endif // _DEBUG

	return m_guarding;
}

// Returns whether every unit in the squad no longer has an attack target.
// ----------------------------------------------------------------------
bool SpaceSquad::isAttackTargetListEmpty() const
{
	bool result = true;
	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;
		AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

		if (   (aiShipController != nullptr)
		    && !aiShipController->getAttackTargetList().isEmpty())
		{
			result = false;
			break;
		}
		else
		{
#ifdef _DEBUG
			FormattedString<1024> fs;
			char const * const text = fs.sprintf("SpaceSquad::isAttackTargetListEmpty() ERROR: Why does the unit(%s) not have an AiShipController?", unit.getValueString().c_str());
			DEBUG_WARNING(true, (text));
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
		}
	}

	return result;
}

// ----------------------------------------------------------------------
Vector SpaceSquad::getAvoidanceVector(ShipObject const & unit) const
{
	Vector result;

	if (getUnitCount() <= 1)
	{
		return result;
	}

	// Make sure the average position has been updated this frame

	if (s_flockingSeperationGain > 0.0f)
	{
		Vector seperationVector;
		float closestDistanceSquared = std::numeric_limits<float>::max();
		float const comfortDistance = (unit.getRadius() * s_flockingComfortDistanceGain);
		UnitMap const & unitMap = getUnitMap();
		UnitMap::const_iterator iterUnitMap = unitMap.begin();

		for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
		{
			CachedNetworkId const & squadUnit = iterUnitMap->first;

			if (squadUnit == unit.getNetworkId())
			{
				// Don't check against ourselves
				continue;
			}

			Object * const squadUnitObject = squadUnit.getObject();
			ServerObject * const squadUnitServerObject = (squadUnitObject != nullptr) ? squadUnitObject->asServerObject() : nullptr;
			ShipObject * const squadUnitShipObject = (squadUnitServerObject != nullptr) ? squadUnitServerObject->asShipObject() : nullptr;

			if (squadUnitShipObject != nullptr)
			{
				Vector const & unitPosition_w = unit.getPosition_w();
				Vector const & squadUnitPosition_w = squadUnitShipObject->getPosition_w();
				float const distanceFromUnitSquared = Vector(squadUnitPosition_w - unitPosition_w).magnitudeSquared();

				if (   (distanceFromUnitSquared <= sqr(comfortDistance))
				    && (distanceFromUnitSquared < closestDistanceSquared))
				{
					closestDistanceSquared = distanceFromUnitSquared;

					seperationVector = (unitPosition_w - squadUnitPosition_w) * 1000.0f;
				}
			}
			else
			{
#ifdef _DEBUG
				FormattedString<1024> fs;
				char const * const text = fs.sprintf("SpaceSquad::getSquadAvoidanceVector() ERROR: Could not convert the squad unit(%s) to a ShipObject.", squadUnit.getValueString().c_str());
				DEBUG_WARNING(true, (text));
				LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
			}
		}

		if (closestDistanceSquared > sqr(comfortDistance))
		{
			result = Vector::zero;
		}
		else
		{
			seperationVector *= s_flockingSeperationGain;
			result += seperationVector;
		}
	}

	return result;
}

// ----------------------------------------------------------------------
Vector const & SpaceSquad::getLeashAnchorPosition_w() const
{
	return m_leashAnchorPosition_w;
}

// ----------------------------------------------------------------------
void SpaceSquad::assignAttackSquad(AiShipController & unitAiShipController)
{
	// Put the unit in an attack squad

	NetworkId const & unit = unitAiShipController.getOwner()->getNetworkId();
	AiShipController::AttackOrders const attackOrders = unitAiShipController.getAttackOrders();
	bool foundAttackSquad = false;
	
	if (attackOrders == AiShipController::AO_holdFire)
	{
		// Since the unit has HOLD_FIRE attack orders, it needs to be in an attack squad by itself

		AttackSquadList::iterator iterAttackSquadList = m_attackSquadList->begin();

		for (; iterAttackSquadList != m_attackSquadList->end(); ++iterAttackSquadList)
		{
			SpaceAttackSquad & attackSquad = *NON_NULL(*iterAttackSquadList);

			if (attackSquad.isEmpty())
			{
				attackSquad.addUnit(unit);
			}
			else if (   (attackSquad.getUnitCount() == 1)
			         && attackSquad.contains(unit))
			{
				foundAttackSquad = true;
				break;
			}
		}
	}
	else
	{
		// See if there is a squad that already contains the unit or one that has room for the unit

		AttackSquadList::iterator iterAttackSquadList = m_attackSquadList->begin();

		for (; iterAttackSquadList != m_attackSquadList->end(); ++iterAttackSquadList)
		{
			SpaceAttackSquad & attackSquad = *NON_NULL(*iterAttackSquadList);

			if (attackSquad.contains(unit))
			{
				foundAttackSquad = true;
				break;
			}
			else
			{
				bool const full = attackSquad.isFull();

				if (!full)
				{
					foundAttackSquad = true;
					attackSquad.addUnit(unit);

#ifdef _DEBUG
					verifyAttackSquads();
#endif // _DEBUG

					break;
				}
			}
		}
	}

	// If there was no room in an existing attack squad, create a new attack squad

	if (!foundAttackSquad)
	{
		// Add a new attack squad

		SpaceAttackSquad * const attackSquad = new SpaceAttackSquad;
		attackSquad->addUnit(unit);

		m_attackSquadList->push_back(attackSquad);
	}
}

// ----------------------------------------------------------------------
float SpaceSquad::getLargestShipRadius() const
{
	// Find the largest ship.
	float largestShipRadius = s_defaultShipRadius;
	
	UnitMap const & unitMap = getUnitMap();
	for (UnitMap::const_iterator iterUnitMap = unitMap.begin(); iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		CachedNetworkId const & unit = iterUnitMap->first;
		Object const * const unitObject = unit.getObject();
		ServerObject const * const unitServerObject = unitObject ? unitObject->asServerObject() : nullptr;
		
		if (unitServerObject) 
		{
			largestShipRadius = std::max(largestShipRadius, unitServerObject->getRadius());
		}
	}

	return largestShipRadius;
}

// ----------------------------------------------------------------------
void SpaceSquad::refreshPathInfo() const
{
	// check for at least one unit
	SpacePath * const path = getPath();
	if (path) 
	{
		float const largestShipRadius = getLargestShipRadius();
		
		if (!path->updateCollisionRadius(this, largestShipRadius)) 
		{
			UnitMap const & unitMap = getUnitMap();
			for (UnitMap::const_iterator iterUnitMap = unitMap.begin(); iterUnitMap != unitMap.end(); ++iterUnitMap)
			{
				CachedNetworkId const & unit = iterUnitMap->first;
				Object const * const unitObject = unit.getObject();
				ServerObject const * const unitServerObject = unitObject ? unitObject->asServerObject() : nullptr;
				if (unitObject && unitServerObject) 
				{
					IGNORE_RETURN(path->updateCollisionRadius(unitObject, unitServerObject->getRadius()));
				}
			}
		}
	}
}



#ifdef _DEBUG
// ----------------------------------------------------------------------
void SpaceSquad::verifyAttackSquads()
{
	// Make sure this unit is in exactly 1 attack squad

	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		CachedNetworkId const & unit = iterUnitMap->first;
		int attackSquadCount = 0;
		AttackSquadList::iterator iterAttackSquadList = m_attackSquadList->begin();

		for (; iterAttackSquadList != m_attackSquadList->end(); ++iterAttackSquadList)
		{
			SpaceAttackSquad const & attackSquad = *NON_NULL(*iterAttackSquadList);

			if (attackSquad.contains(unit))
			{
				++attackSquadCount;
			}
		}

#ifdef _DEBUG
		if (attackSquadCount > 1)
		{
			FormattedString<1024> fs;
			char const * const text = fs.sprintf("SpaceSquad::verifyAttackSquads() squadId(%d) unit(%s) ERROR: Unit is in more than one attack squad! attackSquadCount(%d)", getId(), unit.getValueString().c_str(), attackSquadCount);
			DEBUG_WARNING(true, (text));
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
		}

		if (attackSquadCount != 1)
		{
			FormattedString<1024> fs;
			char const * const text = fs.sprintf("SpaceSquad::verifyAttackSquads() squadId(%d) unit(%s) ERROR: Unit is not in an attack squad!", getId(), unit.getValueString().c_str());
			DEBUG_WARNING(true, (text));
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
		}
#endif // _DEBUG
	}
}
#endif // _DEBUG

// ======================================================================
