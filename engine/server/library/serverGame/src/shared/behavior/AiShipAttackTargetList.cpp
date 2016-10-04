// ======================================================================
//
// AiShipAttackTargetList.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipAttackTargetList.h"

#include "serverGame/AiShipController.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/ShipController.h"
#include "serverGame/ShipObject.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/UpdatePvpStatusMessage.h"
#include "sharedObject/NetworkIdManager.h"

#include <map>

// ======================================================================
//
// AiShipAttackTargetListNamespace
//
// ======================================================================

namespace AiShipAttackTargetListNamespace
{
	typedef std::vector<NetworkId> PurgeList;

	static PurgeList s_purgeList;

	class CheapPvpIsEnemyObserver
	{
	public:
		CheapPvpIsEnemyObserver(Object const * actor, Object const * target);
		~CheapPvpIsEnemyObserver();

	private:
		CheapPvpIsEnemyObserver();
		CheapPvpIsEnemyObserver(CheapPvpIsEnemyObserver const & copy);
		CheapPvpIsEnemyObserver & operator = (CheapPvpIsEnemyObserver const & copy);

	private:
		ConstWatcher<TangibleObject> m_actor;
		ConstWatcher<TangibleObject> m_target;
		bool m_targetIsEnemy;
	};

}

// ======================================================================

AiShipAttackTargetListNamespace::CheapPvpIsEnemyObserver::CheapPvpIsEnemyObserver(Object const * const actor, Object const * const target)
: m_actor(0)
, m_target(0)
, m_targetIsEnemy(false)
{
	ServerObject const * const actorSO = (actor != 0) ? actor->asServerObject() : 0;
	m_actor = (actorSO != 0) ? actorSO->asTangibleObject() : 0;

	ServerObject const * const targetSO = (target != 0) ? target->asServerObject() : 0;
	m_target = (targetSO != 0) ? targetSO->asTangibleObject() : 0;

	if ((m_actor != 0) && (m_target != 0))
	{
		m_targetIsEnemy = Pvp::isEnemy(*m_actor, *m_target);
	}
}

// ----------------------------------------------------------------------

AiShipAttackTargetListNamespace::CheapPvpIsEnemyObserver::~CheapPvpIsEnemyObserver()
{
	if ((m_actor != 0) && (m_target != 0))
	{
		if (m_targetIsEnemy != Pvp::isEnemy(*m_actor, *m_target))
		{
			typedef std::set<Client const *> AuthClients;
			AuthClients authClients;

			m_target->getAuthClients(authClients);
			AuthClients::const_iterator ii = authClients.begin();
			AuthClients::const_iterator iiEnd = authClients.end();

			for (; ii != iiEnd; ++ii)
			{
				Client const * const client = *ii;
				uint32 flags, factionId;
				Pvp::getClientVisibleStatus(*client, *m_actor, flags, factionId);
				UpdatePvpStatusMessage const statusMessage(m_actor->getNetworkId(), flags, factionId);
				client->send(statusMessage, true);
				PvpUpdateObserver::updatePvpStatusCache(client, *m_actor, flags, factionId);
			}
		}
	}
}

using namespace AiShipAttackTargetListNamespace;

// ======================================================================
//
// AiShipAttackTargetList::AiShipAttackTargetEntry
//
// ======================================================================

// ----------------------------------------------------------------------
AiShipAttackTargetList::AiShipAttackTargetEntry::AiShipAttackTargetEntry(float const initialDamage)
 : m_damage(initialDamage)
 , m_lastDamageTime(Os::getRealSystemTime())
{
}

// ======================================================================
//
// AiShipAttackTargetList
//
// ======================================================================

// ----------------------------------------------------------------------
AiShipAttackTargetList::AiShipAttackTargetList(ShipObject * const owner)
 : m_owner(NON_NULL(owner))
 , m_targetList(new TargetList)
 , m_primaryTarget(CachedNetworkId::cms_cachedInvalid)
 , m_primaryTargetDamage(0.0f)
{
}

// ----------------------------------------------------------------------
AiShipAttackTargetList::~AiShipAttackTargetList()
{
	// Notify all the targeted ships that they are no longer being targeted

	while (!m_targetList->empty())
	{
		NetworkId const & unit = m_targetList->begin()->first;
		bool const inDestructorHack = true;

		if (!remove(unit, inDestructorHack))
		{
			DEBUG_WARNING(true, ("debug_ai: AiShipAttackTargetList::~AiShipAttackTargetList() ERROR: Unable to remove the unit(%s) from the target list.", unit.getValueString().c_str()));
			break;
		}
	}

	delete m_targetList;
}

// ----------------------------------------------------------------------
void AiShipAttackTargetList::add(ShipObject & unit, float const damage)
{
#ifdef _DEBUG
	
	AiShipController * const ownerAiShipController = AiShipController::asAiShipController(m_owner->getController());

	if (ownerAiShipController != nullptr)
	{
		if (!ownerAiShipController->isValidTarget(unit))
		{
			DEBUG_WARNING(true, ("debug_ai: AiShipAttackTargetList::add() ERROR: Adding an invalid attack target to (%s) which has an exclusive aggro list and this unit(%s) is not in it.", m_owner->getDebugInformation().c_str(), unit.getDebugInformation().c_str()));
			return;
		}
	}
#endif // _DEBUG

	CheapPvpIsEnemyObserver observer(m_owner, &unit);

	// Add damage to myself

	float attackingUnitDamageTotal = damage;
	TargetList::iterator iterTargetList = m_targetList->find(CachedNetworkId(unit));

	if (iterTargetList != m_targetList->end())
	{
		iterTargetList->second.m_damage += damage;
		attackingUnitDamageTotal = iterTargetList->second.m_damage;
		iterTargetList->second.m_lastDamageTime = Os::getRealSystemTime();
	}
	else
	{
		AiShipAttackTargetEntry entry(damage);

		IGNORE_RETURN(m_targetList->insert(std::make_pair(CachedNetworkId(unit.getNetworkId()), entry)));

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipAttackTargetList::add() unit(%s) attackingUnit(%s) damage(%.f) ADDING NEW ATTACK TARGET", m_owner->getNetworkId().getValueString().c_str(), unit.getNetworkId().getValueString().c_str(), damage));
	}

	m_owner->getController()->asShipController()->onAttackTargetChanged(unit.getNetworkId());

	// See if this unit is the new primary attack target, and it is not the current primary attack target

	if (attackingUnitDamageTotal > m_primaryTargetDamage)
	{
		setNewPrimaryTarget(CachedNetworkId(unit), attackingUnitDamageTotal);
	}

	// Notify the unit that attacked me that I am targeting them

	ShipController * const attackingUnitShipController = unit.getController()->asShipController();

	if (attackingUnitShipController != nullptr)
	{
		attackingUnitShipController->addAiTargetingMe(m_owner->getNetworkId());
	}
	else
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipAttackTargetList::add() ERROR: owner(%s) Why does this unit(%s) not have a ShipController?", m_owner->getNetworkId().getValueString().c_str(), unit.getNetworkId().getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------
bool AiShipAttackTargetList::remove(NetworkId const & unit, bool const inDestructorHack)
{
	DEBUG_FATAL((unit == NetworkId::cms_invalid), ("Removing a nullptr unit"));

	bool result = false;

	// Remove the specified unit

	TargetList::iterator iterTargetList = m_targetList->find(CachedNetworkId(unit));

	if (iterTargetList != m_targetList->end())
	{
		Object * const object = NetworkIdManager::getObjectById(unit);

		CheapPvpIsEnemyObserver observer(m_owner, object);

		result = true;

		m_targetList->erase(iterTargetList);
		m_owner->getController()->asShipController()->onAttackTargetLost(unit);

		// Tell this unit that we are no longer targeting it

		if (object != nullptr)
		{
			ShipController * const shipController = object->getController()->asShipController();

			if (shipController != nullptr)
			{
				shipController->removeAiTargetingMe(m_owner->getNetworkId());
			}
			else
			{
				DEBUG_WARNING(true, ("debug_ai: AiShipAttackTargetList::remove() owner(%s) ERROR: Why does this unit(%s) not have an ShipController?", m_owner->getDebugInformation().c_str(), unit.getValueString().c_str()));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("debug_ai: AiShipAttackTargetList::remove() owner(%s) ERROR: The unit(%s) did not resolve to an Object.", m_owner->getDebugInformation().c_str(), unit.getValueString().c_str()));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("debug_ai: AiShipAttackTargetList::remove() owner(%s) ERROR: Unable to find the unit(%s) in the attack target list.", m_owner->getDebugInformation().c_str(), unit.getValueString().c_str()));
	}

	// If the unit being removed was the primary attack target, find a new primary attack target

	if (   !inDestructorHack
	    && (m_primaryTarget == unit))
	{
		findNewPrimaryTarget();
	}

	return result;
}

// ----------------------------------------------------------------------
CachedNetworkId const & AiShipAttackTargetList::getPrimaryTarget() const
{
	return m_primaryTarget;
}

// ----------------------------------------------------------------------
AiShipAttackTargetList::TargetList const & AiShipAttackTargetList::getUnSortedTargetList() const
{
	return *m_targetList;
}

// ----------------------------------------------------------------------
void AiShipAttackTargetList::getSortedTargetList(SortedTargetList & destTargetList) const
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipAttackTargetList::getSortedTargetList() owner(%s) m_targetList->size(%u)", m_owner->getNetworkId().getValueString().c_str(), m_targetList->size()));

	destTargetList.clear();

	// Sort the targets based on the most damage inflicted

	if (!m_targetList->empty())
	{
		// Sort the results
	
		typedef std::multimap<float, std::pair<CachedNetworkId, AiShipAttackTargetEntry> > SortedResults;
		SortedResults sortedResults;
		TargetList::const_iterator iterTargetList = m_targetList->begin();
	
		for (; iterTargetList != m_targetList->end(); ++iterTargetList)
		{
			IGNORE_RETURN(sortedResults.insert(std::make_pair(iterTargetList->second.m_damage, std::make_pair(iterTargetList->first, iterTargetList->second))));
		}
	
		// Save the sorted results
	
		destTargetList.reserve(sortedResults.size());
		SortedResults::reverse_iterator iterSortedResults = sortedResults.rbegin();
	
		for (; iterSortedResults != sortedResults.rend(); ++iterSortedResults)
		{
			destTargetList.push_back(std::make_pair(iterSortedResults->second.first, iterSortedResults->second.second));
		}
	}
}

// ----------------------------------------------------------------------
bool AiShipAttackTargetList::isEmpty() const
{
	return getUnSortedTargetList().empty();
}

// ----------------------------------------------------------------------
void AiShipAttackTargetList::purge(time_t const maxAge)
{
	// Remove targets that caused damaged too long ago

	if (!isEmpty())
	{
		time_t const systemTime = Os::getRealSystemTime();
		TargetList::iterator iterTargetList = m_targetList->begin();

		s_purgeList.clear();

		for (; iterTargetList != m_targetList->end(); ++iterTargetList)
		{
			NOT_NULL(iterTargetList->first.getObject());

			time_t const age = systemTime - iterTargetList->second.m_lastDamageTime;

			if (age >= maxAge)
			{
				LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipAttackTargetList::purge() target(%s) age(%u) maxAge(%u) m_targetList->size(%u)", iterTargetList->first.getValueString().c_str(), age, maxAge, m_targetList->size() - 1));

				NetworkId const lostUnit(iterTargetList->first);

				s_purgeList.push_back(lostUnit);
			}
		}

		PurgeList::const_iterator iterPurgeList = s_purgeList.begin();

		for (; iterPurgeList != s_purgeList.end(); ++iterPurgeList)
		{
			NetworkId const & unit = (*iterPurgeList);
			remove(unit);
		}
	}
}

// ----------------------------------------------------------------------
void AiShipAttackTargetList::findNewPrimaryTarget()
{
	CachedNetworkId newPrimaryTarget;
	float newPrimaryTargetDamage = 0.0f;

	// Walk all the attack targets and get the one with the most damage inflicted
	
	TargetList::const_iterator iterTargetList = m_targetList->begin();
	
	for (; iterTargetList != m_targetList->end(); ++iterTargetList)
	{
		NOT_NULL(iterTargetList->first.getObject());
	
		if (   (iterTargetList->second.m_damage > m_primaryTargetDamage)
		    || (newPrimaryTarget == CachedNetworkId::cms_cachedInvalid))
		{
			newPrimaryTarget = iterTargetList->first;
			newPrimaryTargetDamage = iterTargetList->second.m_damage;
		}
	}

	setNewPrimaryTarget(newPrimaryTarget, newPrimaryTargetDamage);

#ifdef _DEBUG
	if (m_targetList->empty())
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipAttackTargetList::findNewPrimaryTarget() m_owner(%s) NO TARGET (Empty target list)", m_owner->getNetworkId().getValueString().c_str()));
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipAttackTargetList::findNewPrimaryTarget() m_owner(%s) m_targetList->size(%u) primaryTarget(%s) damage(%f)", m_owner->getNetworkId().getValueString().c_str(), m_targetList->size(), m_primaryTarget.getValueString().c_str(), m_primaryTargetDamage));
	}

	// We should only have ship objects in our target list

	if (m_primaryTarget.getObject() != nullptr)
	{
		ServerObject * const targetServerObject = m_primaryTarget.getObject()->asServerObject();
		ShipObject * const targetShipObject = (targetServerObject != nullptr) ? targetServerObject->asShipObject() : nullptr;

		if (targetShipObject == nullptr)
		{
			DEBUG_WARNING(true, ("debug_ai: AiShipAttackTargetList::findNewPrimaryTarget() ERROR: How did we get a target that is not a ShipObject (%s)", m_primaryTarget.getObject()->getDebugInformation().c_str()));
		}
	}
#endif // _DEBUG
}

// ----------------------------------------------------------------------
void AiShipAttackTargetList::clear()
{
	if (!isEmpty())
	{
		purge(0);

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipAttackTargetList::clear() m_owner(%s) m_targetList->size(%u) primaryTarget(%s)", m_owner->getNetworkId().getValueString().c_str(), m_targetList->size(), m_primaryTarget.getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------
void AiShipAttackTargetList::setNewPrimaryTarget(CachedNetworkId const & newPrimaryTarget, float const damage)
{
	m_primaryTargetDamage = damage;

	if (m_primaryTarget != newPrimaryTarget)
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipAttackTargetList::setNewPrimaryTarget() m_owner(%s) newPrimaryTarge(%s) damage(%.2f) m_targetList->size(%u)", m_owner->getNetworkId().getValueString().c_str(), newPrimaryTarget.getValueString().c_str(), damage, m_targetList->size()));

		m_primaryTarget = newPrimaryTarget;

		m_owner->setPilotLookAtTarget(newPrimaryTarget);
	}
}

// ----------------------------------------------------------------------
void AiShipAttackTargetList::verify()
{
	// If the owner of the target list has exclusive aggros, then we might need to purge some of the targets from the list

	AiShipController * const ownerAiShipController = AiShipController::asAiShipController(m_owner->getController());

	if (ownerAiShipController != nullptr)
	{
		if (ownerAiShipController->hasExclusiveAggros())
		{
			// We have exclusive aggros, remove any invalid targets

			TargetList::iterator iterTargetList = m_targetList->begin();

			s_purgeList.clear();

			for (; iterTargetList != m_targetList->end(); ++iterTargetList)
			{
				ShipObject * const unitShipObject = ShipObject::asShipObject(iterTargetList->first.getObject());
				CreatureObject const * const unitPilot = (unitShipObject != nullptr) ? unitShipObject->getPilot() : nullptr;

				if (   (unitPilot == nullptr)
				    || !ownerAiShipController->isExclusiveAggro(*unitPilot))
				{
					s_purgeList.push_back(unitShipObject->getNetworkId());
				}
			}

			PurgeList::const_iterator iterPurgeList = s_purgeList.begin();

			for (; iterPurgeList != s_purgeList.end(); ++iterPurgeList)
			{
				NetworkId const & unit = (*iterPurgeList);
				remove(unit);
			}

			s_purgeList.clear();
		}
	}
}

// ======================================================================
