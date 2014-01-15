// ======================================================================
//
// PlanetServerConnection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlanetServerConnection.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverNetworkMessages/GameGameServerMessages.h" // we use the GameGameServerConnect message
#include "serverUtility/ConfigServerUtility.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"

// ======================================================================

PlanetServerConnection::PlanetServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

PlanetServerConnection::~PlanetServerConnection()
{
}

//-----------------------------------------------------------------------

void PlanetServerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();

	DEBUG_REPORT_LOG(true, ("Planet server closed connection.  GameServer will exit.\n"));
	static MessageConnectionCallback m("PlanetConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void PlanetServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();

	static MessageConnectionCallback m("PlanetConnectionOpened");
	emitMessage(m);

	// send process id
	GameGameServerConnect c(GameServer::getInstance().getProcessId(), false, ConfigServerUtility::getSpawnCookie(), ConfigServerGame::getPreloadNumber());
	send(c, true);

	LOG("PlanetServerConnections",("Game Server %lu (preload %d) connected to planet server and sent GameGameServerConnect", GameServer::getInstance().getProcessId(), ConfigServerGame::getPreloadNumber()));
}

//-----------------------------------------------------------------------
