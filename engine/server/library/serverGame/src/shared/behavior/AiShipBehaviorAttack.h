// ======================================================================
//
// AiShipBehaviorAttack.cpp
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorAttack_H
#define	INCLUDED_AiShipBehaviorAttack_H

#include "serverGame/AiShipBehaviorBase.h"
#include "sharedFoundation/Watcher.h"

class AiDebugString;
class NetworkId;
class Object;
class ShipObject;

// ======================================================================

class AiShipBehaviorAttack : public AiShipBehaviorBase
{
public:

	explicit AiShipBehaviorAttack(AiShipController & aiShipController);
	AiShipBehaviorAttack(AiShipBehaviorAttack const & rhs);
	virtual ~AiShipBehaviorAttack() = 0;

	virtual void alter(float time);

	void setLeashRadius(float const radius);
	float getLeashRadius() const;

	bool isWithinLeashDistance() const;
	float getPercentWithinLeashDistance() const;

#ifdef _DEBUG
	virtual void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG

private:

	float m_leashRadius;
	bool m_damageTaken;

	// Disabled
	AiShipBehaviorAttack();
	AiShipBehaviorAttack &operator =(AiShipBehaviorAttack const &);
};

// ======================================================================

#endif //INCLUDED_AiShipBehaviorAttack_H
