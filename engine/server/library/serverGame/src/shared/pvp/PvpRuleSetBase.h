// ======================================================================
//
// PvpRuleSetBase.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpRuleSetBase_H
#define _PvpRuleSetBase_H

// ======================================================================

class TangibleObject;
struct PvpEnemy;

// ======================================================================

class PvpRuleSetBase
{
public:
	virtual bool canAttack(TangibleObject const &actor, TangibleObject const &target) const;
	virtual bool canHelp(TangibleObject const &actor, TangibleObject const &target) const;
	virtual bool isEnemy(TangibleObject const &actor, TangibleObject const &target) const;
	virtual bool isDuelingAllowed(TangibleObject const &actor, TangibleObject const &target) const;
	virtual void getAttackRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const;
	virtual void getHelpRepercussions(TangibleObject const &actor, TangibleObject const &target, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const;
	virtual void applyRepercussions(TangibleObject &actor, TangibleObject &target, std::vector<PvpEnemy> const &actorRepercussions, std::vector<PvpEnemy> const &targetRepercussions);
};

// ======================================================================

#endif // _PvpRuleSetBase_H

