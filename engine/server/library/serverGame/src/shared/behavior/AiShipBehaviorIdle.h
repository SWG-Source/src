// ======================================================================
//
// AiShipBehaviorIdle.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorIdle_H
#define	INCLUDED_AiShipBehaviorIdle_H

#include "serverGame/AiShipBehaviorBase.h"
class AiShipController;

// ======================================================================

class AiShipBehaviorIdle : public AiShipBehaviorBase
{
public:

	explicit AiShipBehaviorIdle(AiShipController & aiShipController);

	virtual void alter(float deltaSeconds);
	virtual AiShipBehaviorType getBehaviorType() const;

private:

	// Disabled
	AiShipBehaviorIdle();
	AiShipBehaviorIdle(AiShipBehaviorIdle const &);
	AiShipBehaviorIdle &operator =(AiShipBehaviorIdle const &);
};

// ======================================================================

#endif // INCLUDED_AiShipBehaviorIdle_H
