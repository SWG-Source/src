// ======================================================================
//
// ObjectLocator.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/ObjectLocator.h"

#include <vector>

#include "serverDatabase/GameServerConnection.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

ObjectLocator::ObjectLocator()
{
}

// ----------------------------------------------------------------------

ObjectLocator::~ObjectLocator()
{
}

// ----------------------------------------------------------------------

/**
 * Override this function to send any additional non-object data.
 * This is invoked before any of the objects are sent.  For example,
 * this could be used to tell the game server that one of the objects
 * we are loading is a character.
 */
void ObjectLocator::sendPreBaselinesCustomData(GameServerConnection &conn) const
{
	UNREF(conn);
}

// ----------------------------------------------------------------------

/**
 * Override this function to send any additional non-object data.
 * This is invoked after all the objects are sent.  For example,
 * this is used to send the UniverseComplete message to indicate that
 * the Snapshot contained all the universe objects.
 */
void ObjectLocator::sendPostBaselinesCustomData(GameServerConnection &conn) const
{
	UNREF(conn);
}

// ======================================================================
