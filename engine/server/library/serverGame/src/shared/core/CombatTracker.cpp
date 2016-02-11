// ======================================================================
//
// CombatTracker.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CombatTracker.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TangibleObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedObject/CellProperty.h"

// ======================================================================
static std::set<TangibleObject*> *s_defenders;

// ======================================================================

void CombatTracker::install()
{
	FATAL(s_defenders, ("CombatTracker::install - already installed"));
	s_defenders = new std::set<TangibleObject*>;
}

// ----------------------------------------------------------------------

void CombatTracker::remove()
{
	FATAL(!s_defenders, ("CombatTracker::remove - not installed"));
	delete s_defenders;
	s_defenders = 0;
}

// ----------------------------------------------------------------------

void CombatTracker::getInterestedViewers(TangibleObject const *defender, std::vector<ServerObject *> &combatViewers)
{
	std::vector<ServerObject *> unfilteredViewers;
	AICreatureController const * const defenderAiCreatureController = AICreatureController::asAiCreatureController(defender->getController());
	bool const defenderIsInWorldCell = defender->isInWorldCell();

	if (defenderAiCreatureController != nullptr)
	{
		ServerWorld::findObjectsInRange(defender->getPosition_w(), defenderAiCreatureController->getAssistRadius(), unfilteredViewers);

		for (std::vector<ServerObject *>::iterator i = unfilteredViewers.begin(); i != unfilteredViewers.end(); ++i)
		{
			bool interested = true;
			ServerObject const * const serverObject = *i;

			if (   (serverObject == nullptr)
				|| (serverObject == defender)
				|| serverObject->isPlayerControlled()
				|| !serverObject->wantSawAttackTriggers())
			{
				interested = false;
			}
			else
			{
				TangibleObject const * const tangibleObject = serverObject->asTangibleObject();

				if (tangibleObject != nullptr)
				{
					if (   tangibleObject->isInCombat()
						|| tangibleObject->isDisabled()
						|| tangibleObject->isInvulnerable())
					{
						interested = false;
					}
					else
					{
						CreatureObject const * const creatureObject = tangibleObject->asCreatureObject();

						if (creatureObject != nullptr)
						{
							if (   creatureObject->isIncapacitated()
								|| creatureObject->isDead())
							{
								interested = false;
							}
							else
							{	
								AICreatureController const * const aiCreatureController = AICreatureController::asAiCreatureController(creatureObject->getController());

								if (aiCreatureController != nullptr)
								{
									if (aiCreatureController->isRetreating())
									{
										interested = false;
									}
									else if (!defenderIsInWorldCell && !creatureObject->isInWorldCell()
										&& !CellProperty::areAdjacent(defender->getParentCell(), creatureObject->getParentCell()))
									{
										interested = false;
									}
								}
							}
						}
					}
				}
			}

			if (interested)
			{
				combatViewers.push_back(*i);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CombatTracker::update()
{
	std::set<TangibleObject*>::const_iterator iterDefenders = s_defenders->begin();

	for (; iterDefenders != s_defenders->end(); ++iterDefenders)
	{
		// get defender and attackers

		TangibleObject * const defender = *iterDefenders;
		HateList::RecentList const & attackers = defender->getRecentHateList();

		if (!attackers.empty())
		{
			// get attackers with proxies on this server

			std::vector<NetworkId> localAttackers;
			HateList::RecentList::const_iterator iterAttackers = attackers.begin();

			for (; iterAttackers != attackers.end(); ++iterAttackers)
			{
				TangibleObject const * const attackerTangibleObject = TangibleObject::asTangibleObject(iterAttackers->getObject());

				if (attackerTangibleObject != nullptr)
				{
					localAttackers.push_back(attackerTangibleObject->getNetworkId());
				}
			}

			if (!localAttackers.empty())
			{
				// get interested combat viewers

				std::vector<ServerObject *> combatViewers;
				getInterestedViewers(defender, combatViewers);

				std::vector<ServerObject *>::const_iterator iterCombatViewers = combatViewers.begin();

				for (; iterCombatViewers != combatViewers.end(); ++iterCombatViewers)
				{
					ServerObject * const combatViewer = *iterCombatViewers;
					GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(combatViewer);

					if (gameScriptObject != nullptr)
					{
						ScriptParams scriptParams;
						scriptParams.addParam(defender->getNetworkId());
						scriptParams.addParam(localAttackers);
						IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SAW_ATTACK, scriptParams));
					}
				}
			}

			defender->clearRecentHateList();
		}
	}

	s_defenders->clear();
}

// ----------------------------------------------------------------------

void CombatTracker::addDefender(TangibleObject *defender)
{
	if (s_defenders)
	{
		IGNORE_RETURN( s_defenders->insert(defender) );
	}
}

// ----------------------------------------------------------------------

void CombatTracker::removeDefender(TangibleObject *defender)
{
	if (s_defenders)
	{
		IGNORE_RETURN( s_defenders->erase(defender) );
	}
}

// ======================================================================
