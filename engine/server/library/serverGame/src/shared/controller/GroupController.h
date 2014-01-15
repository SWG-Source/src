// ======================================================================
//
// GroupController.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_GroupController_H
#define	INCLUDED_GroupController_H

// ======================================================================

#include "serverGame/UniverseController.h"

// ======================================================================

class GroupObject;

// ======================================================================

class GroupController: public UniverseController
{
public:
	explicit GroupController(GroupObject *newOwner);
	~GroupController();

protected:
	virtual void handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags);

private:
	GroupController();
	GroupController(GroupController const &);
	GroupController& operator=(GroupController const &);
};

// ======================================================================

#endif

