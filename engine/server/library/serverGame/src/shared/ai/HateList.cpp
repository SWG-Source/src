// ======================================================================
//
// HateList.cpp
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/HateList.h"

#include "serverGame/AggroListProperty.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/AiLogManager.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Pvp.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/UpdatePvpStatusMessage.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"

#include <map>
#include <set>
#include <vector>


// ======================================================================
//
// HateList
//
// ======================================================================

// ----------------------------------------------------------------------
HateList::HateList()
 : m_owner(nullptr)
 , m_playerObject(nullptr)
 , m_hateList()
 , m_target(CachedNetworkId::cms_cachedInvalid)
 , m_maxHate(0.0f)
 , m_lastUpdateTime(0)
 , m_autoExpireTargetDuration(ConfigServerGame::getDefaultAutoExpireTargetDuration())
 , m_recentHateList()
{
}

// ----------------------------------------------------------------------
HateList::~HateList()
{
	clear();
	m_owner = nullptr;
	m_playerObject = nullptr;
}

// ----------------------------------------------------------------------
void HateList::alter()
{
	if (!isEmpty())
	{
		if (!isOwnerValid())
		{
			clear();
		}
		else
		{
			// Determine if it is time to expire the current target
			{
				if (   (getAutoExpireTargetDuration() > 0)
					&& (getTimeSinceLastUpdate() > getAutoExpireTargetDuration()))
				{
					LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::alter() owner(%s:%s) Auto-clearing hate list primary target due to lack of activity", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));

					// Remove the primary target

					removeTarget(getTarget());
				}
			}
			
			while (   (m_target.get() != CachedNetworkId::cms_cachedInvalid)
				   && !isValidTarget(m_target.get().getObject()))
			{
				removeTarget(m_target.get());
			}
		}
	}
}

// ----------------------------------------------------------------------
bool HateList::addHate(NetworkId const & target, float const hate)
{
	bool result = false;

	if (target == m_owner->getNetworkId())
	{
		DEBUG_WARNING(true, ("HateList::addHate() owner(%s) Owner trying to add hate to itself.", m_owner->getDebugInformation().c_str()));
	}
	else if (!isOwnerValid())
	{
		DEBUG_WARNING(true, ("HateList::addHate() owner(%s) Invalid owner specified", m_owner->getDebugInformation().c_str()));
	}
	else
	{
		Object * const targetObject = NetworkIdManager::getObjectById(target);

		if (!isValidTarget(targetObject))
		{
			DEBUG_WARNING(true, ("HateList::addHate() owner(%s) Invalid target(%s) specified", m_owner->getDebugInformation().c_str(), target.getValueString().c_str()));
		}
		else
		{
			// If a target AI has a master, the target and the master needs to be added to the hate list (ie. pets should cause their master to gain hate)
			{
				CreatureObject const * const targetCreatureObject = CreatureObject::asCreatureObject(targetObject);
				NetworkId const & masterId = (targetCreatureObject != nullptr) ? targetCreatureObject->getMasterId() : NetworkId::cms_invalid;

				if (masterId != NetworkId::cms_invalid)
				{
					addHate(masterId, 0.0f);
				}
			}

			// A > 0 hate causes the owner to send out an OnSawAttack() trigger about the target

			if (hate > 0.0f)
			{
				m_recentHateList.insert(CachedNetworkId(target));
			}

			resetHateTimer();

			float totalTargetHate = hate;
			UnSortedList::const_iterator iterHateList = m_hateList.find(CachedNetworkId(target));

			if (iterHateList != m_hateList.end())
			{
				totalTargetHate = iterHateList->second + hate;

				//LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::addHate() owner(%s:%s) target(%s) hate(%.2f+%.2f=%.2f)", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str(), target.getValueString().c_str(), iterHateList->second, hate, totalTargetHate));

				m_hateList.set(iterHateList->first, totalTargetHate);
			}
			else
			{
				IGNORE_RETURN(m_hateList.insert(CachedNetworkId(target), hate));
				if (m_playerObject)
					m_playerObject->addToPlayerHateList(target);

				triggerTargetAdded(target);

				//LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::addHate() owner(%s:%s) target(%s) hate(0+%.2f=%.2f)", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str(), target.getValueString().c_str(), hate, hate));
			}

			if (   (m_target.get() == CachedNetworkId::cms_cachedInvalid)
				|| (totalTargetHate > m_maxHate.get()))
			{
				setTarget(CachedNetworkId(target), totalTargetHate);
			}

			result = true;
		}
	}

	return result;
}

// ----------------------------------------------------------------------
bool HateList::setHate(NetworkId const & target, float const hate)
{
	bool result = false;

	if (target == m_owner->getNetworkId())
	{
		DEBUG_WARNING(true, ("HateList::setHate() owner(%s) Owner trying to add hate to itself.", m_owner->getDebugInformation().c_str()));
	}
	else if (!isOwnerValid())
	{
		DEBUG_WARNING(true, ("HateList::setHate() owner(%s) Invalid owner specified", m_owner->getDebugInformation().c_str()));
	}
	else if (!isValidTarget(NetworkIdManager::getObjectById(target)))
	{
		DEBUG_WARNING(true, ("HateList::setHate() owner(%s) Invalid target(%s) specified", m_owner->getDebugInformation().c_str(), target.getValueString().c_str()));
	}
	else
	{
		// A > 0 hate causes the owner to send out an OnSawAttack() trigger about the target

		if (hate > 0.0f)
		{
			m_recentHateList.insert(CachedNetworkId(target));
		}

		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::setHate() owner(%s:%s) target(%s) hate(%.2f)", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str(), target.getValueString().c_str(), hate));

		resetHateTimer();

		UnSortedList::const_iterator iterHateList = m_hateList.find(CachedNetworkId(target));

		if (iterHateList != m_hateList.end())
		{
			m_hateList.set(iterHateList->first, hate);

			if(getTarget() == iterHateList->first && hate < m_maxHate.get())
				findNewTarget();
		}
		else
		{
			IGNORE_RETURN(m_hateList.insert(CachedNetworkId(target), hate));
			if (m_playerObject)
				m_playerObject->addToPlayerHateList(target);

			triggerTargetAdded(target);
		}

		if (   (m_target.get() == CachedNetworkId::cms_cachedInvalid)
			|| (hate > m_maxHate.get()))
		{
			setTarget(CachedNetworkId(target), hate);
		}

		result = true;
	}

	return result;
}

// ----------------------------------------------------------------------
float HateList::getHate(NetworkId const & target) const
{
	float result = 0.0f;
	UnSortedList::const_iterator iterHateList = m_hateList.find(CachedNetworkId(target));

	if (iterHateList != m_hateList.end())
	{
		result = iterHateList->second;
	}

	return result;
}

// ----------------------------------------------------------------------
float HateList::getMaxHate() const
{
	return m_maxHate.get();
}

// ----------------------------------------------------------------------
bool HateList::removeTarget(NetworkId const & target)
{
	bool result = false;

	// Remove the specified unit

	UnSortedList::const_iterator iterHateList = m_hateList.find(CachedNetworkId(target));

	if (iterHateList != m_hateList.end())
	{
		result = true;

		m_hateList.erase(iterHateList);
		if (m_playerObject)
			m_playerObject->removeFromPlayerHateList(target);

		triggerTargetRemoved(target);

		// Transfer the hate target back to the aggro list
		{
			AggroListProperty * const aggroList = AggroListProperty::getAggroListProperty(*m_owner);

			if (aggroList != nullptr)
			{
				aggroList->addTarget(target);
			}
		}
	}
	else
	{
		DEBUG_WARNING(true, ("HateList::removeTarget() owner(%s auth=%s) Unable to find the target(%s) in the hate list.", m_owner->getDebugInformation().c_str(), (m_owner->isAuthoritative() ? "yes" : "no"), target.getValueString().c_str()));
	}

	// If the current target was removed, find a new one

	if (m_target.get() == target)
	{
		findNewTarget();
	}

	return result;
}

// ----------------------------------------------------------------------
bool HateList::isValidTarget(Object * const target)
{
	bool valid = true;
	TangibleObject * const targetTangibleObject = TangibleObject::asTangibleObject(target);

	if (targetTangibleObject == nullptr)
	{
		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isValidTarget() owner(%s:%s) TARGET IS NOT A TANGIBLEOBJECT", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
		valid = false;
	}
	else if (m_owner->getSceneId() != targetTangibleObject->getSceneId())
	{
		// Not in the same scene

		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isValidTarget() owner(%s:%s) TARGET IS NOT IN THE SAME SCENE", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
		valid = false;
	}
	else if (m_owner->getPosition_w().magnitudeBetweenSquared(targetTangibleObject->getPosition_w()) > sqr(getMaxDistanceToTarget()))
	{
		// Objects are too far from each other

		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isValidTarget() owner(%s:%s) TARGET IS TOO FAR AWAY", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
		valid = false;
	}
	else
	{
		int temp = 0;

		if (   targetTangibleObject->isInvulnerable()
			|| (targetTangibleObject->getObjVars().getItem("gm", temp) && (temp > 0)))
		{
			LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isValidTarget() owner(%s:%s) TARGET IS A GM", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
			valid = false;
		}
		else
		{
			CreatureObject const * const targetCreatureObject = targetTangibleObject->asCreatureObject();

			if (targetCreatureObject != nullptr)
			{
				if (targetTangibleObject->isDisabled())
				{
					LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isValidTarget() owner(%s:%s) TARGET IS DISABLED", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
					valid = false;
				}
				else if (targetCreatureObject->isIncapacitated())
				{
					LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isValidTarget() owner(%s:%s) TARGET IS INCAPACITATED", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
					valid = false;
				}
				else if (targetCreatureObject->isDead())
				{
					LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isValidTarget() owner(%s:%s) TARGET IS DEAD", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
					valid = false;
				}
				else
				{
					AICreatureController const * const targetAiCreatureController = AICreatureController::asAiCreatureController(targetCreatureObject->getController());

					if (   (targetAiCreatureController != nullptr)
						&& targetAiCreatureController->isRetreating())
					{
						LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isValidTarget() owner(%s:%s) TARGET IS RETREATING", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
						valid = false;
					}
					else
					{
						// If the owner object is not a creature object, we are
						// just going to allow the hate to be added to it, regardless
						// of the pvp rules. For instance, if a player attacks a lair,
						// crate, or turret, etc. we want those objects to add hate to
						// themselves towards the player so that they and the player
						// enter combat correctly.
						{
							if (   (m_owner->asCreatureObject() != nullptr)
								&& !Pvp::canAttack(*m_owner, *targetTangibleObject))
							{
								LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isValidTarget() owner(%s:%s) PVP CAN'T ATTACK", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
								valid = false;
							}
						}
					}
				}
			}
		}
	}

	return valid;
}

// ----------------------------------------------------------------------
CachedNetworkId const & HateList::getTarget() const
{
#ifdef _DEBUG
	if (   (m_target.get() == CachedNetworkId::cms_cachedInvalid)
	    && !isEmpty())
	{
		WARNING(true, ("HateList::getTarget() owner(%s) m_hateList.size(%u) m_target is nullptr but the HateList is not empty.", m_owner->getDebugInformation().c_str(), m_hateList.size()));
	}
#endif // _DEBUG

	return m_target.get();
}

// ----------------------------------------------------------------------
HateList::UnSortedList const & HateList::getUnSortedList() const
{
	return m_hateList.getMap();
}

// ----------------------------------------------------------------------
void HateList::getSortedList(SortedList & sortedList) const
{
	//LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::getSortedList() m_hateList.size(%u)", m_hateList.size()));

	sortedList.clear();

	// Sort the targets based on the most hate

	if (!m_hateList.empty())
	{
		// Sort the results

		typedef std::multimap<float, CachedNetworkId> SortedResults;
		SortedResults sortedResults;
		UnSortedList::const_iterator iterHateList = m_hateList.begin();

		for (; iterHateList != m_hateList.end(); ++iterHateList)
		{
			IGNORE_RETURN(sortedResults.insert(std::make_pair(iterHateList->second, iterHateList->first)));
		}

		// Save the sorted results

		sortedList.reserve(sortedResults.size());
		SortedResults::reverse_iterator iterSortedResults = sortedResults.rbegin();

		for (; iterSortedResults != sortedResults.rend(); ++iterSortedResults)
		{
			sortedList.push_back(std::make_pair(iterSortedResults->second, iterSortedResults->first));
		}
	}
}

// ----------------------------------------------------------------------
bool HateList::isEmpty() const
{
	return getUnSortedList().empty();
}

// ----------------------------------------------------------------------
void HateList::findNewTarget()
{
	CachedNetworkId target;
	float maxHate = 0.0f;

	// Walk all the attack targets and get the one with the most damage inflicted

	UnSortedList::const_iterator iterHateList = m_hateList.begin();

	for (; iterHateList != m_hateList.end(); ++iterHateList)
	{
		if (iterHateList->first.getObject() == nullptr)
		{
			// This target will be removed in the next alter call
			continue;
		}

		if (   (iterHateList->second > maxHate)
		    || (target == CachedNetworkId::cms_cachedInvalid))
		{
			target = iterHateList->first;
			maxHate = iterHateList->second;
		}
	}

	setTarget(target, maxHate);
}

// ----------------------------------------------------------------------
void HateList::clear()
{
	m_hateList.clear();
	if (m_playerObject)
		m_playerObject->clearPlayerHateList();
	m_target = CachedNetworkId::cms_cachedInvalid;
	m_maxHate = 0.0f;
	m_recentHateList.clear();
}

// ----------------------------------------------------------------------
bool HateList::isOnList(NetworkId const & target) const
{
	return (m_hateList.find(CachedNetworkId(target)) != m_hateList.end());
}

// ----------------------------------------------------------------------
void HateList::setTarget(CachedNetworkId const & target, float const hate)
{
	m_maxHate = hate;

	if (m_target.get() != target)
	{
		m_target = target;

		if (target != CachedNetworkId::cms_cachedInvalid)
		{
			triggerTargetChanged(target);
		}
	}

#ifdef _DEBUG
	// We should only have tangible objects in our target list

	if (m_target.get().getObject() != nullptr)
	{
		TangibleObject const * const targetTangibleObject = TangibleObject::asTangibleObject(m_target.get().getObject()->asServerObject());

		if (targetTangibleObject == nullptr)
		{
			WARNING(true, ("HateList::setTarget() owner(%s) How did we get a target(%s) that is not a TangibleObject", m_owner->getDebugInformation().c_str(), m_target.get().getObject()->getDebugInformation().c_str()));
		}
	}
#endif // _DEBUG
}

// ----------------------------------------------------------------------
void HateList::addServerNpAutoDeltaVariables(Archive::AutoDeltaByteStream & stream)
{
	stream.addVariable(m_hateList);
	stream.addVariable(m_target);
	stream.addVariable(m_maxHate);
	stream.addVariable(m_lastUpdateTime);
	stream.addVariable(m_autoExpireTargetDuration);
}

// ----------------------------------------------------------------------
int HateList::getTimeSinceLastUpdate() const
{
	time_t const currentTime = Os::getRealSystemTime();
	time_t const timeSinceActivitiy = (currentTime - m_lastUpdateTime.get());

	return clamp(0, static_cast<int>(timeSinceActivitiy), 1024);
}

// ----------------------------------------------------------------------
void HateList::setOwner(TangibleObject * const owner)
{
	m_owner = owner;

	CreatureObject * const creatureObject = m_owner ? m_owner->asCreatureObject() : 0;

	if (creatureObject)
		m_playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	else
		m_playerObject = 0;
}

// ----------------------------------------------------------------------
void HateList::triggerTargetChanged(NetworkId const & target)
{
	GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(m_owner);

	if (gameScriptObject != nullptr)
	{
		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::triggerTargetChanged() owner(%s:%s) target(%s)", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str(), target.getValueString().c_str()));

		ScriptParams scriptParams;
		scriptParams.addParam(target);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_HATE_TARGET_CHANGED, scriptParams));
	}
	else
	{
		WARNING(true, ("HateList::triggerTargetChanged() Unable to get the ScriptObject for this owner(%s)", m_owner->getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------
void HateList::triggerTargetAdded(NetworkId const & target)
{
	GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(m_owner);

	if (gameScriptObject != nullptr)
	{
		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::triggerTargetAdded() owner(%s:%s) target(%s)", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str(), target.getValueString().c_str()));

		ScriptParams scriptParams;
		scriptParams.addParam(target);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_HATE_TARGET_ADDED, scriptParams));
	}
	else
	{
		WARNING(true, ("HateList::triggerTargetAdded() Unable to get the ScriptObject for this owner(%s)", m_owner->getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------
void HateList::triggerTargetRemoved(NetworkId const & target)
{
	GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(m_owner);

	if (gameScriptObject != nullptr)
	{
		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::triggerTargetRemoved() owner(%s:%s) target(%s)", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str(), target.getValueString().c_str()));

		ScriptParams scriptParams;
		scriptParams.addParam(target);
		IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_HATE_TARGET_REMOVED, scriptParams));
	}
	else
	{
		WARNING(true, ("HateList::triggerTargetRemoved() Unable to get the ScriptObject for this owner(%s)", m_owner->getDebugInformation().c_str()));
	}
}

// ----------------------------------------------------------------------
int HateList::getAutoExpireTargetDuration() const
{
	int result = static_cast<int>(m_autoExpireTargetDuration.get());

	if (!m_owner->getParentCell()->isWorldCell())
	{
		result = static_cast<int>(result * ConfigServerGame::getInteriorTargetDurationFactor());
	}

	return result;
}

// ----------------------------------------------------------------------
bool HateList::isOwnerValid() const
{
	CreatureObject const * const ownerCreature = CreatureObject::asCreatureObject(m_owner);
	bool const ownerIncapacitated = (ownerCreature != nullptr) ? ownerCreature->isIncapacitated() : false;

	if (ownerIncapacitated)
	{
		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isOwnerValid() owner(%s:%s) OWNER IS INCAPACITATED", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
		return false;
	}

	bool const ownerDead = (ownerCreature != nullptr) ? ownerCreature->isDead() : false;

	if (ownerDead)
	{
		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isOwnerValid() owner(%s:%s) OWNER IS DEAD", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
		return false;
	}

	bool const ownerDisabled = (ownerCreature != nullptr) ? ownerCreature->isDisabled() : false;

	if (ownerDisabled)
	{
		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isOwnerValid() owner(%s:%s) OWNER IS DISABLED", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
		return false;
	}

	AICreatureController const * const ownerAiCreatureController = AICreatureController::asAiCreatureController(m_owner->getController());
	const bool ownerRetreating = (ownerAiCreatureController != nullptr) ? ownerAiCreatureController->isRetreating() : false;

	if (ownerRetreating)
	{
		LOGC(AiLogManager::isLogging(m_owner->getNetworkId()), "debug_ai", ("HateList::isOwnerValid() owner(%s:%s) OWNER IS RETREATING", m_owner->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_owner->getDebugName(), FileNameUtils::fileName).c_str()));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------
HateList::RecentList const & HateList::getRecentList() const
{
	return m_recentHateList;
}

// ----------------------------------------------------------------------
void HateList::clearRecentList()
{
	m_recentHateList.clear();
}

// ----------------------------------------------------------------------
void HateList::resetHateTimer()
{
	m_lastUpdateTime = Os::getRealSystemTime();

	// for NPC, reset the hate timer of every player on the NPC's hate list
	if (!m_playerObject && !m_hateList.empty())
	{
		for (UnSortedList::const_iterator iterHateList = m_hateList.begin(); iterHateList != m_hateList.end(); ++iterHateList)
		{
			TangibleObject * const to = TangibleObject::asTangibleObject(iterHateList->first.getObject());
			if (to && to->isHateListOwnerPlayer())
			{
				to->resetHateTimer();
			}
		}
	}
}

// ----------------------------------------------------------------------
float HateList::getMaxDistanceToTarget()
{
	return (ConfigServerGame::getMaxCombatRange() * 2.0f);
}

// ----------------------------------------------------------------------
void HateList::setAutoExpireTargetEnabled(bool const enabled)
{
	if (enabled)
	{
		m_autoExpireTargetDuration = ConfigServerGame::getDefaultAutoExpireTargetDuration();
	}
	else
	{
		m_autoExpireTargetDuration = 0;
	}
}

// ----------------------------------------------------------------------
bool HateList::isAutoExpireTargetEnabled() const
{
	return (getAutoExpireTargetDuration() > 0);
}

// ----------------------------------------------------------------------
float HateList::getAILeashTime( ) const
{
	return static_cast<float>(m_autoExpireTargetDuration.get());
}

// ----------------------------------------------------------------------
void HateList::setAILeashTime( float time )
{
	m_autoExpireTargetDuration.set( static_cast<time_t>(time) );
}

void HateList::forceHateTarget(const NetworkId &target)
{
	if(!isOnList(target)) // Creature isn't on our hate list.
		return;

	m_maxHate = getHate(target);

	if (m_target.get() != CachedNetworkId(target))
	{
		m_target = CachedNetworkId(target);

		if (m_target.get() != CachedNetworkId::cms_cachedInvalid)
		{
			triggerTargetChanged(target);
		}
	}

#ifdef _DEBUG
	// We should only have tangible objects in our target list

	if (m_target.get().getObject() != nullptr)
	{
		TangibleObject const * const targetTangibleObject = TangibleObject::asTangibleObject(m_target.get().getObject()->asServerObject());

		if (targetTangibleObject == nullptr)
		{
			WARNING(true, ("HateList::setTarget() owner(%s) How did we get a target(%s) that is not a TangibleObject", m_owner->getDebugInformation().c_str(), m_target.get().getObject()->getDebugInformation().c_str()));
		}
	}
#endif // _DEBUG
}

// ======================================================================
