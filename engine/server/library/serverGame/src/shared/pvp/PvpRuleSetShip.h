// ======================================================================
//
// PvpRuleSetShip.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpRuleSetShip_H
#define _PvpRuleSetShip_H

// ======================================================================

#include "serverGame/PvpRuleSetBase.h"

// ======================================================================

class PvpRuleSetShip: public PvpRuleSetBase
{
public:
	virtual bool canAttack(TangibleObject const &actor, TangibleObject const &target) const;
	virtual bool isEnemy(TangibleObject const &actor, TangibleObject const &target) const;
	virtual bool isDuelingAllowed(TangibleObject const &actor, TangibleObject const &target) const;
};

// ======================================================================

#endif // _PvpRuleSetShip_H

