// GameServerStatus.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GameServerStatus.h"

//-----------------------------------------------------------------------

GameServerStatus::GameServerStatus(const bool isOnline, const ServerInfo & serverInfo) :
GameNetworkMessage("GameServerStatus"),
m_online(isOnline),
m_serverInfo(serverInfo)
{
	addVariable(m_online);
	addVariable(m_serverInfo);
}

//-----------------------------------------------------------------------

GameServerStatus::GameServerStatus(Archive::ReadIterator & source) :
GameNetworkMessage("GameServerStatus"),
m_online(),
m_serverInfo()
{
	addVariable(m_online);
	addVariable(m_serverInfo);
	unpack(source);
}

//-----------------------------------------------------------------------

GameServerStatus::~GameServerStatus()
{
}

//-----------------------------------------------------------------------

const ServerInfo & GameServerStatus::getServerInfo() const
{
	return m_serverInfo.get();
}

//-----------------------------------------------------------------------

const bool GameServerStatus::isOnline() const
{
	return m_online.get();
}

//-----------------------------------------------------------------------

