// ======================================================================
//
// GuildController.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_GuildController_H
#define	INCLUDED_GuildController_H

// ======================================================================

#include "serverGame/UniverseController.h"

// ======================================================================

class GuildObject;

// ======================================================================

class GuildController: public UniverseController
{
public:
	explicit GuildController(GuildObject *newOwner);
	~GuildController();

protected:
	virtual void handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags);

private:
	GuildController();
	GuildController(GuildController const &);
	GuildController& operator=(GuildController const &);
};

// ======================================================================

#endif

