// CentralServerConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "FirstCustomerServiceServer.h"
#include "CentralServerConnection.h"

#include "CustomerServiceServer.h"
#include "Archive/ByteStream.h"
#include "serverNetworkMessages/EnumerateServers.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetwork/Service.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

CentralServerConnection::CentralServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
	LOG("CentralServerConnection", ("Connection created...listening on (%s:%d)", a.c_str(), static_cast<int>(p)));
}

//-----------------------------------------------------------------------

CentralServerConnection::~CentralServerConnection()
{
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionClosed()
{
	LOG("CentralServerConnection", ("onConnectionClosed()"));

	CustomerServiceServer::getInstance().setDone(true);
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionOpened()
{
	LOG("CentralServerConnection", ("onConnectionOpened()"));

	{
		// Send to central that the address for the chat server to use when communicating with the customer service server
		
		Service *chatServerService = CustomerServiceServer::getInstance().getChatServerService();

		if (chatServerService != nullptr)
		{
			const std::string address(chatServerService->getBindAddress());
			const int port = chatServerService->getBindPort();

			//DEBUG_REPORT_LOG(true, ("CustServ: sending chat server service address(%s:%d)\n", address.c_str(), port));

			const GenericValueTypeMessage<std::pair<std::string, unsigned short> > message("CustomerServiceServerChatServerServiceAddress", std::make_pair(address, port));
			
			send(message, true);
		}
		else
		{
			LOG("CentralServerConnection", ("onConnectionOpened() ERROR: ChatServerService is nullptr"));
		}
	}

	{
		// Send to central that the address for the game server to use when communicating with the customer service server
		
		Service *gameServerService = CustomerServiceServer::getInstance().getGameServerService();

		if (gameServerService != nullptr)
		{
			const std::string address(gameServerService->getBindAddress());
			const int port = gameServerService->getBindPort();

			//DEBUG_REPORT_LOG(true, ("CustServ: sending game server service address(%s:%d)\n", address.c_str(), port));

			const GenericValueTypeMessage<std::pair<std::string, unsigned short> > message("CustomerServiceServerGameServerServiceAddress", std::make_pair(address, port));
			
			send(message, true);
		}
		else
		{
			LOG("CentralServerConnection", ("onConnectionOpened() ERROR: GameServerService is nullptr"));
		}
	}
}

//-----------------------------------------------------------------------

void CentralServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);

	ri = message.begin();

	if(m.isType("EnumerateServers"))
	{
		static MessageDispatch::Transceiver<const EnumerateServers &> emitter;

		EnumerateServers e(ri);
		emitter.emitMessage(e);
	}
}

//-----------------------------------------------------------------------

