// ======================================================================
//
// PvpRuleSetBattlefield.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpRuleSetBattlefield_H
#define _PvpRuleSetBattlefield_H

// ======================================================================

#include "serverGame/PvpRuleSetBase.h"

// ======================================================================

class PvpRuleSetBattlefield: public PvpRuleSetBase
{
public:
	virtual bool canAttack(TangibleObject const &actor, TangibleObject const &target) const;
	virtual bool canHelp(TangibleObject const &actor, TangibleObject const &target) const;
	virtual void getAttackRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const;
	virtual void getHelpRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const;
	virtual bool isEnemy(TangibleObject const &actor, TangibleObject const &target) const;
};

// ======================================================================

#endif // _PvpRuleSetBattlefield_H

