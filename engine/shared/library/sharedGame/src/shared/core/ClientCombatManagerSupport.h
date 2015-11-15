// ======================================================================
//
// ClientCombatManagerSupport.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClientCombatManagerSupport_H
#define INCLUDED_ClientCombatManagerSupport_H

// ======================================================================

class CrcString;

// ======================================================================
/**
 * Provides support for the server to determine if a given combat action name
 * is supported via the client combat playback visualization system.
 *
 * The primary interface is the doesCombatActionNameExist() member function.
 * It returns true if the client visualization system has an entry for the
 * specified combat action name key.
 *
 * This system was added to address the infrequent but hard-to-track issue
 * where server-side script generates a combat action name (via doCombatResults)
 * when the client has no mapping for the action name.  Since the action name
 * comes across as a 32-bit CRC value, we don't even know which name the server
 * is trying to generate.  This class will provide a way for the server to
 * check if a given action name is valid prior to converting to a CRC value.
 *
 * This class runs off of the same data used by the client's 
 * ClientCombatPlaybackManager class in the clientGame library.
 */

class ClientCombatManagerSupport
{
public:

	static void install(char const *clientCombatManagerPath);
	static bool doesCombatActionNameExist(CrcString const &combatActionName);

};

// ======================================================================

#endif
