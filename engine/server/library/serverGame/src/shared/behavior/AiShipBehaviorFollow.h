// ======================================================================
//
// AiShipBehaviorFollow.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorFollow_H
#define	INCLUDED_AiShipBehaviorFollow_H

#include "serverGame/AiShipBehaviorBase.h"
#include "sharedObject/CachedNetworkId.h"

class AiShipController;
class NetworkId;

// ======================================================================

class AiShipBehaviorFollow : public AiShipBehaviorBase
{
public:

	AiShipBehaviorFollow(AiShipController & aiShipController, NetworkId const & followedUnit);

	virtual void alter(float deltaSeconds);
	virtual AiShipBehaviorType getBehaviorType() const;

private:

	void triggerFollowedUnitLost();

	CachedNetworkId m_followedUnit;
	bool m_followedUnitLost;

	// Disabled
	AiShipBehaviorFollow();
	AiShipBehaviorFollow(AiShipBehaviorFollow const &);
	AiShipBehaviorFollow &operator =(AiShipBehaviorFollow const &);
};

// ======================================================================

#endif // INCLUDED_AiShipBehaviorFollow_H
