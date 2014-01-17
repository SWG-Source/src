// CentralConnection.cpp
// copyright 2001 Verant Interactive

//-----------------------------------------------------------------------

#include "FirstConnectionServer.h"
#include "CentralConnection.h"

#include "Archive/ByteStream.h"
#include "ConnectionServer.h"
#include "PseudoClientConnection.h"
#include "serverKeyShare/KeyShare.h"
#include "serverNetworkMessages/CentralConnectionServerMessages.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

//-----------------------------------------------------------------------

CentralConnection::CentralConnection(const std::string & address, const unsigned short port) :
ServerConnection(address, port, NetworkSetupData())
{
}


//-----------------------------------------------------------------------
CentralConnection::CentralConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t)
{
	PseudoClientConnection::destroyAllPseudoClientConnectionInstances();
}

//-----------------------------------------------------------------------

CentralConnection::~CentralConnection()
{
	PseudoClientConnection::destroyAllPseudoClientConnectionInstances();
}

//-----------------------------------------------------------------------


void CentralConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("CentralConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CentralConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
	static const MessageConnectionCallback m("CentralConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CentralConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	const GameNetworkMessage msg(ri);
	ri = message.begin();

	if(msg.isType("TransferLoginCharacterToSourceServer"))
	{
		GenericValueTypeMessage<TransferCharacterData> login(ri);
		// received a request to create a pseudoclient and connect it
		// to a game server.
		LOG("CustomerService", ("CharacterTransfer: ***ConnectionServer: Received TransferLoginCharacterToSourceServer request from CentralServer for %s\n", login.getValue().toString().c_str()));
		IGNORE_RETURN(PseudoClientConnection::tryToDeliverMessageTo(login.getValue().getSourceStationId(), message));
	}
	else if(msg.isType("TransferLoginCharacterToDestinationServer"))
	{
		GenericValueTypeMessage<TransferCharacterData> login(ri);
		LOG("CustomerService", ("CharacterTransfer: ***ConnectionServer: Received TransferLoginCharacterToDestinationServer request from CentralServer for %s", login.getValue().toString().c_str()));
		IGNORE_RETURN(PseudoClientConnection::tryToDeliverMessageTo(login.getValue().getDestinationStationId(), message));
	}
	else if(msg.isType("CtsSrcCharWrongPlanet"))
	{
		GenericValueTypeMessage<std::pair<NetworkId, unsigned int> > const failureMsg(ri);
		LOG("CustomerService", ("CharacterTransfer: ***ConnectionServer: Received CtsSrcCharWrongPlanet error from CentralServer for character (%s) stationId (%u) because character is not one of the 10 original ground planets", failureMsg.getValue().first.getValueString().c_str(), failureMsg.getValue().second));
		IGNORE_RETURN(PseudoClientConnection::tryToDeliverMessageTo(failureMsg.getValue().second, message));
	}
	else if(msg.isType("TransferKickConnectedClients"))
	{
		GenericValueTypeMessage<unsigned int> kick(ri);
		ClientConnection * clientConnection = ConnectionServer::getClientConnection(kick.getValue());
		if(clientConnection)
		{
			ConnectionServer::dropClient(clientConnection, "TransferServer requests client drop");
		}
	}
	else if(msg.isType("TransferClosePseudoClientConnection"))
	{
		GenericValueTypeMessage<unsigned int> closeRequest(ri);
		PseudoClientConnection * pseudoClient = PseudoClientConnection::getPseudoClientConnection(closeRequest.getValue());
		delete pseudoClient;
	}
	else if(msg.isType("LoginDeniedRecentCTS"))
	{
		GenericValueTypeMessage<std::pair<NetworkId, uint32> > loginDeniedRecentCTS(ri);
		ClientConnection * clientConnection = ConnectionServer::getClientConnection(loginDeniedRecentCTS.getValue().second);
		if (clientConnection)
		{
			LOG("CustomerService", ("Login:%s, character %s (%s) is a recent CTS that has not been persisted yet.", ClientConnection::describeAccount(clientConnection).c_str(), clientConnection->getCharacterName().c_str(), clientConnection->getCharacterId().getValueString().c_str()));
			ErrorMessage err("Login Failed", "The selected character has just been recently transferred and has not been completely initialized.  In most cases, it takes about 15 minutes (but in some cases can take up to 2 hours) to complete initialization.  Please try again later.");
			clientConnection->send(err, true);
		}
	}
	else if(msg.isType("LoginDeniedPendingPlayerRenameRequest"))
	{
		GenericValueTypeMessage<std::pair<NetworkId, uint32> > loginDeniedPendingPlayerRenameRequest(ri);
		ClientConnection * clientConnection = ConnectionServer::getClientConnection(loginDeniedPendingPlayerRenameRequest.getValue().second);
		if (clientConnection)
		{
			LOG("CustomerService", ("Login:%s, character %s (%s) has a pending player requested character rename request.", ClientConnection::describeAccount(clientConnection).c_str(), clientConnection->getCharacterName().c_str(), clientConnection->getCharacterId().getValueString().c_str()));
			ErrorMessage err("Login Failed", "The selected character currently has a pending character rename request.  It can take up to 30 minutes for the rename request to complete.");
			clientConnection->send(err, true);
		}
	}
	else
	{
		ServerConnection::onReceive(message);
	}
}

//-----------------------------------------------------------------------
