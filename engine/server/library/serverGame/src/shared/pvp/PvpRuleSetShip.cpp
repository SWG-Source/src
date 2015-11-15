// ======================================================================
//
// PvpRuleSetShip.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpRuleSetShip.h"

#include "serverGame/AiShipAttackTargetList.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PvpInternal.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipAiReactionManager.h"
#include "serverGame/ShipController.h"
#include "serverGame/ShipObject.h"
#include <map>

// ======================================================================

bool PvpRuleSetShip::canAttack(TangibleObject const &actor, TangibleObject const &target) const
{
	// no attacking invulnerable targets
	if (target.isInvulnerable())
		return false;

	// Cannot attack yourself
	if (&actor == &target)
		return false;

	ShipObject const * const actorShip = actor.asShipObject();
	ShipObject const * const targetShip = target.asShipObject();

	// Ships may only attack other ships
	if (!actorShip || !targetShip)
		return false;

	if (actorShip->isPlayerShip() && targetShip->isPlayerShip())
	{
		// player vs. player
		// dueling player ships can attack each other
		if (   PvpInternal::hasDuelEnemyFlag(*actorShip, targetShip->getNetworkId())
		    && PvpInternal::hasDuelEnemyFlag(*targetShip, actorShip->getNetworkId()))
			return true;
		// battlefield zones and pvp with both declared fall through to npc rules, otherwise cannot attack
		if (!ServerWorld::isSpaceBattlefieldZoneWithPvp() && (!PvpInternal::isDeclared(*actorShip) || !PvpInternal::isDeclared(*targetShip)))
			return false;
	}

	// if the ships are of the same space faction, they cannot attack each other
	uint32 const actorFaction = actorShip->getSpaceFaction();
	if (actorFaction && actorFaction == targetShip->getSpaceFaction())
		return false;

	// if either is an ally of the other, they cannot attack each other
	return    !ShipAiReactionManager::isAlly(*actorShip, *targetShip)
	       && !ShipAiReactionManager::isAlly(*targetShip, *actorShip);
}

// ----------------------------------------------------------------------

bool PvpRuleSetShip::isEnemy(TangibleObject const &actor, TangibleObject const &target) const
{
	// You are not your own enemy, usually
	if (&actor == &target)
		return false;

	ShipObject const * const actorShip = actor.asShipObject();
	ShipObject const * const targetShip = target.asShipObject();

	// Ships may only attack other ships
	if (!actorShip || !targetShip)
		return false;

	if (actorShip->isPlayerShip() && targetShip->isPlayerShip())
	{
		// consider players enemies if they can attack each other
		return canAttack(actor, target);
	}

	// at least 1 npc is involved, or this is a space battlefield zone, so see if they are enemies
	if (ShipAiReactionManager::isEnemy(*actorShip, *targetShip))
	{
		return true;
	}

	if ((!actorShip->isPlayerShip()) && (targetShip->isPlayerShip()))
	{
		ShipController const * const actorController = safe_cast<ShipController const *>(actorShip->getController());

		if (actorController != 0)
		{
			CachedNetworkId const targetCachedNetworkId(*targetShip);
			AiShipAttackTargetList const & actorAttackList = actorController->getAttackTargetList();
			AiShipAttackTargetList::TargetList::const_iterator ii = actorAttackList.getUnSortedTargetList().find(targetCachedNetworkId);

			if (ii != actorAttackList.getUnSortedTargetList().end())
			{
				return true;
			}
		}
	}
	return false;
}

// ----------------------------------------------------------------------

bool PvpRuleSetShip::isDuelingAllowed(TangibleObject const &actor, TangibleObject const &target) const
{
	ShipObject const *const actorShip = actor.asShipObject();
	ShipObject const *const targetShip = target.asShipObject();

	if (actorShip && targetShip && actorShip->isPlayerShip() && targetShip->isPlayerShip())
	{
		CreatureObject const * actorPilot = actorShip->getPilot();
		CreatureObject const * targetPilot = targetShip->getPilot();
		
		// Don't know whether it is valid to PvP a pilot-less ship, but previous behavior 
		//  allowed it so I will maintain it.
		return (!actorPilot || !actorPilot->isInTutorial()) && (!targetPilot || !targetPilot->isInTutorial());
	}

	return false;
}

// ======================================================================

