// ============================================================================
// 
// ServerBuffBuilderManager.h
// Copyright 2006 Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_ServerBuffBuilderManager_H
#define INCLUDED_ServerBuffBuilderManager_H

//-----------------------------------------------------------------------------

#include "sharedGame/SharedBuffBuilderManager.h"

//-----------------------------------------------------------------------------

class ScriptParams;

//-----------------------------------------------------------------------------

class ServerBuffBuilderManager
{
public:
	static void install();
	static void remove();

	static bool makeChanges(SharedBuffBuilderManager::Session const & session);
	static void sendSessionToScriptForValidation(SharedBuffBuilderManager::Session const & session);
	static void convertSessionToScriptParams(SharedBuffBuilderManager::Session const & session, ScriptParams & /*IN-OUT*/ params);
	static void cancelSession(NetworkId const & bufferId, NetworkId const & buffeeId);

private:
	// Disable
	ServerBuffBuilderManager();
	ServerBuffBuilderManager(ServerBuffBuilderManager const &);
	ServerBuffBuilderManager &operator =(ServerBuffBuilderManager const &);

private:
	static void sendSessionToScript(SharedBuffBuilderManager::Session const & session, NetworkId const & objectToTriggerId, int trigger);
};

// ============================================================================

#endif // INCLUDED_ServerBuffBuilderManager_H
