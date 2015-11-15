// ======================================================================
//
// PvpRuleSetNormal.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpRuleSetNormal_H
#define _PvpRuleSetNormal_H

// ======================================================================

#include "serverGame/PvpRuleSetBase.h"

// ======================================================================

class PvpRuleSetNormal: public PvpRuleSetBase
{
public:
	virtual bool canAttack(TangibleObject const &actor, TangibleObject const &target) const;
	virtual bool canHelp(TangibleObject const &actor, TangibleObject const &target) const;
	virtual void getAttackRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const;
	virtual void getHelpRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const;
	virtual bool isEnemy(TangibleObject const &actor, TangibleObject const &target) const;
	virtual bool isDuelingAllowed(TangibleObject const &actor, TangibleObject const &target) const;
};

// ======================================================================

#endif // _PvpRuleSetNormal_H

