// ======================================================================
//
// BattlefieldMarkerController.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_BattlefieldMarkerController_H
#define	_INCLUDED_BattlefieldMarkerController_H

//-----------------------------------------------------------------------

#include "serverGame/TangibleController.h"

//----------------------------------------------------------------------

class BattlefieldMarkerObject;

//-----------------------------------------------------------------------

class BattlefieldMarkerController : public TangibleController
{
public:
	explicit BattlefieldMarkerController(BattlefieldMarkerObject *newOwner);
	virtual ~BattlefieldMarkerController();

protected:
	virtual void handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags);
	
private:
	BattlefieldMarkerController();
	BattlefieldMarkerController(BattlefieldMarkerController const &);
	BattlefieldMarkerController &operator=(BattlefieldMarkerController const &);
};

// ======================================================================

#endif

