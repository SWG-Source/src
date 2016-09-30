// TransferServerConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "ConfigCentralServer.h"
#include "ConnectionServerConnection.h"
#include "ConsoleConnection.h"
#include "serverNetworkMessages/CharacterTransferStatusMessage.h"
#include "serverNetworkMessages/ToggleAvatarLoginStatus.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "serverNetworkMessages/TransferAccountDataArchive.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/TransferRequestMoveValidation.h"
#include "serverNetworkMessages/UploadCharacterMessage.h"
#include "serverNetworkMessages/ValidateAccountMessage.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "TransferServerConnection.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

TransferServerConnection::TransferServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

TransferServerConnection::~TransferServerConnection()
{
}

//-----------------------------------------------------------------------

void TransferServerConnection::onConnectionClosed()
{
	static MessageConnectionCallback m("TransferServerConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void TransferServerConnection::onConnectionOpened()
{
	LOG("TransferServerConnection", ("Connection with the transfer server is open, sending CentralGalaxyName message(%s)", ConfigCentralServer::getClusterName()));
	static MessageConnectionCallback m("TransferServerConnectionOpened");
	emitMessage(m);

	// send galaxy name to the character transfer server
	GenericValueTypeMessage<std::string> cgn("CentralGalaxyName", ConfigCentralServer::getClusterName());
	send(cgn, true);
}

//-----------------------------------------------------------------------

void TransferServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	const GameNetworkMessage msg(ri);
	ri = message.begin();
	
	const uint32 messageType = msg.getType();

	switch(messageType) {
		case constcrc("UploadCharacterMessage") :
		{
			const UploadCharacterMessage ucm(ri);
			CentralServer::getInstance().sendToGameServer(ucm.getFromGameServerId(), ucm, true);
			LOG("TransferServerConnection", ("Received character data for SUID %lu for character object %s on game server %d", ucm.getStationId(), ucm.getFromCharacterId().getValueString().c_str(), ucm.getFromGameServerId()));
			break;
		}
		case constcrc("CharacterTransferStatusMessage") :
		{
			const CharacterTransferStatusMessage ctsm(ri);
			CentralServer::getInstance().sendToGameServer(ctsm.getGameServerId(), ctsm, true);
			break;
		}
		case constcrc("TransferRequestCharacterList") :
		{
			// simply forward the request to any available login server
			const GenericValueTypeMessage<TransferCharacterData> request(ri);
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(request));
			break;
		}
		case constcrc("TransferRequestMoveValidation") :
		{
			const TransferRequestMoveValidation request(ri);
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(request));
			LOG("CustomerService", ("CharacterTransfer: Received TransferRequestMoveValidation : %s on %s to %s on %s. Forwarding request to LoginServer.", request.getSourceCharacter().c_str(), request.getSourceGalaxy().c_str(), request.getDestinationCharacter().c_str(), request.getDestinationGalaxy().c_str()));
			break;
		}
		case constcrc("TransferRequestKickConnectedClients") :
		{
			GenericValueTypeMessage<std::pair<unsigned int, unsigned int> > const request(ri);
			// disconnect any clients with a connection to SWG services
			GenericValueTypeMessage<unsigned int> kickSource("TransferKickConnectedClients", request.getValue().first);
			GenericValueTypeMessage<unsigned int> kickDestination("TransferKickConnectedClients", request.getValue().second);
			CentralServer::getInstance().sendToAllLoginServers(kickSource);
			CentralServer::getInstance().sendToAllLoginServers(kickDestination);
			CentralServer::getInstance().sendToAllConnectionServers(kickSource, true);
			CentralServer::getInstance().sendToAllConnectionServers(kickDestination, true);
			break;
		}
		case constcrc("TransferGetCharacterDataFromLoginServer") :
		{
			const GenericValueTypeMessage<TransferCharacterData> getCharacterData(ri);
			
			// The transfer server has received a move request. The request doesn't
			// contain the source id of the character, character's container or scene
			// information. The LoginServer can retrieve the character ID for the 
			// source station id + character name contained in character data.
			// Forward the request to the login server. It will respond and the
			// message can be routed back to the transfer server for further 
			// processing
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(getCharacterData));
			LOG("CustomerService", ("CharacterTransfer: ***CentralServer: received TransferGetCharacterDataFromLoginServer : %s)", getCharacterData.getValue().toString().c_str()));
			break;
		}
		case constcrc("TransferGetLoginLocationData") :
		{
			GenericValueTypeMessage<TransferCharacterData> getLoginData(ri);
			CentralServer::getInstance().sendToDBProcess(getLoginData, true);
			LOG("CustomerService", ("CharacterTransfer: ***CentralServer: Received TransferGetLoginLocationData for %s\n", getLoginData.getValue().getSourceCharacterName().c_str()));
			break;
		}
		case constcrc("TransferLoginCharacterToSourceServer") :
		{
			const GenericValueTypeMessage<TransferCharacterData> loginCharacter(ri);
			ConnectionServerConnection * connectionServer = CentralServer::getInstance().getAnyConnectionServer();
			if(connectionServer)
			{
				connectionServer->send(loginCharacter, true);
				LOG("CustomerService", ("CharacterTransfer: ***CentralServer: Received TransferLoginCharacterToSourceServer request from Transfer Server. Sendint request to ConnectionServer : %s\n", loginCharacter.getValue().toString().c_str()));
			}
			else
			{
				LOG("CustomerService", ("CharacterTransfer: ***CentralServer: Couldn't find a connection server to satisfy the transfer request for %s\n", loginCharacter.getValue().toString().c_str()));
			}
			break;
		}
		case constcrc("TransferLoginCharacterToDestinationServer") :
		{
			const GenericValueTypeMessage<TransferCharacterData> loginCharacter(ri);
			LOG("CustomerService", ("CharacterTransfer: ***CentralServer: Received TransferLoginCharacterToDestinationServer request from Transfer Server for %s\n", loginCharacter.getValue().toString().c_str()));
			ConnectionServerConnection * connectionServer = CentralServer::getInstance().getAnyConnectionServer();
			if(connectionServer)
			{
				connectionServer->send(loginCharacter, true);
			}
			break;
		}
		case constcrc("ToggleAvatarLoginStatus") :
		{
			ToggleAvatarLoginStatus toggle(ri);
			LOG("CustomerService", ("CharacterTransfer: Received ToggleAvatarLoginStatus for station ID %d, Character Object %s", toggle.getStationId(), toggle.getCharacterId().getValueString().c_str()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(toggle));
			break;
		}
		case constcrc("TransferRequestNameValidation") :
		{
			GenericValueTypeMessage<TransferCharacterData> request(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferRequestNameValidation from TransferServer. Forwarding request to a random GameServer. %s", request.getValue().toString().c_str()));
			IGNORE_RETURN(CentralServer::getInstance().sendToRandomGameServer(request));
			break;
		}
		case constcrc("TransferRenameCharacter") :
		{
			GenericValueTypeMessage<TransferCharacterData> renameRequest(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferRenameCharacter. Sending rename request to game database. %s", renameRequest.getValue().toString().c_str()));
			// send first portion of request to DBProcess , wait for response
			CentralServer::getInstance().sendToDBProcess(renameRequest, true);
			break;
		}
		case constcrc("TransferAccountRequestLoginServer") :
		{
			// got message to transfer a character from one account to another - have to update in loginServer
			GenericValueTypeMessage<TransferAccountData> accountTransferRequest(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferAccountRequestLoginServer from station ID %d to station ID %d", accountTransferRequest.getValue().getSourceStationId(), accountTransferRequest.getValue().getDestinationStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(accountTransferRequest));
			break;
		}
		case constcrc("RequestChatTransferAvatar") :
		{
			GenericValueTypeMessage<TransferCharacterData> chatTransferAvatar(ri);
			LOG("CustomerService", ("CharacterTransfer: Received RequestChatTransferAvatar from TransferServer. Forward request to ChatServer. %s", chatTransferAvatar.getValue().toString().c_str()));
			CentralServer::getInstance().broadcastToChatServers(chatTransferAvatar);
			break;
		}
		case constcrc("DeleteFailedTransfer") :
		{
			GenericValueTypeMessage<TransferCharacterData> deleteCharacter(ri);
			LOG("CustomerService", ("CharacterTransfer: received request to delete a character for a failed transfer. %s", deleteCharacter.getValue().toString().c_str()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(deleteCharacter));
			break;
		}
		case constcrc("TransferClosePseudoClientConnection") :
		{
			GenericValueTypeMessage<unsigned int> closeRequest(ri);
			CentralServer::getInstance().sendToAllConnectionServers(closeRequest, true);
			break;
		}
		case constcrc("ConGenericMessage") :
		{
			ConGenericMessage con(ri);
			ConsoleConnection::onCommandComplete(con.getMsg(), static_cast<int>(con.getMsgId()));
			break;		
		}
	}
}

//-----------------------------------------------------------------------
