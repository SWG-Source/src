// ======================================================================
//
// DatabaseProcessConnection.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/DatabaseProcessConnection.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverNetworkMessages/GameGameServerMessages.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"

// ======================================================================

DatabaseProcessConnection::DatabaseProcessConnection(std::string const &remoteAddress, unsigned short remotePort, uint32 pid) :
	ServerConnection(remoteAddress, remotePort, NetworkSetupData()),
	m_pid(pid)
{
}

// ----------------------------------------------------------------------

DatabaseProcessConnection::~DatabaseProcessConnection()
{
}

// ----------------------------------------------------------------------

void DatabaseProcessConnection::onConnectionClosed()
{
	GameServer::getInstance().setDatabaseProcessConnection(0);
	GameServer::getInstance().setDone("DatabaseProcessConnection closed: %s", getDisconnectReason().c_str());
}

// ----------------------------------------------------------------------

void DatabaseProcessConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();

	LOG("GameGameConnect", ("DatabaseProcessConnection::onConnectionOpened() of %lu,%lu (preload %d)", m_pid, ServerConnection::getProcessId(), ConfigServerGame::getPreloadNumber()));
	GameGameServerConnect const connectMessage(GameServer::getInstance().getProcessId(), false, ConfigServerUtility::getSpawnCookie(), ConfigServerGame::getPreloadNumber());
	send(connectMessage, true);
}

// ----------------------------------------------------------------------

void DatabaseProcessConnection::onReceive(Archive::ByteStream const &message)
{
	ServerConnection::onReceive(message);
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage const msg(ri);
	ri = message.begin();

	if (msg.isType("GameGameServerConnect"))
	{
		GameGameServerConnect const connectMessage(ri);

		FATAL(!connectMessage.getIsDbProcess(), ("Received GameGameServerConnect message from something other than the database process?"));

		uint32 const pid = connectMessage.getProcessId();

		LOG("GameGameConnect", ("Received GameGameServerConnect message.  Old pid %lu new pid %lu. My pid %lu for Database Process", m_pid, pid, GameServer::getInstance().getProcessId()));
		m_pid = pid;
		GameServer::getInstance().setDatabaseProcessConnection(this);
	}
}

// ----------------------------------------------------------------------

uint32 DatabaseProcessConnection::getPid() const
{
	return m_pid;
}

// ======================================================================

