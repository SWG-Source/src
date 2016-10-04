// ======================================================================
//
// AiCreatureWeaponActions.cpp
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiCreatureWeaponActions.h"

#include "serverGame/AiCreatureCombatProfile.h"
#include "serverGame/AiCreatureCombatProfile_Action.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedRandom/Random.h"

// ======================================================================
//
// AiCreatureWeaponActionsNamespace
//
// ======================================================================

namespace AiCreatureWeaponActionsNamespace
{
	time_t const s_useChanceRetryTime = 5;

	typedef AiCreatureWeaponActions::Time Time;

	void resetActionTimers(Archive::AutoDeltaVector<Time> & actions, AiCreatureCombatProfile::ActionList const & combatProfileActionList);
}

using namespace AiCreatureWeaponActionsNamespace;

// ----------------------------------------------------------------------
void AiCreatureWeaponActionsNamespace::resetActionTimers(Archive::AutoDeltaVector<Time> & weaponActionList, AiCreatureCombatProfile::ActionList const & combatProfileActionList)
{
	time_t const osTime = Os::getRealSystemTime();

	weaponActionList.clear();

	for (unsigned int i = 0; i < combatProfileActionList.size(); ++i)
	{
		AiCreatureCombatProfile::Action const * const action = combatProfileActionList[i];

		weaponActionList.push_back(Time(osTime + action->m_useTime));
	}
}

// ======================================================================
//
// AiCreatureWeaponActions
//
// ======================================================================

// ----------------------------------------------------------------------
AiCreatureWeaponActions::AiCreatureWeaponActions()
 : m_singleUseActionList()
 , m_delayRepeatActionList()
 , m_instantRepeatActionList()
 , m_combatProfile(nullptr)
{
}

// ----------------------------------------------------------------------
void AiCreatureWeaponActions::addServerNpAutoDeltaVariables(Archive::AutoDeltaByteStream & stream)
{
	stream.addVariable(m_singleUseActionList);
	stream.addVariable(m_delayRepeatActionList);
	stream.addVariable(m_delayRepeatActionList);
}

// ----------------------------------------------------------------------
void AiCreatureWeaponActions::setCombatProfile(CreatureObject & owner, AiCreatureCombatProfile const & combatProfile)
{
	m_combatProfile = &combatProfile;

	m_combatProfile->grantActions(owner);
}

// ----------------------------------------------------------------------
void AiCreatureWeaponActions::reset()
{
	if (m_combatProfile != nullptr)
	{
		resetActionTimers(m_singleUseActionList, m_combatProfile->m_singleUseActionList);
		resetActionTimers(m_delayRepeatActionList, m_combatProfile->m_delayRepeatActionList);
		resetActionTimers(m_instantRepeatActionList, m_combatProfile->m_instantRepeatActionList);
	}
}

// ----------------------------------------------------------------------
PersistentCrcString const & AiCreatureWeaponActions::getCombatAction()
{
	// If the combat profile is nullptr, then the AI has no special actions assigned

	if (m_combatProfile != nullptr)
	{
		time_t const osTime = Os::getRealSystemTime();

		// Single use actions
		{
			unsigned int expiredCount = 0;

			for (unsigned int i = 0; i < m_singleUseActionList.size(); ++i)
			{
				if (m_singleUseActionList[i] != 0)
				{
					if (osTime > time_t(m_singleUseActionList[i]))
					{
						m_singleUseActionList.set(i, 0);

						AiCreatureCombatProfile::Action const * const action = m_combatProfile->m_singleUseActionList[i];
						float const diceRoll = Random::randomReal();

						if (action->m_useChance >= diceRoll)
						{
							return action->m_name;
						}
					}
				}
				else
				{
					++expiredCount;
				}
			}

			if (expiredCount >= m_singleUseActionList.size())
			{
				m_singleUseActionList.clear();
			}
		}

		// Delayed repeat actions
		{
			for (unsigned int i = 0; i < m_delayRepeatActionList.size(); ++i)
			{
				if (osTime > time_t(m_delayRepeatActionList[i]))
				{
					AiCreatureCombatProfile::Action const * const action = m_combatProfile->m_delayRepeatActionList[i];
					float const diceRoll = Random::randomReal();

					if (action->m_useChance >= diceRoll)
					{
						m_delayRepeatActionList.set(i, Time(osTime + action->m_useTime));
						return action->m_name;
					}
					else
					{
						// Chance failed, delay the retry

						m_delayRepeatActionList.set(i, Time(osTime + s_useChanceRetryTime));
					}
				}
			}
		}

		// Instant repeat actions
		{
			time_t nextActionTime = 0;
			unsigned int nextActionIndex = 0;

			for (unsigned int i = 0; i < m_instantRepeatActionList.size(); ++i)
			{
				if (osTime >= time_t(m_instantRepeatActionList[i]))
				{
					if (   (nextActionTime == 0)
						|| (time_t(m_instantRepeatActionList[i]) < nextActionTime))
					{
						AiCreatureCombatProfile::Action const * const action = m_combatProfile->m_instantRepeatActionList[i];
						float const diceRoll = Random::randomReal();

						if (action->m_useChance >= diceRoll)
						{
							nextActionIndex = i;
							nextActionTime = m_instantRepeatActionList[i];
						}
						else
						{
							// Chance failed, delay the retry

							m_instantRepeatActionList.set(nextActionIndex, Time(osTime + s_useChanceRetryTime));
						}
					}
				}
			}

			if (nextActionTime > 0)
			{
				AiCreatureCombatProfile::Action const * const action = m_combatProfile->m_instantRepeatActionList[nextActionIndex];

				m_instantRepeatActionList.set(nextActionIndex, Time(osTime + action->m_useTime + 1));

				return action->m_name;
			}
		}
	}

	return PersistentCrcString::empty;
}

// ======================================================================
