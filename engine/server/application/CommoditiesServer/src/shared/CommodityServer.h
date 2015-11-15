// ======================================================================
//
// ConfigCommodityServer.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
// Server Infrastructure by: Justin Randall
//
// This is the primary entry point to the commodities service. This class
// is called from main.cpp. The main server loop is encapsulated here.
//
// ======================================================================
#ifndef	_INCLUDED_CommodityServer_H
#define	_INCLUDED_CommodityServer_H

//-----------------------------------------------------------------------

#include "Singleton/Singleton.h"
#include "Archive/ByteStream.h"
#include "sharedMessageDispatch/Receiver.h"
#include <map>

class GameNetworkMessage;
class GameServerConnection;
class DatabaseServerConnection;

//-----------------------------------------------------------------------

class CommodityServer : public Singleton<CommodityServer>, public MessageDispatch::Receiver
{
public:
	CommodityServer();
	~CommodityServer();

	void                       receiveMessage      (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	
	int                        addGameServer       (GameServerConnection & gameserver);
	void                       removeGameServer    (int gameServerId);
	GameServerConnection*      getGameServer       (int gameServerId);
	void                       sendToAllGameServers(const GameNetworkMessage & message);
	DatabaseServerConnection*  getDatabaseServer   ();
	int                        getCommoditiesServerLoadTime() const;

	static void run();

private:

	std::map<int, GameServerConnection *> m_gameserverMap;
	int                                   m_commoditiesServerLoadDone;
	time_t                                m_timeCommoditiesServerStarted;
	int                                   m_commoditiesServerLoadTime;

	CommodityServer & operator = (const CommodityServer & rhs);
	CommodityServer(const CommodityServer & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CommodityServer_H
