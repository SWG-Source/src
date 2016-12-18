// ======================================================================
//
// AiLogManager.cpp
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiLogManager.h"

#include "serverGame/ConfigServerGame.h"
#include "sharedFoundation/NetworkId.h"

#include <set>

// ======================================================================
//
// AiLogManagerNamespace
//
// ======================================================================

// ----------------------------------------------------------------------
namespace AiLogManagerNamespace
{
	typedef std::set<NetworkId> AiList;

	AiList s_aiList;
}

using namespace AiLogManagerNamespace;

// ======================================================================
//
// AiLogManager
//
// ======================================================================

// ----------------------------------------------------------------------
void AiLogManager::setLogging(NetworkId const & networkId, bool const enabled)
{
	if (enabled)
	{
		s_aiList.insert(networkId);
	}
	else
	{
		s_aiList.erase(networkId);
	}
}

// ----------------------------------------------------------------------
bool AiLogManager::isLogging(NetworkId const & networkId)
{
	bool result = (ConfigServerGame::isAiLoggingEnabled() && (s_aiList.find(networkId) != s_aiList.end()));
	
	if (!result) {
		s_aiList.erase(networkId);
	}

	return result;
}

// ======================================================================
