// ======================================================================
//
// CentralServerConnection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "CentralServerConnection.h"

#include "ConsoleManager.h"
#include "serverNetworkMessages/CentralPlanetServerConnect.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "ConfigPlanetServer.h"
#include "PlanetServer.h"

//-----------------------------------------------------------------------

CentralServerConnection::CentralServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

CentralServerConnection::~CentralServerConnection()
{
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();

	static MessageConnectionCallback m("CentralConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();

	DEBUG_REPORT_LOG(true,("Connection to Central opened.\n"));

	CentralPlanetServerConnect msg(ConfigPlanetServer::getSceneID(), PlanetServer::getInstance().getGameService()->getBindAddress(), PlanetServer::getInstance().getGameServicePort());
	send(msg,true);

	PlanetServer::getInstance().onCentralConnected(this);

	static MessageConnectionCallback m("CentralConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CentralServerConnection::onReceive(const Archive::ByteStream & message)
{
    Archive::ReadIterator ri = message.begin();
    const GameNetworkMessage msg(ri);
    ri = message.begin();

	if(msg.isType("ConGenericMessage"))
	{
		const ConGenericMessage cm(ri);
		std::string result;
		ConsoleManager::processString(cm.getMsg(), cm.getMsgId(), result);
		const ConGenericMessage response(result, cm.getMsgId());
		send(response, true);
	}

	ServerConnection::onReceive(message);
}

//-----------------------------------------------------------------------
