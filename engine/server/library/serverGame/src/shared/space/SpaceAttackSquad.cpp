// ======================================================================
// 
// SpaceAttackSquad.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SpaceAttackSquad.h"

#include "serverGame/AiPilotManager.h"
#include "serverGame/AiShipController.h"
#include "serverGame/AiShipControllerInterface.h"
#include "serverGame/AiShipPilotData.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ShipObject.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedLog/Log.h"

// ======================================================================
//
// SpaceAttackSquadNamespace
//
// ======================================================================

namespace SpaceAttackSquadNamespace
{
	int s_id = 1;
}

using namespace SpaceAttackSquadNamespace;

// ======================================================================
//
// SpaceAttackSquad
//
// ======================================================================

// ----------------------------------------------------------------------
SpaceAttackSquad::SpaceAttackSquad()
 : Squad()
 , m_inFormation(false)
 , m_tooCloseToTarget(false)
 , m_maxNumberOfUnits(2 + (rand() % 2))
 , m_chaseDistance(0.0f)
 , m_projectileAttackRange(0.0f)
 , m_weaponAttackRange(0.0f)
 , m_totalShotsFired(0)
{
	setId(s_id++);
	getFormation().setSpacing(3.0f);
	getFormation().setShape(Formation::S_wall);
}

// ----------------------------------------------------------------------
SpaceAttackSquad::~SpaceAttackSquad()
{
}

// ----------------------------------------------------------------------
char const * SpaceAttackSquad::getClassName() const
{
	return "SpaceAttackSquad";
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::onAddUnit(NetworkId const & unit)
{
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);
	
	if (aiShipController != nullptr)
	{
		aiShipController->setAttackSquad(this);
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("SpaceAttackSquad::onAddUnit() id(%d) ERROR: Trying to add a unit(%s) that can't resolve to an AiShipController.", getId(), unit.getValueString().c_str());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}

	calculateAttackRanges();
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::onRemoveUnit()
{
	calculateAttackRanges();
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::onNewLeader(NetworkId const & /*oldLeader*/)
{
	AiShipController * const newLeaderAiShipController = AiShipController::getAiShipController(getLeader());

	if (newLeaderAiShipController != nullptr)
	{
		m_leaderOffsetPosition_l = -newLeaderAiShipController->getFormationPosition_l();
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("SpaceAttackSquad::onNewLeader() ERROR: The new leader(%s) could not be resolved to an AiShipController.", getLeader().getValueString().c_str());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::onSetUnitFormationPosition_l(NetworkId const & unit, Vector const & position_l)
{
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		aiShipController->setAttackFormationPosition_l(position_l);
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("SpaceAttackSquad::onSetUnitFormationPosition_l() ERROR: Unable to resolve AiShipController for unit(%s)", unit.getValueString().c_str());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::addDamageTaken(NetworkId const & attackingUnit, float const damage) const
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceAttackSquad::addDamageTaken() squadId(%d) attackingUnit(%s) damage(%.f)", getId(), attackingUnit.getValueString().c_str(), damage));

	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;
		bool const notifySquad = false;
		bool const checkPlayerAttacker = true;

		IGNORE_RETURN(AiShipControllerInterface::addDamageTaken(unit, attackingUnit, damage, notifySquad, checkPlayerAttacker));
	}
}

// ----------------------------------------------------------------------
CachedNetworkId const & SpaceAttackSquad::getPrimaryAttackTarget() const
{
	Object * const leaderObject = getLeader().getObject();

	if (leaderObject != nullptr)
	{
		AiShipController * const leaderAiShipController = AiShipController::asAiShipController(leaderObject->getController());

		return leaderAiShipController->getPrimaryAttackTarget();
	}

#ifdef _DEBUG
	FormattedString<1024> fs;
	char const * const text = fs.sprintf("SpaceAttackSquad::getPrimaryAttackTarget() ERROR: Why is the attack squad leader(%s) object nullptr?", getLeader().getValueString().c_str());
	DEBUG_WARNING(true, (text));
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG

	return CachedNetworkId::cms_cachedInvalid;
}

// ----------------------------------------------------------------------
bool SpaceAttackSquad::isAttacking() const
{
	Object * const leaderObject = getLeader().getObject();

	if (leaderObject != nullptr)
	{
		AiShipController * const leaderAiShipController = AiShipController::asAiShipController(leaderObject->getController());

		return leaderAiShipController->isAttacking();
	}

#ifdef _DEBUG
	FormattedString<1024> fs;
	char const * const text = fs.sprintf("SpaceAttackSquad::isAttacking() ERROR: Why is the attack squad leader(%s) object nullptr?", getLeader().getValueString().c_str());
	DEBUG_WARNING(true, (text));
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG

	return false;
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::setInFormation(bool const inFormation)
{
	//LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceAttackSquad::setInFormation() inFormation(%s)", inFormation ? "yes" : "no"));

	m_inFormation = inFormation;
}

// ----------------------------------------------------------------------
bool SpaceAttackSquad::isInFormation() const
{
	return m_inFormation;
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::setTooCloseToTarget(bool const tooCloseToTarget)
{
	//LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceAttackSquad::setTooCloseToTarget() tooCloseToTarget(%s)", tooCloseToTarget ? "yes" : "no"));

	m_tooCloseToTarget = tooCloseToTarget;
}

// ----------------------------------------------------------------------
bool SpaceAttackSquad::isTooCloseToTarget() const
{
	return m_tooCloseToTarget;
}

// ----------------------------------------------------------------------
int SpaceAttackSquad::getMaxNumberOfUnits() const
{
	UnitMap const & unitMap = getUnitMap();

	if (unitMap.size() == 1)
	{
		NetworkId const & unit = unitMap.begin()->first;
		AiShipController * const unitAiShipController = AiShipController::getAiShipController(unit);

		if (unitAiShipController != nullptr)
		{
			if (unitAiShipController->getAttackOrders() == AiShipController::AO_holdFire)
			{
				return 1;
			}
		}
	}

	return m_maxNumberOfUnits;
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::setChaseDistance(float const chaseDistance)
{
	//LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceAttackSquad::setChaseDistance() chaseDistance(%.f)", chaseDistance));

	m_chaseDistance = chaseDistance;
}

// ----------------------------------------------------------------------
float SpaceAttackSquad::getChaseDistance() const
{
	return m_chaseDistance;
}

// ----------------------------------------------------------------------
AiPilotManager const & SpaceAttackSquad::getLeaderPilotManagerInfo() const
{
	Object * const leaderObject = NON_NULL(getLeader().getObject());
	AiShipController * const leaderAiShipController = NON_NULL(AiShipController::asAiShipController(leaderObject->getController()));

	return leaderAiShipController->getPilotManagerInfo();
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::calculateAttackRanges()
{
	if (!isEmpty())
	{
		m_projectileAttackRange = FLT_MAX;
		m_weaponAttackRange = FLT_MAX;
	}

	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		CachedNetworkId const & unit = iterUnitMap->first;
		Object * const unitObject = unit.getObject();
		ServerObject * const unitServerObject = (unitObject != nullptr) ? unitObject->asServerObject() : nullptr;
		ShipObject * const unitShipObject = (unitServerObject != nullptr) ? unitServerObject->asShipObject() : nullptr;

		if (unitShipObject != nullptr)
		{
			m_projectileAttackRange = std::min(m_projectileAttackRange, unitShipObject->getApproximateAttackRange());
			
			bool includeMissiles = true;
			m_weaponAttackRange = std::min(m_projectileAttackRange, unitShipObject->getApproximateAttackRange(includeMissiles));
		}
		else
		{
#ifdef _DEBUG
			FormattedString<1024> fs;
			char const * const text = fs.sprintf("SpaceAttackSquad::calculateProjectileAttackRange() ERROR: Could not convert unit(%s) to a ShipObject.", unit.getValueString().c_str());
			DEBUG_WARNING(true, (text));
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
		}
	}

	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceAttackSquad::calculateProjectileAttackRange() m_projectileAttackRange(%.0f) m_weaponAttackRange(%.0f)", m_projectileAttackRange, m_weaponAttackRange));
}

// ----------------------------------------------------------------------
float SpaceAttackSquad::getProjectileAttackRange() const
{
	return m_projectileAttackRange;
}

// ----------------------------------------------------------------------
float SpaceAttackSquad::getWeaponAttackRange() const
{
	return m_weaponAttackRange;
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::assignNewLeader()
{
	UnitMap const & unitMap = getUnitMap();
	UnitMap::const_iterator iterUnitMap = unitMap.begin();

	for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
	{
		CachedNetworkId const & unit = iterUnitMap->first;
		Object * const unitObject = unit.getObject();
		ServerObject * const unitServerObject = (unitObject != nullptr) ? unitObject->asServerObject() : nullptr;
		ShipObject * const unitShipObject = (unitServerObject != nullptr) ? unitServerObject->asShipObject() : nullptr;

		if (unitShipObject != nullptr)
		{
			if (unitShipObject->isComponentFunctional(ShipChassisSlotType::SCST_engine))
			{
				bool const rebuildFormation = false;

				if (!setLeader(unit, rebuildFormation))
				{
#ifdef _DEBUG
					FormattedString<1024> fs;
					char const * const text = fs.sprintf("SpaceAttackSquad::assignNewLeader() squadId(%d) ERROR: Failure to set the new squad leader(%s)", getId(), unit.getValueString().c_str());
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
			char const * const text = fs.sprintf("SpaceAttackSquad::assignNewLeader() ERROR: Could not convert unit(%s) to a ShipObject.", unit.getValueString().c_str());
			DEBUG_WARNING(true, (text));
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
		}
	}
}

// ----------------------------------------------------------------------
bool SpaceAttackSquad::isFull() const
{
	int const unitCount = getUnitCount();
	int maxNumberOfUnits = getMaxNumberOfUnits();

	return (unitCount >= maxNumberOfUnits);
}

// ----------------------------------------------------------------------
int SpaceAttackSquad::getNumberOfShotsToFire() const
{
	Object * const leaderObject = NON_NULL(getLeader().getObject());
	AiShipController const & leaderAiShipController = *NON_NULL(AiShipController::asAiShipController(leaderObject->getController()));
	AiShipPilotData const & pilotData = *NON_NULL(leaderAiShipController.getPilotData());

	return pilotData.m_fighterChaseMaxShots * getUnitCount();
}

// ----------------------------------------------------------------------
int SpaceAttackSquad::getTotalShotsFired() const
{
	return m_totalShotsFired;
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::clearShotsFired()
{
	m_totalShotsFired = 0;
}

// ----------------------------------------------------------------------
void SpaceAttackSquad::increaseShotsFired()
{
	++m_totalShotsFired;
}

// ======================================================================
