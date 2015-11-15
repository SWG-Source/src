// ======================================================================
//
// TeleportFixupHandler.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/TeleportFixupHandler.h"
#include "serverGame/ServerObject.h"
#include "sharedLog/Log.h"
#include <list>

// ======================================================================

const float TELEPORT_FIXUP_TIMEOUT_SEC = 20.0f;
static std::list<std::pair<Watcher<ServerObject>, float> > s_teleportFixups;

// ======================================================================

void TeleportFixupHandler::add(ServerObject &obj)
{
	s_teleportFixups.push_back(std::make_pair(Watcher<ServerObject>(&obj), TELEPORT_FIXUP_TIMEOUT_SEC));
	LOG("TeleportFixup", ("Adding %s to teleport fixup handler\n", obj.getNetworkId().getValueString().c_str()));
}

// ----------------------------------------------------------------------

void TeleportFixupHandler::update(float time)
{
	// Run through the list, trying to fix up any outstanding teleports and counting down the timers
	// When we try the teleport fixup, we force it if we've timed out, and we remove fixups from the
	// list when either the object cannot be found, is no longer authoritative, or succeeds at the
	// fixup (which it always will when forced).
	std::list<std::pair<Watcher<ServerObject>, float> >::iterator i = s_teleportFixups.begin();
	while (i != s_teleportFixups.end())
	{
		ServerObject *obj = (*i).first.getPointer();
		(*i).second -= time;
		if (!obj || !obj->isAuthoritative() || obj->handleTeleportFixup((*i).second <= 0.0f))
		{
			IGNORE_RETURN(s_teleportFixups.erase(i++));
			if (obj)
				LOG("TeleportFixup", ("Removing %s from teleport fixup handler, was forced: %s\n", obj->getNetworkId().getValueString().c_str(), ((*i).second <= 0.0f) ? "true" : "false"));
		}
		else
			++i;
	}
}

// ======================================================================

