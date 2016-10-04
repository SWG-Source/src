// ======================================================================
//
// AuthTransferTracker.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AuthTransferTracker.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerMessageForwarding.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include <algorithm>
#include <queue>
#include <cstdio>

// ======================================================================

struct AuthTransferInfo
{
	uint32 newAuthProcessId;
	std::vector<uint32> unconfirmedProcessIds;
	unsigned long startTime;
};

static std::map<NetworkId, AuthTransferInfo> *s_authTransferMap;
static std::queue<NetworkId> *s_authTransfersOrdered;

const unsigned long AUTH_TRACK_TIME_MS = 15000;

// ======================================================================

void AuthTransferTracker::install()
{
	DEBUG_FATAL(s_authTransferMap, ("AuthTransferTracker already installed"));
	s_authTransferMap = new std::map<NetworkId, AuthTransferInfo>;
	s_authTransfersOrdered = new std::queue<NetworkId>;
}

// ----------------------------------------------------------------------

void AuthTransferTracker::remove()
{
	DEBUG_FATAL(!s_authTransferMap, ("AuthTransferTracker not installed"));
	delete s_authTransferMap;
	s_authTransferMap = 0;
	delete s_authTransfersOrdered;
	s_authTransfersOrdered = 0;
}

// ----------------------------------------------------------------------

void AuthTransferTracker::beginAuthTransfer(NetworkId const &networkId, uint32 newAuthProcessId, ProxyList const &proxyList)
{
	// sanity check
	{
		std::map<NetworkId, AuthTransferInfo>::iterator i = s_authTransferMap->find(networkId);
		if (i != s_authTransferMap->end())
		{
			WARNING_STRICT_FATAL(true, ("Tried to transfer authority for %s to %lu with an authority transfer to %lu still outstanding.  Some controller messages could be lost.", networkId.getValueString().c_str(), newAuthProcessId, (*i).second.newAuthProcessId));
			s_authTransferMap->erase(i);
			return;
		}
	}

	// We only need to track auth transfers if there are proxies that messages could be coming from
	bool trackAuthTransfer = false;
	{
		for (ProxyList::const_iterator i = proxyList.begin(); i != proxyList.end(); ++i)
			if ((*i) != GameServer::getInstance().getProcessId())
				trackAuthTransfer = true;
	}

	if (trackAuthTransfer)
	{
		AuthTransferInfo &authTransferInfo = (*s_authTransferMap)[networkId];
		authTransferInfo.newAuthProcessId = newAuthProcessId;
		authTransferInfo.startTime = Clock::timeMs();
		for (ProxyList::const_iterator i = proxyList.begin(); i != proxyList.end(); ++i)
			if ((*i) != GameServer::getInstance().getProcessId())
				authTransferInfo.unconfirmedProcessIds.push_back(*i);
		s_authTransfersOrdered->push(networkId);

		if (ConfigServerGame::getLogAuthTransfer())
		{
			char logBuffer[1024];
			logBuffer[0] = '\0';
			for (std::vector<uint32>::const_iterator i = authTransferInfo.unconfirmedProcessIds.begin(); i != authTransferInfo.unconfirmedProcessIds.end(); ++i)
				snprintf(logBuffer + strlen(logBuffer), sizeof(logBuffer), "%lu ", *i);
			LOG("AuthTransfer", ("Begin auth transfer confirm for %s ( %s)", networkId.getValueString().c_str(), logBuffer));
		}
	}
}

// ----------------------------------------------------------------------

void AuthTransferTracker::sendConfirmAuthTransfer(NetworkId const &networkId, uint32 fromServer)
{
	// only send if the old server was set, wasn't this server, and wasn't the db.
	if (fromServer
		&& fromServer != GameServer::getInstance().getProcessId()
		&& fromServer != GameServer::getInstance().getDatabaseProcessId())
	{
		// also only send if we have a connection, since it may be an auth transfer due to a server crash
		if (GameServer::getInstance().isGameServerConnected(fromServer))
		{
			ServerMessageForwarding::begin(fromServer);

			GenericValueTypeMessage<std::pair<NetworkId, uint32> > const confirmMessage(
				"AuthTransferConfirmMessage",
				std::make_pair(networkId, GameServer::getInstance().getProcessId()));
			ServerMessageForwarding::send(confirmMessage);

			ServerMessageForwarding::end();
		}
	}
}

// ----------------------------------------------------------------------

void AuthTransferTracker::handleConfirmAuthTransfer(NetworkId const &networkId, uint32 whichServer)
{
	std::map<NetworkId, AuthTransferInfo>::iterator i = s_authTransferMap->find(networkId);

	if (i != s_authTransferMap->end())
	{
		AuthTransferInfo &authTransferInfo = (*i).second;
		std::vector<uint32>::iterator j = std::find(authTransferInfo.unconfirmedProcessIds.begin(), authTransferInfo.unconfirmedProcessIds.end(), whichServer);
		if (j != authTransferInfo.unconfirmedProcessIds.end())
		{
			if (ConfigServerGame::getLogAuthTransfer())
				LOG("AuthTransfer", ("Confirm for auth transfer of %s from %lu", networkId.getValueString().c_str(), whichServer));

			if (authTransferInfo.unconfirmedProcessIds.size() == 1)
			{
				s_authTransferMap->erase(i);
				if (ConfigServerGame::getLogAuthTransfer())
					LOG("AuthTransfer", ("Auth transfer confirm for %s complete", networkId.getValueString().c_str()));
			}
			else
			{
				authTransferInfo.unconfirmedProcessIds.erase(j);
			}
			return;
		}
	}

	if (ConfigServerGame::getLogAuthTransfer())
		LOG("AuthTransfer", ("unexpected confirm for auth transfer of %s from %lu", networkId.getValueString().c_str(), whichServer));
}

// ----------------------------------------------------------------------

uint32 AuthTransferTracker::getAuthTransferDest(NetworkId const &networkId)
{
	std::map<NetworkId, AuthTransferInfo>::const_iterator i = s_authTransferMap->find(networkId);
	if (i != s_authTransferMap->end())
		return (*i).second.newAuthProcessId;
	return 0;
}

// ----------------------------------------------------------------------

void AuthTransferTracker::handleGameServerDisconnect(uint32 processId)
{
	if (s_authTransferMap)
	{
		std::map<NetworkId, AuthTransferInfo>::iterator i = s_authTransferMap->begin();
		while (i != s_authTransferMap->end())
		{
			AuthTransferInfo &authTransferInfo = (*i).second;
			std::vector<uint32>::iterator j = std::find(authTransferInfo.unconfirmedProcessIds.begin(), authTransferInfo.unconfirmedProcessIds.end(), processId);
			if (j != authTransferInfo.unconfirmedProcessIds.end())
				authTransferInfo.unconfirmedProcessIds.erase(j);
			if (authTransferInfo.newAuthProcessId == processId || authTransferInfo.unconfirmedProcessIds.empty())
				s_authTransferMap->erase(i++);
			else
				++i;
		}
	}
}

// ----------------------------------------------------------------------

void AuthTransferTracker::update()
{
	if (!s_authTransfersOrdered->empty())
	{
		NetworkId const &networkId = s_authTransfersOrdered->front();
		std::map<NetworkId, AuthTransferInfo>::iterator i = s_authTransferMap->find(networkId);
		if (i != s_authTransferMap->end())
		{
			if (Clock::timeMs() - (*i).second.startTime < AUTH_TRACK_TIME_MS)
				return;
			s_authTransferMap->erase(i);
		}
		s_authTransfersOrdered->pop();
	}
}

// ======================================================================