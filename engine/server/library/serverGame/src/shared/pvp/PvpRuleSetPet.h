// ======================================================================
//
// PvpRuleSetPet.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpRuleSetPet_H
#define _PvpRuleSetPet_H

// ======================================================================

#include "serverGame/PvpRuleSetNormal.h"

// ======================================================================

class PvpRuleSetPet: public PvpRuleSetNormal
{
public:
	virtual bool canAttack(TangibleObject const &actor, TangibleObject const &target) const;
	virtual bool canHelp(TangibleObject const &actor, TangibleObject const &target) const;
	virtual void getAttackRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const;
	virtual void getHelpRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const;
	virtual bool isEnemy(TangibleObject const &actor, TangibleObject const &target) const;
	virtual void applyRepercussions(TangibleObject &actor, TangibleObject &target, std::vector<PvpEnemy> const &actorRepercussions, std::vector<PvpEnemy> const &targetRepercussions);

private:
	TangibleObject const *resolveMaster(TangibleObject const &who) const;
	TangibleObject *resolveMaster(TangibleObject &who) const;
};

// ======================================================================

#endif // _PvpRuleSetPet_H

