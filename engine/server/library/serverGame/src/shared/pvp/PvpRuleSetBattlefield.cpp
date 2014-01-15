// ======================================================================
//
// PvpRuleSetBattlefield.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpRuleSetBattlefield.h"
#include "serverGame/PvpFactions.h"
#include "serverGame/PvpInternal.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/Region.h"
#include "serverGame/TangibleObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

bool PvpRuleSetBattlefield::canAttack(TangibleObject const &actor, TangibleObject const &target) const
{
	// you can't attack yourself
	if (&actor == &target)
		return false;

	// non-pvpable and invulnerable objects cannot be attacked
	if (target.isNonPvpObject() || target.isInvulnerable())
		return false;

	Region const *baseRegion = actor.getPvpRegion();
	if (!baseRegion)
		return false;

	RegionPvp const *region = baseRegion->asRegionPvp();
	if (!region)
		return false;

	// non-creatures cannot attack unless they have the pvpCanAttack objvar
	if (!actor.asCreatureObject())
	{
		int pvpCanAttack = 0;
		if (!actor.getObjVars().getItem("pvpCanAttack", pvpCanAttack) || pvpCanAttack != 1)
			return false;
	}

	Pvp::FactionId actorSide = PvpInternal::battlefieldGetFaction(actor, *region);
	Pvp::FactionId targetSide = PvpInternal::battlefieldGetFaction(target, *region);
	if (actorSide != targetSide && actorSide && targetSide)
		return true;
	return false;
}

// ----------------------------------------------------------------------

bool PvpRuleSetBattlefield::canHelp(TangibleObject const &actor, TangibleObject const &target) const
{
	if (&actor == &target)
		return true;

	Region const *baseRegion = actor.getPvpRegion();
	if (!baseRegion)
		return false;

	RegionPvp const *region = baseRegion->asRegionPvp();
	if (!region)
		return false;

	Pvp::FactionId actorSide = PvpInternal::battlefieldGetFaction(actor, *region);
	Pvp::FactionId targetSide = PvpInternal::battlefieldGetFaction(target, *region);
	if (actorSide && actorSide == targetSide)
		return true;
	return false;
}

// ----------------------------------------------------------------------

void PvpRuleSetBattlefield::getAttackRepercussions(TangibleObject const &, TangibleObject const &, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const
{
	actorRepercussions.clear();
	actorRepercussions.push_back(PvpEnemy(NetworkId::cms_invalid, PvpFactions::getBattlefieldFactionId(), -1));

	targetRepercussions.clear();
}

// ----------------------------------------------------------------------

void PvpRuleSetBattlefield::getHelpRepercussions(TangibleObject const &, TangibleObject const &, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const
{
	actorRepercussions.clear();
	actorRepercussions.push_back(PvpEnemy(NetworkId::cms_invalid, PvpFactions::getBattlefieldFactionId(), -1));

	targetRepercussions.clear();
}

// ----------------------------------------------------------------------

bool PvpRuleSetBattlefield::isEnemy(TangibleObject const &actor, TangibleObject const &target) const
{
	return canAttack(actor, target);
}

// ======================================================================

