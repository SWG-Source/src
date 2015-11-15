// ======================================================================
//
// AiShipBehaviorAttackBomber.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorAttackBomber_H
#define	INCLUDED_AiShipBehaviorAttackBomber_H

#include "sharedFoundation/Timer.h"
#include "serverGame/AiShipBehaviorAttack.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedMath/Vector.h"

class AiDebugString;
class AiShipController;
class ShipObject;

// ======================================================================

class AiShipBehaviorAttackBomber : public AiShipBehaviorAttack
{
public:

	explicit AiShipBehaviorAttackBomber(AiShipController & aiShipController);
	explicit AiShipBehaviorAttackBomber(AiShipBehaviorAttack const & sourceBehavior);
	virtual ~AiShipBehaviorAttackBomber();

	virtual void alter(float time);
	void addDebug(AiDebugString & aiDebugString);

  private:
	enum AttackStage {AS_attack, AS_evade};

  private:
	void selectTargetComponent();
	void doEvade(float const deltaTime);
	void doAttackRun(float const deltaTime);
	ShipObject const * getTargetCapitalShip() const;
	void changeTargetObject(NetworkId const & newTarget);
	void changeTargetComponentSlot(ShipChassisSlotType::Type newTarget);

  private:
	AttackStage m_attackStage;
	NetworkId m_targetId;
	ShipChassisSlotType::Type m_targetComponentSlot;
	Vector m_targetComponentPosition_o; // in the space of the target ship
	Vector m_evadePosition_o; // in the space of the target ship
	float m_evadeDistanceSquared;
	Vector m_evadeOffset;
	Timer m_evadeChangeDirectionTimer;
	int m_missilesFired;
	
  private:
	// Disabled
	AiShipBehaviorAttackBomber();
	AiShipBehaviorAttackBomber(AiShipBehaviorAttackBomber const &);
	AiShipBehaviorAttackBomber &operator =(AiShipBehaviorAttackBomber const &);
};

// ======================================================================

#endif //INCLUDED_AiShipBehaviorAttackBomber_H
