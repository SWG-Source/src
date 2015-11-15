// ======================================================================
//
// ClientCombatManagerSupport.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ClientCombatManagerSupport.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <set>

// ======================================================================

namespace ClientCombatManagerSupportNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::set<CrcString const*, LessPointerComparator> ActionNameSet;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_CBTM = TAG(C,B,T,M);
	Tag const TAG_KEY  = TAG3(K,E,Y);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();
	void  load(char const *pathName);
	void  load_0002(Iff &iff);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool           s_installed;
	ActionNameSet  s_actionNameSet;
}

using namespace ClientCombatManagerSupportNamespace;

// ======================================================================
// ClientCombatManagerSupportNamespace
// ======================================================================

void ClientCombatManagerSupportNamespace::remove()
{
	FATAL(!s_installed, ("ClientCombatManagerSupport not installed."));
	s_installed = false;

	std::for_each(s_actionNameSet.begin(), s_actionNameSet.end(), PointerDeleter());

	s_actionNameSet.clear();
}

// ----------------------------------------------------------------------

void ClientCombatManagerSupportNamespace::load(char const *pathName)
{
	Iff iff(pathName);

	iff.enterForm(TAG_CBTM);
	{
		Tag const version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0002:
				load_0002(iff);
				break;

			default:
				{
					char buffer[5];
					ConvertTagToString(version, buffer);
					FATAL(true, ("ClientCombatManagerSupport: unsupported client combat manager file version [%s].", buffer));
				}
		}
	}
	iff.exitForm(TAG_CBTM);
}

// ----------------------------------------------------------------------

void ClientCombatManagerSupportNamespace::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);
	{
		while (!iff.atEndOfForm())
		{
			iff.enterForm(TAG_ENTR);
			{
				iff.enterChunk(TAG_KEY);
				{
					//-- Read the client combat playback manager's combat action name.
					char buffer[1024];
					iff.read_string(buffer, sizeof(buffer) - 1);
					buffer[sizeof(buffer) - 1] = '\0';

					//-- Add the key to the set of combat action names.
					CrcString *const newKey = new PersistentCrcString(buffer, true);
					std::pair<ActionNameSet::iterator, bool> insertResult = s_actionNameSet.insert(newKey);
					if (!insertResult.second)
					{
						// Must be a duplicate.
						WARNING(true, ("ClientCombatManagerSupport: the combat manager iff has multiple entries for combat action name [%s], please fix so that only one entry appears for each combat action name key.", newKey->getString()));
						delete newKey;
					}
				}
				iff.exitChunk(TAG_KEY);
			}
			iff.exitForm(TAG_ENTR, true);
		}
	}
	iff.exitForm(TAG_0002);
}

// ======================================================================
// ClientCombatManagerSupport PUBLIC STATIC
// ======================================================================

void ClientCombatManagerSupport::install(char const *clientCombatManagerPath)
{
	InstallTimer const installTimer("ClientCombatManagerSupport::install");

	FATAL(s_installed, ("ClientCombatManagerSupport already installed."));

	load(clientCombatManagerPath);

	s_installed = true;
	ExitChain::add(ClientCombatManagerSupportNamespace::remove, "ClientCombatManagerSupport");
}

// ----------------------------------------------------------------------

bool ClientCombatManagerSupport::doesCombatActionNameExist(CrcString const &combatActionName)
{
	FATAL(!s_installed, ("ClientCombatManagerSupport not installed."));
	return (s_actionNameSet.find(&combatActionName) != s_actionNameSet.end());
}

// ======================================================================
