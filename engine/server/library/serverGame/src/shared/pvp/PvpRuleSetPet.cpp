// ======================================================================
//
// PvpRuleSetPet.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpRuleSetPet.h"
#include "serverGame/PvpInternal.h"
#include "serverUtility/PvpEnemy.h"

// ======================================================================

bool PvpRuleSetPet::canAttack(TangibleObject const &actor, TangibleObject const &target) const
{
	TangibleObject const *realActor = resolveMaster(actor);
	TangibleObject const *realTarget = resolveMaster(target);
	if (!realActor || !realTarget)
		return false;
	return PvpInternal::getRuleSet(*realActor, *realTarget).canAttack(*realActor, *realTarget);
}

// ----------------------------------------------------------------------

bool PvpRuleSetPet::canHelp(TangibleObject const &actor, TangibleObject const &target) const
{
	TangibleObject const *realActor = resolveMaster(actor);
	TangibleObject const *realTarget = resolveMaster(target);
	if (!realActor || !realTarget)
		return false;
	return PvpInternal::getRuleSet(*realActor, *realTarget).canHelp(*realActor, *realTarget);
}

// ----------------------------------------------------------------------

void PvpRuleSetPet::getAttackRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const
{
	actorRepercussions.clear();
	targetRepercussions.clear();
	TangibleObject const *realActor = resolveMaster(actor);
	TangibleObject const *realTarget = resolveMaster(target);
	if (realActor && realTarget)
		PvpInternal::getRuleSet(*realActor, *realTarget).getAttackRepercussions(*realActor, *realTarget, actorRepercussions, targetRepercussions);
}

// ----------------------------------------------------------------------

void PvpRuleSetPet::getHelpRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const
{
	actorRepercussions.clear();
	targetRepercussions.clear();
	TangibleObject const *realActor = resolveMaster(actor);
	TangibleObject const *realTarget = resolveMaster(target);
	if (realActor && realTarget)
		PvpInternal::getRuleSet(*realActor, *realTarget).getHelpRepercussions(*realActor, *realTarget, actorRepercussions, targetRepercussions);
}

// ----------------------------------------------------------------------

void PvpRuleSetPet::applyRepercussions(TangibleObject &actor, TangibleObject &target, std::vector<PvpEnemy> const &actorRepercussions, std::vector<PvpEnemy> const &targetRepercussions)
{
	TangibleObject *realActor = resolveMaster(actor);
	TangibleObject *realTarget = resolveMaster(target);
	if (realActor && realTarget)
		PvpInternal::getRuleSet(*realActor, *realTarget).applyRepercussions(*realActor, *realTarget, actorRepercussions, targetRepercussions);
}

// ----------------------------------------------------------------------

bool PvpRuleSetPet::isEnemy(TangibleObject const &actor, TangibleObject const &target) const
{
	TangibleObject const *realActor = resolveMaster(actor);
	TangibleObject const *realTarget = resolveMaster(target);
	if (!realActor || !realTarget)
		return false;
	return PvpInternal::getRuleSet(*realActor, *realTarget).isEnemy(*realActor, *realTarget);
}

// ----------------------------------------------------------------------

TangibleObject const *PvpRuleSetPet::resolveMaster(TangibleObject const &who) const
{
	if (!PvpInternal::isPet(who))
		return &who;
	TangibleObject const *master = PvpInternal::getPetMaster(who);
	if (master != &who)
		return master;
	return 0;
}

// ----------------------------------------------------------------------

TangibleObject *PvpRuleSetPet::resolveMaster(TangibleObject &who) const
{
	if (!PvpInternal::isPet(who))
		return &who;
	TangibleObject *master = PvpInternal::getPetMaster(who);
	if (master != &who)
		return master;
	return 0;
}

// ======================================================================

