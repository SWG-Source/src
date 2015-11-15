// ======================================================================
//
// AiShipBehaviorAttackCapitalShip.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorAttackCapitalShip_H
#define	INCLUDED_AiShipBehaviorAttackCapitalShip_H

#include "serverGame/AiShipBehaviorAttack.h"
#include "sharedFoundation/Timer.h"

class NetworkId;

// ======================================================================

class AiShipBehaviorAttackCapitalShip : public AiShipBehaviorAttack
{
  public:

	explicit AiShipBehaviorAttackCapitalShip(AiShipController & aiShipController);
	virtual ~AiShipBehaviorAttackCapitalShip();

	virtual void alter(float deltaSeconds);

  private:
	// Disabled
	AiShipBehaviorAttackCapitalShip();
	AiShipBehaviorAttackCapitalShip(AiShipBehaviorAttackCapitalShip const &);
	AiShipBehaviorAttackCapitalShip &operator =(AiShipBehaviorAttackCapitalShip const &);
};

// ======================================================================

#endif //INCLUDED_AiShipBehaviorAttackCapitalShip_H
