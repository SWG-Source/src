// ======================================================================
//
// AiShipBehaviorTrack.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorTrack_H
#define	INCLUDED_AiShipBehaviorTrack_H

#include "serverGame/AiShipBehaviorBase.h"
#include "sharedFoundation/Watcher.h"

class AiShipController;
class Object;

// ======================================================================

class AiShipBehaviorTrack : public AiShipBehaviorBase
{
public:

	AiShipBehaviorTrack(AiShipController & aiShipController, Object const & target);
	
	virtual void alter(float deltaSeconds);
	virtual AiShipBehaviorType getBehaviorType() const;

private:

	ConstWatcher<Object> m_target;

	// Disabled
	AiShipBehaviorTrack();
	AiShipBehaviorTrack(AiShipBehaviorTrack const &);
	AiShipBehaviorTrack &operator =(AiShipBehaviorTrack const &);
};

// ======================================================================

#endif // INCLUDED_AiShipBehaviorTrack_H
