// ======================================================================
//
// AiCreatureCombatProfile.cpp
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiCreatureCombatProfile.h"

#include "serverGame/AiCreatureCombatProfile_Action.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedLog/Log.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"

#include <map>
#include <set>

// ======================================================================
//
// AiCreatureCombatProfileNamespace
//
// ======================================================================

namespace AiCreatureCombatProfileNamespace
{
	bool s_installed = false;
	char const * const s_combatProfileTable = "datatables/ai/ai_combat_profiles.iff";

	typedef std::set<PersistentCrcString> CachedActionNameList;
	typedef std::map<PersistentCrcString, AiCreatureCombatProfile> CombatProfileMap;

	CachedActionNameList s_cachedActionNameList;
	CombatProfileMap s_combatProfileMap;
	int const s_actionCount = 14;

	void remove();
	void clearData();
	void loadCombatProfileTable(DataTable const & dataTable);
	std::pair<CachedActionNameList::iterator, CachedActionNameList::iterator> cacheActionName(std::string const & action);
	void loadCombatProfile();
	void grantActions(CreatureObject & owner, AiCreatureCombatProfile::ActionList const & actionList);
}

using namespace AiCreatureCombatProfileNamespace;

// ----------------------------------------------------------------------
void AiCreatureCombatProfileNamespace::remove()
{
	clearData();
}

// ----------------------------------------------------------------------
void AiCreatureCombatProfileNamespace::clearData()
{
	s_cachedActionNameList.clear();

	CombatProfileMap::iterator iterCombatProfileMap = s_combatProfileMap.begin();

	for (; iterCombatProfileMap != s_combatProfileMap.end(); ++iterCombatProfileMap)
	{
		{
			AiCreatureCombatProfile::ActionList::iterator iterActionList = iterCombatProfileMap->second.m_singleUseActionList.begin();

			for (; iterActionList != iterCombatProfileMap->second.m_singleUseActionList.end(); ++iterActionList)
			{
				delete *iterActionList;
			}
		}

		{
			AiCreatureCombatProfile::ActionList::iterator iterActionList = iterCombatProfileMap->second.m_delayRepeatActionList.begin();

			for (; iterActionList != iterCombatProfileMap->second.m_delayRepeatActionList.end(); ++iterActionList)
			{
				delete *iterActionList;
			}
		}

		{
			AiCreatureCombatProfile::ActionList::iterator iterActionList = iterCombatProfileMap->second.m_instantRepeatActionList.begin();

			for (; iterActionList != iterCombatProfileMap->second.m_instantRepeatActionList.end(); ++iterActionList)
			{
				delete *iterActionList;
			}
		}
	}

	s_combatProfileMap.clear();
}

// ----------------------------------------------------------------------
std::pair<CachedActionNameList::iterator, CachedActionNameList::iterator> AiCreatureCombatProfileNamespace::cacheActionName(std::string const & action)
{
	std::pair<CachedActionNameList::iterator, CachedActionNameList::iterator> result;
	std::string const trimmedAction(Unicode::getTrim(action));

	if (!trimmedAction.empty())
	{
		std::string const trimmedBaseAction(trimmedAction.substr(0, trimmedAction.find_last_of('_')));

		result.first = s_cachedActionNameList.insert(PersistentCrcString(trimmedAction.c_str(), false)).first;
		result.second = s_cachedActionNameList.insert(PersistentCrcString(trimmedBaseAction.c_str(), false)).first;
	}
	else
	{
		result.first = s_cachedActionNameList.end();
		result.second = s_cachedActionNameList.end();
	}

	return result;
}

// ----------------------------------------------------------------------
void AiCreatureCombatProfileNamespace::loadCombatProfileTable(DataTable const & dataTable)
{
	// All AI must be dead when this gets RELOADED
	
	if (s_installed)
	{
		int const serverObjectCount = ServerWorld::getNumObjects();

		for (int i = 0; i < serverObjectCount; ++i)
		{
			ServerObject * const serverObject = ServerWorld::getObject(i);
			CreatureObject * const creatureObject = (serverObject != nullptr) ? serverObject->asCreatureObject() : nullptr;

			if (   (creatureObject != nullptr)
			    && !creatureObject->isPlayerControlled())
			{
				creatureObject->makeDead(NetworkId::cms_invalid, NetworkId::cms_invalid);
			}
		}
	}

	clearData();

	int const rowCount = dataTable.getNumRows();

	for (int row = 0; row < rowCount; ++row)
	{
		PersistentCrcString const profileName(dataTable.getStringValue("profile_id", row), false);
		CombatProfileMap::iterator iterCombatProfileMap = s_combatProfileMap.find(profileName);

		if (iterCombatProfileMap == s_combatProfileMap.end())
		{
			std::pair<CombatProfileMap::iterator, bool> result = s_combatProfileMap.insert(std::make_pair(profileName, AiCreatureCombatProfile()));

			iterCombatProfileMap = result.first;
		}

		// Store the combat profile

		FormattedString<32> fs;
		AiCreatureCombatProfile & combatProfile = iterCombatProfileMap->second;

		combatProfile.m_profileId = &(iterCombatProfileMap->first);

		for (int actionIndex = 1; actionIndex <= s_actionCount; ++actionIndex)
		{
			std::string const actionName(dataTable.getStringValue(fs.sprintf("action%i", actionIndex), row));
			std::pair<CachedActionNameList::iterator, CachedActionNameList::iterator> const result = cacheActionName(actionName);

			if (result.first != s_cachedActionNameList.end())
			{
				PersistentCrcString const & name = *result.first;
				PersistentCrcString const & baseName = *result.second;
				time_t const useTime = static_cast<time_t>(clamp(0, dataTable.getIntValue(fs.sprintf("action%i_use_time", actionIndex), row), 4096));
				float const useChance = static_cast<float>(clamp(0, dataTable.getIntValue(fs.sprintf("action%i_use_chance", actionIndex), row), 100)) / 100.0f;
				WARNING((useChance <= 0.0f), ("AiCreatureCombatProfile::loadCombatProfileTable() profile(%s) contains action%i with 0%% use chance which is invalid", combatProfile.m_profileId->getString(), actionIndex));

				AiCreatureCombatProfile::Action::UseCount const useCount = static_cast<AiCreatureCombatProfile::Action::UseCount>(dataTable.getIntValue(fs.sprintf("action%i_use_count", actionIndex), row));

				// Sort the action into the correct type

				if (useCount == AiCreatureCombatProfile::Action::UC_once)
				{
					combatProfile.m_singleUseActionList.push_back(new AiCreatureCombatProfile::Action(name, baseName, useTime, useChance, useCount));
				}
				else if (useTime > 0)
				{
					combatProfile.m_delayRepeatActionList.push_back(new AiCreatureCombatProfile::Action(name, baseName, useTime, useChance, useCount));
				}
				else
				{
					combatProfile.m_instantRepeatActionList.push_back(new AiCreatureCombatProfile::Action(name, baseName, useTime, useChance, useCount));
				}
			}
		}

		combatProfile.m_knockDownRecoveryTime = static_cast<time_t>(clamp(0, dataTable.getIntValue("knockdown_recovery_time", row), 32));
	}

	LOG("debug_ai", ("AiCreatureCombatProfile::loadCombatProfileTable() Loading...%s - profiles(%u)", dataTable.getName().c_str(), s_combatProfileMap.size()));
}

// ----------------------------------------------------------------------
void AiCreatureCombatProfileNamespace::grantActions(CreatureObject & owner, AiCreatureCombatProfile::ActionList const & actionList)
{
	AiCreatureCombatProfile::ActionList::const_iterator iterActionList = actionList.begin();

	for (; iterActionList != actionList.end(); ++iterActionList)
	{
		AiCreatureCombatProfile::Action const & action = **iterActionList;

		owner.grantCommand(action.m_name.getString(), false);
	}
}

// ======================================================================
//
// AiCreatureCombatProfile
//
// ======================================================================

// ----------------------------------------------------------------------
AiCreatureCombatProfile::AiCreatureCombatProfile()
 : m_profileId(nullptr)
 , m_singleUseActionList()
 , m_delayRepeatActionList()
 , m_instantRepeatActionList()
 , m_knockDownRecoveryTime(4)
{
}

// ----------------------------------------------------------------------
void AiCreatureCombatProfile::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));
	ExitChain::add(AiCreatureCombatProfileNamespace::remove, "AiCreatureCombatProfileNamespace::remove");

	// Load data from ai_combat_profiles.tab
	{
		bool const openIfNotFound = true;
		DataTable * const dataTable = DataTableManager::getTable(s_combatProfileTable, openIfNotFound);

		if (dataTable != nullptr)
		{
#ifdef _DEBUG
			DataTableManager::addReloadCallback(s_combatProfileTable, &loadCombatProfileTable);
#endif // _DEBUG
			loadCombatProfileTable(*dataTable);
			DataTableManager::close(s_combatProfileTable);
		}
		else
		{
			FATAL(true, ("AiCreatureCombatProfile::install() Could not open file %s", s_combatProfileTable));
		}
	}

	s_installed = true;
}

// ----------------------------------------------------------------------
AiCreatureCombatProfile const * AiCreatureCombatProfile::getCombatProfile(CrcString const & profileName)
{
	AiCreatureCombatProfile const * result = nullptr;
	CombatProfileMap::const_iterator iterCombatProfileMap = s_combatProfileMap.find(PersistentCrcString(profileName));

	if (iterCombatProfileMap != s_combatProfileMap.end())
	{
		result = &iterCombatProfileMap->second;
	}

	return result;
}

// ----------------------------------------------------------------------
void AiCreatureCombatProfile::grantActions(CreatureObject & owner) const
{
	AiCreatureCombatProfileNamespace::grantActions(owner, m_singleUseActionList);
	AiCreatureCombatProfileNamespace::grantActions(owner, m_delayRepeatActionList);
	AiCreatureCombatProfileNamespace::grantActions(owner, m_instantRepeatActionList);
}

// ----------------------------------------------------------------------
std::string const AiCreatureCombatProfile::toString() const
{
	std::string result;
	FormattedString<4096> fs;

	result += fs.sprintf("  WeaponProfileId: %s\n", m_profileId->getString());
	result += fs.sprintf("* Single-Use Actions *\n");

	if (m_singleUseActionList.empty())
	{
		result += fs.sprintf(" none\n");
	}
	else
	{
		ActionList::const_iterator iterActionList = m_singleUseActionList.begin();

		for (; iterActionList != m_singleUseActionList.end(); ++iterActionList)
		{
			AiCreatureCombatProfile::Action const * const action = *iterActionList;

			result += fs.sprintf(" - %30s %3.0f%% chance after %2u seconds\n", action->m_name.getString(), action->m_useChance * 100.0f, static_cast<unsigned int>(action->m_useTime));
		}
	}

	result += fs.sprintf("* Delay-Repeat Actions *\n");

	if (m_delayRepeatActionList.empty())
	{
		result += fs.sprintf(" none\n");
	}
	else
	{
		ActionList::const_iterator iterActionList = m_delayRepeatActionList.begin();

		for (; iterActionList != m_delayRepeatActionList.end(); ++iterActionList)
		{
			AiCreatureCombatProfile::Action const * const action = *iterActionList;

			result += fs.sprintf(" - %30s %3.0f%% chance every %2u seconds\n", action->m_name.getString(), action->m_useChance * 100.0f, static_cast<unsigned int>(action->m_useTime));
		}
	}

	result += fs.sprintf("* Instant-Repeat Actions *\n");

	if (m_instantRepeatActionList.empty())
	{
		result += fs.sprintf(" none\n");
	}
	else
	{
		ActionList::const_iterator iterActionList = m_instantRepeatActionList.begin();

		for (; iterActionList != m_instantRepeatActionList.end(); ++iterActionList)
		{
			AiCreatureCombatProfile::Action const * const action = *iterActionList;

			result += fs.sprintf(" - %30s %3.0f%% chance\n", action->m_name.getString(), action->m_useChance * 100.0f);
		}
	}

	return result;
}

// ======================================================================
