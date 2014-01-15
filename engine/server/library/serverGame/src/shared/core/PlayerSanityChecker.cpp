// ======================================================================
//
// PlayerSanityChecker.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlayerSanityChecker.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/LogoutTracker.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace PlayerSanityCheckerNamespace
{
	bool s_initialized = false;
	bool s_enabled = true;
	
	struct SanityCheckInfo
	{
		SanityCheckInfo() :
			proxyCheckFailCount(0),
			clientCheckFailCount(0)
		{
		}
		int proxyCheckFailCount;
		int clientCheckFailCount;
	};

	const float MIN_SANITY_CHECK_LOOP_TIME = 15.0f;
	const int MAX_CLIENT_CHECK_FAILS = 2;
	unsigned int s_checkPos = 0;
	std::vector<std::pair<NetworkId, SanityCheckInfo> > s_checkVec;
	std::map<NetworkId, int> s_checkMap;
	float s_loopTime = 0.0f;

}
using namespace PlayerSanityCheckerNamespace;

// ======================================================================

void PlayerSanityChecker::add(NetworkId const &id)
{
	if (s_checkMap.find(id) == s_checkMap.end())
	{
		s_checkVec.push_back(std::make_pair(id, SanityCheckInfo()));
		s_checkMap[id] = s_checkVec.size()-1;
	}
}

// ----------------------------------------------------------------------

void PlayerSanityChecker::update(float time)
{
	if (!s_initialized)
	{
		s_enabled = !ConfigServerGame::getDisablePlayerSanityChecker();
		s_initialized = true;
	}
	
	if (s_enabled)
	{
		static const int maxProxyCheckFailures = ConfigServerGame::getMaxPlayerSanityCheckFailures();

		s_loopTime += time;
		++s_checkPos;
		if (s_checkPos >= s_checkVec.size())
		{
			if (s_loopTime < MIN_SANITY_CHECK_LOOP_TIME)
				return;
			s_checkPos = 0;
			s_loopTime = 0;
			// pull any cleared entries from the vector and rebuild the map
			s_checkMap.clear();
			int pos = 0;
			std::vector<std::pair<NetworkId, SanityCheckInfo> >::iterator i = s_checkVec.begin();
			while (i != s_checkVec.end())
			{
				if ((*i).first == NetworkId::cms_invalid)
					i = s_checkVec.erase(i);
				else
				{
					s_checkMap[(*i).first] = pos;
					++pos;
					++i;
				}
			}
		}

		if (s_checkVec.empty())
			return;

		std::pair<NetworkId, SanityCheckInfo> &entry = s_checkVec[s_checkPos];

		ServerObject * const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(entry.first));
		if (!obj)
		{
			s_checkMap.erase(entry.first);
			entry = std::make_pair(NetworkId::cms_invalid, SanityCheckInfo());
		}
		else if (obj->isAuthoritative())
		{
			if (obj->isInWorld()) // Players waiting for their data to be saved are not in the world.  We shouldn't check proxies for these.  (They won't have any.)
			{
				if (!obj->getClient())
				{
					++(entry.second.clientCheckFailCount);
					if (entry.second.clientCheckFailCount >= MAX_CLIENT_CHECK_FAILS)
					{
						entry.second.clientCheckFailCount = 0;
						PlayerObject *playerObject = PlayerCreatureController::getPlayerObject(obj->asCreatureObject());
						if (playerObject)
							playerObject->setLinkDead(true);
						if (!LogoutTracker::isInLogoutCallbackList(entry.first))
							obj->handleDisconnect(false);
					}
				}
				if (maxProxyCheckFailures)
				{
					entry.second.proxyCheckFailCount = 0;

					ProxyList const &proxyList = obj->getExposedProxyList();
					if (!proxyList.empty())
					{
						ServerMessageForwarding::begin(std::vector<uint32>(proxyList.begin(), proxyList.end()));

						GenericValueTypeMessage<std::pair<NetworkId, uint32> > const proxyCheckMessage(
							"PlayerSanityCheckProxy",
							std::make_pair(entry.first, GameServer::getInstance().getProcessId()));
						ServerMessageForwarding::send(proxyCheckMessage);

						ServerMessageForwarding::end();
					}
				}
			}
		}
		else
		{
			entry.second.clientCheckFailCount = 0;
			if (maxProxyCheckFailures)
			{
				//Only send sanity check if we are not in the middle of a transfer
				//and hence we aren't authoritative even though this is temporarily the auth process
				if (obj->getAuthServerProcessId() != GameServer::getInstance().getProcessId())
				{
					++(entry.second.proxyCheckFailCount);
					if (entry.second.proxyCheckFailCount <= maxProxyCheckFailures)
					{
						ServerMessageForwarding::begin(obj->getAuthServerProcessId());

						GenericValueTypeMessage<std::pair<NetworkId, uint32> > const checkMessage(
							"PlayerSanityCheck",
							std::make_pair(entry.first, GameServer::getInstance().getProcessId()));
						ServerMessageForwarding::send(checkMessage);

						ServerMessageForwarding::end();
					}
					else
					{
						s_checkMap.erase(entry.first);
						entry = std::make_pair(NetworkId::cms_invalid, SanityCheckInfo());
						WARNING(true, ("PlayerSanityCheck: Unloading proxy for %s due to exceeding maximum number of sanity check failures.", obj->getNetworkId().getValueString().c_str()));
						obj->unload();
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void PlayerSanityChecker::handlePlayerSanityCheck(NetworkId const &id, uint32 fromServerId)
{
	ServerObject * const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(id));
	if (obj && obj->isAuthoritative())
	{
		ProxyList const &proxyList = obj->getExposedProxyList();
		if (proxyList.find(fromServerId) != proxyList.end())
		{
			ServerMessageForwarding::begin(fromServerId);

			GenericValueTypeMessage<NetworkId> const successMessage("PlayerSanityCheckSuccess", id);
			ServerMessageForwarding::send(successMessage);

			ServerMessageForwarding::end();
		}
	}
}

// ----------------------------------------------------------------------

void PlayerSanityChecker::handlePlayerSanityCheckSuccess(NetworkId const &id)
{
	std::map<NetworkId, int>::iterator i = s_checkMap.find(id);
	if (i != s_checkMap.end())
		s_checkVec[(*i).second].second.proxyCheckFailCount = 0;
}

// ----------------------------------------------------------------------

void PlayerSanityChecker::handlePlayerSanityCheckProxy(NetworkId const &id, uint32 fromServerId)
{
	// The auth server is asking us if we have a proxy that should be here.  We only respond on failure.
	ServerObject * const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(id));
	if (!obj || obj->isAuthoritative())
	{
		ServerMessageForwarding::begin(fromServerId);

		GenericValueTypeMessage<std::pair<NetworkId, uint32> > const failMessage(
			"PlayerSanityCheckProxyFail",
			std::make_pair(id, GameServer::getInstance().getProcessId()));
		ServerMessageForwarding::send(failMessage);

		ServerMessageForwarding::end();

		if (obj)
		{
			// Erm, bad.  We have 2 auth objects... unload and hope something ends up ok.
			obj->unload();
			WARNING(true, ("PlayerSanityCheck: Found an auth object %s where we should have been a proxy.", id.getValueString().c_str()));
		}
		else
		{
			WARNING(true, ("PlayerSanityCheck: Failed to find proxy which should exist for object %s.", id.getValueString().c_str()));
		}
	}
}

// ----------------------------------------------------------------------

void PlayerSanityChecker::handlePlayerSanityCheckProxyFail(NetworkId const &id, uint32 fromServerId)
{
	ServerObject *obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(id));
	if (obj && obj->isAuthoritative() && obj->isInWorld())
	{
		ProxyList const &proxyList = obj->getExposedProxyList();
		if (proxyList.find(fromServerId) != proxyList.end())
		{
			WARNING(true, ("PlayerSanityCheck: Sending baselines to server in proxy list that has no proxy for %s.", id.getValueString().c_str()));

			ServerMessageForwarding::begin(fromServerId);

			obj->forwardServerCreateAndBaselines();

			ServerMessageForwarding::end();
		}
	}
}

// ----------------------------------------------------------------------

void PlayerSanityChecker::enable(bool flag)
{
	s_enabled=flag;
}

// ======================================================================

