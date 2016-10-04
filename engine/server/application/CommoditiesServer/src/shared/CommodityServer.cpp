// ======================================================================
//
// ConfigCommodityServer.cpp
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
// Server Infrastructure by: Justin Randall
//
// This is the primary entry point to the commodities service. This class
// is called from main.cpp. The main server loop is encapsulated here
// in the run method
//
// ======================================================================

#include "FirstCommodityServer.h"

#include "AuctionMarket.h"
#include "CommodityServer.h"
#include "CommodityServerMetricsData.h"
#include "CentralServerConnection.h"
#include "DatabaseServerConnection.h"
#include "GameServerConnection.h"
#include "ConfigCommodityServer.h"
#include "serverMetrics/MetricsManager.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/Service.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedLog/Log.h"
#include "sharedLog/SetupSharedLog.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include <stdio.h>

//-----------------------------------------------------------------------

namespace CommodityServerNamespace
{
	Service                  * s_gameServerService        = 0; // the listening service waiting for game server connections
	AuctionMarket            * s_auctionMarketManager     = 0; // the Auction Market singleton containg all the logic
	CentralServerConnection  * s_centralServerConnection  = 0; // connection to central
	DatabaseServerConnection * s_databaseServerConnection = 0;
	CommodityServerMetricsData * s_commodityServerMetricsData = 0;
}

using namespace CommodityServerNamespace;

//-----------------------------------------------------------------------

CommodityServer::CommodityServer() :
Singleton<CommodityServer>(),
MessageDispatch::Receiver(),
m_gameserverMap(),
m_commoditiesServerLoadDone(0),
m_timeCommoditiesServerStarted(time(0)),
m_commoditiesServerLoadTime(-1)
{
	connectToMessage("CommoditiesLoadDone");
	connectToMessage("DatabaseServerConnectionClosed");
}

//-----------------------------------------------------------------------

CommodityServer::~CommodityServer()
{
}

//-----------------------------------------------------------------------

void CommodityServer::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	const uint32 messageType = message.getType();
	
	if(messageType == constcrc("CommoditiesLoadDone"))
	{
		m_commoditiesServerLoadDone = 1;

		// record how long it took the commodities server to load
		if (m_commoditiesServerLoadTime == -1)
			m_commoditiesServerLoadTime = (time(0) - m_timeCommoditiesServerStarted) / 60;
	}
	else if (messageType == constcrc("DatabaseServerConnectionClosed"))
	{
		WARNING(true, ("[Commodities Server] : No connection to the database server. Shutting down.\n"));
		exit(0);
	}
}

//-----------------------------------------------------------------------

int CommodityServer::addGameServer (GameServerConnection & gameserver)
{
	static int nextGameServerId = 0;
	int tmp = ++nextGameServerId;
	m_gameserverMap[tmp] = &gameserver;
	AuctionMarket::getInstance().SendItemTypeMap(gameserver);
	AuctionMarket::getInstance().SendResourceTypeMap(gameserver);
	return tmp;
}

//-----------------------------------------------------------------------

void CommodityServer::removeGameServer (int gameServerId)
{
	WARNING_STRICT_FATAL(gameServerId == 0, ("Tried to remove a gameserver with id == 0"));
	std::map<int, GameServerConnection*>::iterator i = m_gameserverMap.find(gameServerId);
	if (i != m_gameserverMap.end())
	{
		IGNORE_RETURN(m_gameserverMap.erase(gameServerId));
	}
}

//-----------------------------------------------------------------------

GameServerConnection* CommodityServer::getGameServer (int gameServerId)
{
	WARNING_STRICT_FATAL(gameServerId == 0, ("Tried to get a gameserver with id == 0"));
	if (gameServerId == -1)
	{
		// if requested game server id is -1 we don't know what game server
		// to send to so use the first known good one
		std::map<int, GameServerConnection*>::iterator i = m_gameserverMap.begin();
		while (i != m_gameserverMap.end())
		{
			if ((*i).second)
			{
				return (*i).second;
			}
			++i;
		}
		return 0;
	}
	else
	{
		std::map<int, GameServerConnection*>::iterator i = m_gameserverMap.find(gameServerId);
		if (i == m_gameserverMap.end())
			return 0;
		else
			return i->second;
	}
}

//-----------------------------------------------------------------------

void CommodityServer::sendToAllGameServers(const GameNetworkMessage & message)
{
	for (std::map<int, GameServerConnection*>::const_iterator i = m_gameserverMap.begin(); i != m_gameserverMap.end(); ++i)
		i->second->send(message, true);
}

//-----------------------------------------------------------------------

DatabaseServerConnection* CommodityServer::getDatabaseServer ()
{
	return s_databaseServerConnection;
}

//-----------------------------------------------------------------------

int CommodityServer::getCommoditiesServerLoadTime() const
{
	if (m_commoditiesServerLoadTime != -1)
		return m_commoditiesServerLoadTime;
	else
		// return how long the commodities server has been loading
		return ((time(0) - m_timeCommoditiesServerStarted) / 60);
}

//-----------------------------------------------------------------------

void CommodityServer::run()
{

	// setup log for CommoditiesServer
	SetupSharedLog::install("CommoditiesServer");

	int const sleepTimePerFrameMs = ConfigCommodityServer::getSleepTimePerFrameMs();

	NetworkSetupData setup;
	setup.port = ConfigCommodityServer::getCMServerServiceBindPort();
	setup.bindInterface = ConfigCommodityServer::getCMServerServiceBindInterface();
	setup.maxConnections=500;

	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Starting Commodities Auction Market Manager.\n"));
	AuctionMarket::getInstance().SingletonInialize();

	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Initiating connection to SWG Database Server.\n"));
	s_databaseServerConnection = new DatabaseServerConnection(ConfigCommodityServer::getDatabaseServerAddress(), ConfigCommodityServer::getDatabaseServerPort());

	Auction::Initialization();
	AuctionLocation::Initialization();
	AuctionMarket::getInstance().InitializeFromDB();

	s_commodityServerMetricsData = new CommodityServerMetricsData;
	MetricsManager::install(s_commodityServerMetricsData, false, "CommoditiesServer", "", 0);

	time_t timePrevious = ::time(nullptr);
	time_t timeCurrent = timePrevious;

	while (true)
	{
		volatile int i;
		i = CommodityServer::getInstance().m_commoditiesServerLoadDone;
		if (i == 1)
			break;
		NetworkHandler::update();

		timeCurrent = ::time(nullptr);
		MetricsManager::update(static_cast<float>((timeCurrent - timePrevious) * 1000));
		timePrevious = timeCurrent;

		NetworkHandler::dispatch();
		NetworkHandler::clearBytesThisFrame();
		Os::sleep(sleepTimePerFrameMs);
	}
	
	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Auction Market loaded and initialized.\n"));	
	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Starting Commodities Game Server Listening Service.\n"));
	s_gameServerService =     new Service(ConnectionAllocator<GameServerConnection>(), setup);
	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Initiating connection to central server.\n"));
	s_centralServerConnection = new CentralServerConnection(ConfigCommodityServer::getCentralServerAddress(), ConfigCommodityServer::getCentralServerPort());
	
	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Entering Commodities Server main message processing loop (sleep time %dms).\n", sleepTimePerFrameMs));

	// one time operation to request from the game server
	// (any game server) the list of GOT types and resource
	// classes that are excluded from the vendor/bazaar
	// Object Type Filter tree so that we can check to see
	// if we have any item that belongs in an excluded GOT type
	// or resource class so that we can remove that GOT type
	// or resource class from the excluded list;
	// this is not a high priority thing, so wait until
	// the cluster has started and "stabilized" before
	// doing this; 3 hours should be adequate
	time_t timeToRequestExcludedType = ::time(nullptr) + 10800;

	// one time request from the game server (any game server)
	// to receive the resource tree hierarchy to support
	// searching for resource container
	time_t timeToRequestResourceTree = ::time(nullptr);

	while(true)
	{
		NetworkHandler::update();

		timeCurrent = ::time(nullptr);
		MetricsManager::update(static_cast<float>((timeCurrent - timePrevious) * 1000));
		timePrevious = timeCurrent;

		NetworkHandler::dispatch();
		if (! CommodityServer::getInstance().m_gameserverMap.empty())
		{
			// one time request for a game server to send us the resource
			// tree hierarchy to support searching resource containers 
			if (!AuctionMarket::getInstance().HasReceivedResourceTreeHierarchy())
			{
				// keep asking every 10 seconds until we receive the resource tree
				if (timeToRequestResourceTree <= timeCurrent)
				{
					GameServerConnection * gsConn = CommodityServer::getInstance().getGameServer(-1);
					if (gsConn)
					{
						GenericValueTypeMessage<int> const msg("RequestResourceTreeHierarchy", 1);
						gsConn->send(msg, true);
					}

					timeToRequestResourceTree = timeCurrent + 10;
				}
			}

			if (timeToRequestExcludedType && (timeToRequestExcludedType <= timeCurrent))
			{
				GameServerConnection * gsConn = CommodityServer::getInstance().getGameServer(-1);
				if (gsConn)
				{
					GenericValueTypeMessage<int> const msg("RequestCommoditiesExcludedGotTypes", 1);
					gsConn->send(msg, true);

					GenericValueTypeMessage<int> const msg2("RequestCommoditiesExcludedResourceClasses", 1);
					gsConn->send(msg2, true);

					timeToRequestExcludedType = 0;
				}
			}

			AuctionMarket::getInstance().Update(time(0));
		}
		NetworkHandler::clearBytesThisFrame();
		Os::sleep(sleepTimePerFrameMs);
	}
	s_centralServerConnection = 0;
	s_gameServerService = 0;
	s_databaseServerConnection = 0;

	MetricsManager::remove();
	delete s_commodityServerMetricsData;
	s_commodityServerMetricsData = 0;
}

//-----------------------------------------------------------------------
