// LoginServerConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "CentralServer.h"
#include "CentralCSHandler.h"
#include "ConfigCentralServer.h"
#include "ConnectionServerConnection.h"
#include "ConsoleConnection.h"
#include "ConsoleCommandParser.h"
#include "ConsoleManager.h"
#include "GameServerConnection.h"
#include "LoginServerConnection.h"
#include "UniverseManager.h"
#include "serverNetworkMessages/CSDBNetMessages.h"
#include "serverNetworkMessages/CSToolRequest.h"
#include "serverNetworkMessages/CSToolResponse.h"
#include "serverNetworkMessages/GameServerCSRequestMessage.h"
#include "serverNetworkMessages/GameServerCSResponseMessage.h"
#include "serverNetworkMessages/LoginClusterName.h"
#include "serverNetworkMessages/LoginClusterName2.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "serverNetworkMessages/TransferAccountDataArchive.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/TransferReplyCharacterList.h"
#include "serverNetworkMessages/TransferReplyMoveValidation.h"
#include "serverUtility/FreeCtsDataTable.h"
#include "sharedFoundation/Clock.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

LoginServerConnection::LoginServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

LoginServerConnection::LoginServerConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t)
{
}

//-----------------------------------------------------------------------

LoginServerConnection::~LoginServerConnection()
{
}

//-----------------------------------------------------------------------

void LoginServerConnection::onConnectionClosed()
{
	static MessageConnectionCallback m("LoginConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void LoginServerConnection::onConnectionOpened()
{
	LoginClusterName2 n(ConfigCentralServer::getClusterName(), Clock::getTimeZone());
	send(n, true);

	// emit open message
	ServerConnection::onConnectionOpened();

	static MessageConnectionCallback m("LoginConnectionOpened");
	emitMessage(m);

}

//-----------------------------------------------------------------------

void LoginServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);
	ri = message.begin();
	
	const uint32 messageType = m.getType();
	
	switch (messageType) {
		case constcrc("TransferReplyCharacterList") :
		{
			// if this is not a free CTS galaxy, immediately forward the message to the TransferServer,
			// else if this is a free CTS galaxy, forward the message to a game server so that characters
			// in the list that qualifies for free CTS are removed from the list; characters that qualify
			// for free CTS should/must use the in-game free CTS to transfer the character;
			TransferReplyCharacterList reply(ri);
			if (!FreeCtsDataTable::isFreeCtsSourceCluster(ConfigCentralServer::getClusterName()))
				CentralServer::getInstance().sendToTransferServer(reply);
			else
				IGNORE_RETURN(CentralServer::getInstance().sendToRandomGameServer(reply));
			
			break;
		}
		case constcrc("TransferReplyCharacterDataFromLoginServer") :
		{
			GenericValueTypeMessage<TransferCharacterData> reply(ri);
			// see if we need to send to transfer or connection server.
			if(reply.getValue().getCSToolId() > 0)  
			{
				// for CS admin login.  send to DB.  This just shortcuts what would normally
				// happen at the transfer server.
				GenericValueTypeMessage<TransferCharacterData> login("TransferGetLoginLocationData", reply.getValue());
				CentralServer::getInstance().sendToDBProcess(login, true);
			}
			else if (reply.getValue().getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
			{
				// for CTS.  Send to transfer.
				CentralServer::getInstance().sendToTransferServer(reply);
			}
			break;
		}
		case constcrc("TransferReplyMoveValidation") :
		{
			TransferReplyMoveValidation const reply(ri);

			if (reply.getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
			{
				LOG("CustomerService", ("CharacterTransfer: Received TransferReplyMoveValidation for %s on %s to %s on %s from LoginServer. Forwarding reply to TransferServer.", reply.getSourceCharacter().c_str(), reply.getSourceGalaxy().c_str(), reply.getDestinationCharacter().c_str(), reply.getDestinationGalaxy().c_str()));
				CentralServer::getInstance().sendToTransferServer(reply);
			}
			else if (reply.getResult() != TransferReplyMoveValidation::TRMVR_can_create_regular_character)
			{
				LOG("CustomerService", ("CharacterTransfer: Received ***FAILED*** TransferReplyMoveValidation for [%u, %s, %s (%s)] to [%u, %s, %s] from LoginServer. Forwarding ***FAILED*** reply to GameServer (%u).", reply.getSourceStationId(), reply.getSourceGalaxy().c_str(), reply.getSourceCharacter().c_str(), reply.getSourceCharacterId().getValueString().c_str(), reply.getDestinationStationId(), reply.getDestinationGalaxy().c_str(), reply.getDestinationCharacter().c_str(), reply.getTrack()));
				GameServerConnection * gs = CentralServer::getInstance().getGameServer(reply.getTrack());
				if (!gs)
					gs = CentralServer::getInstance().getRandomGameServer();

				if (gs)
					gs->send(reply, true);
			}
			else
			{
				LOG("CustomerService", ("CharacterTransfer: Received ***SUCCESS*** TransferReplyMoveValidation for [%u, %s, %s (%s)] to [%u, %s, %s] from LoginServer. Sending TransferRequestNameValidation to a random game server.", reply.getSourceStationId(), reply.getSourceGalaxy().c_str(), reply.getSourceCharacter().c_str(), reply.getSourceCharacterId().getValueString().c_str(), reply.getDestinationStationId(), reply.getDestinationGalaxy().c_str(), reply.getDestinationCharacter().c_str()));

				TransferCharacterData requestData(reply.getTransferRequestSource());
				requestData.setTrack(reply.getTrack());
				requestData.setCustomerLocalizedLanguage(reply.getCustomerLocalizedLanguage());
				requestData.setSourceGalaxy(reply.getSourceGalaxy());
				requestData.setDestinationGalaxy(reply.getDestinationGalaxy());
				requestData.setSourceCharacterName(reply.getSourceCharacter());
				requestData.setCharacterId(reply.getSourceCharacterId());
				requestData.setObjectTemplateCrc(reply.getSourceCharacterTemplateId());
				requestData.setDestinationCharacterName(reply.getDestinationCharacter());
				requestData.setSourceStationId(reply.getSourceStationId());
				requestData.setDestinationStationId(reply.getDestinationStationId());
				const GenericValueTypeMessage<TransferCharacterData> requestNameValidation("TransferRequestNameValidation", requestData);
				IGNORE_RETURN(CentralServer::getInstance().sendToRandomGameServer(requestNameValidation));
			}
			
			break;
		}
		case constcrc("TransferReplyNameValidation") :
		{
			GenericValueTypeMessage<std::pair<std::string, TransferCharacterData> > const replyNameValidation(ri);
			if (!replyNameValidation.getValue().second.getIsValidName() || (replyNameValidation.getValue().second.getTransferRequestSource() == TransferRequestMoveValidation::TRS_ingame_freects_command_validate) || (replyNameValidation.getValue().second.getTransferRequestSource() == TransferRequestMoveValidation::TRS_ingame_cts_command_validate))
			{
				if (!replyNameValidation.getValue().second.getIsValidName())
					LOG("CustomerService", ("CharacterTransfer: Received ***FAILED*** TransferReplyNameValidation from target galaxy CentralServer : (%s) %s", replyNameValidation.getValue().first.c_str(), replyNameValidation.getValue().second.toString().c_str()));
				else
					LOG("CustomerService", ("CharacterTransfer: Received ***SUCCESS*** TransferReplyNameValidation from target galaxy CentralServer. Forwarding reply to GameServer (%u). %s", replyNameValidation.getValue().second.getTrack(), replyNameValidation.getValue().second.toString().c_str()));

				GameServerConnection * gs = CentralServer::getInstance().getGameServer(replyNameValidation.getValue().second.getTrack());
				if (!gs)
					gs = CentralServer::getInstance().getRandomGameServer();

				if (gs)
					gs->send(replyNameValidation, true);
			}
			else
			{
				LOG("CustomerService", ("CharacterTransfer: Received ***SUCCESS*** TransferReplyNameValidation from target galaxy CentralServer. Starting transfer process : %s", replyNameValidation.getValue().second.toString().c_str()));

				// disconnect any clients with a connection to SWG services on this (the source) galaxy
				GenericValueTypeMessage<unsigned int> kickSource("TransferKickConnectedClients", replyNameValidation.getValue().second.getSourceStationId());
				GenericValueTypeMessage<unsigned int> kickDestination("TransferKickConnectedClients", replyNameValidation.getValue().second.getDestinationStationId());
				CentralServer::getInstance().sendToAllLoginServers(kickSource);
				CentralServer::getInstance().sendToAllLoginServers(kickDestination);
				CentralServer::getInstance().sendToAllConnectionServers(kickSource, true);
				CentralServer::getInstance().sendToAllConnectionServers(kickDestination, true);

				TransferCharacterData characterData(replyNameValidation.getValue().second);
				characterData.setIsMoveRequest(true);
				characterData.setWithItems(true);
				characterData.setAllowOverride(true);

				LOG("CustomerService", ("CharacterTransfer: Sending TransferGetLoginLocationData for %s\n", characterData.getSourceCharacterName().c_str()));

				const GenericValueTypeMessage<TransferCharacterData> getLoginLocation("TransferGetLoginLocationData", characterData);
				CentralServer::getInstance().sendToDBProcess(getLoginLocation, true);
			}
			break;
		}
		case constcrc("TransferLoginCharacterToDestinationServer") :
		{
			GenericValueTypeMessage<TransferCharacterData> const loginCharacter(ri);
			LOG("CustomerService", ("CharacterTransfer: ***CentralServer: Received TransferLoginCharacterToDestinationServer request from Login Server for %s\n", loginCharacter.getValue().toString().c_str()));
			ConnectionServerConnection * connectionServer = CentralServer::getInstance().getAnyConnectionServer();
			if(connectionServer)
			{
				connectionServer->send(loginCharacter, true);
			}
			break;
		}
		case constcrc("TransferClosePseudoClientConnection") :
		{
			GenericValueTypeMessage<unsigned int> const closeRequest(ri);
			CentralServer::getInstance().sendToAllConnectionServers(closeRequest, true);
			break;
		}
		case constcrc("TransferCloseClientConnection") :
		{
			GenericValueTypeMessage<unsigned int> const closeRequest(ri);

			GenericValueTypeMessage<unsigned int> const kick("TransferKickConnectedClients", closeRequest.getValue());
			CentralServer::getInstance().sendToAllLoginServers(kick);
			CentralServer::getInstance().sendToAllConnectionServers(kick, true);
			break;
		}
		case constcrc("CtsCompletedForcharacter") :
		{
			GenericValueTypeMessage<std::pair<std::string, NetworkId> > const msg(ri);
			IGNORE_RETURN(CentralServer::getInstance().sendToRandomGameServer(msg));
			break;
		}
		case constcrc("TransferRenameCharacterReplyFromLoginServer") :
		{
			GenericValueTypeMessage<TransferCharacterData> replyFromLoginServer(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferRenameCharacterReplyFromLoginServer : %s", replyFromLoginServer.getValue().toString().c_str()));
			
			GenericValueTypeMessage<TransferCharacterData> reply("TransferRenameCharacterReply", replyFromLoginServer.getValue());
			CentralServer::getInstance().sendToTransferServer(reply);
			
			break;
		}
		case constcrc("TransferAccountRequestCentralDatabase") :
		{
			// got message back from loginServer - have to transfer to DBProcess to update the game database
			GenericValueTypeMessage<TransferAccountData> accountTransferRequest(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferAccountRequestCentralDatabase from station ID %d to station ID %d", accountTransferRequest.getValue().getSourceStationId(), accountTransferRequest.getValue().getDestinationStationId()));
			CentralServer::getInstance().sendToDBProcess(accountTransferRequest, true);
			break;
		}
		case constcrc("TransferAccountReplySuccessTransferServer") :
		{
			// got reply back from login server - fwd to transfer server
			GenericValueTypeMessage<TransferAccountData> replyToTransferServer(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferAccountReplySuccessTransferServer from station ID %d to station ID %d", replyToTransferServer.getValue().getSourceStationId(), replyToTransferServer.getValue().getDestinationStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToTransferServer(replyToTransferServer));
			
			break;
		}
		case constcrc("TransferAccountFailedToUpdateGameDatabase") :
		{
			// got failure message back from login server - fwd to transfer server
			GenericValueTypeMessage<TransferAccountData> replyToTransferServer(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferAccountFailedToUpdateGameDatabase for transfer from station ID %d to station ID %d", replyToTransferServer.getValue().getSourceStationId(), replyToTransferServer.getValue().getDestinationStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToTransferServer(replyToTransferServer));
			break;
		}
		case constcrc("TransferAccountFailedDestinationNotEmpty") :
		{
			// got failure message back from login server - fwd to transfer server
			GenericValueTypeMessage<TransferAccountData> replyToTransferServer(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferAccountFailedDestinationNotEmpty for transfer from station ID %d to station ID %d", replyToTransferServer.getValue().getSourceStationId(), replyToTransferServer.getValue().getDestinationStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToTransferServer(replyToTransferServer));
			break;
		}
		case constcrc("CSToolRequest") :
		{
			// for now, just parse the command through the command queue.
			// TODO:  implement command processor for this, specifically.  Or, at least, make sure we're
			// specifying access levels.
			CSToolRequest const req(ri);
			
			// Doesn't appear to be too much that we can really do with the Central Server.
			// So, for now, we'll just implement a 'central' command here and spam the GameServers with
			// the original message otherwise.
			
			unsigned pos = req.getCommandString().find( " " );
			pos = pos == std::string::npos ? req.getCommandString().length() : pos;
			
			CentralCSHandler::getInstance().handle( req, getProcessId() );
			
			break;
		}
		case constcrc("ConGenericMessage") :
		{
			ConGenericMessage con(ri);
			ConsoleConnection::onCommandComplete(con.getMsg(), static_cast<int>(con.getMsgId()));
			
			break;
		}
		case constcrc("AllCluserGlobalChannel") :
		{
			//TODO: Route to connection server directly if there is no channel?
			typedef std::pair<std::pair<std::string,std::string>, bool> PayloadType;
			GenericValueTypeMessage<PayloadType> msg(ri);

			PayloadType const & payload = msg.getValue();
			std::string const & channelName = payload.first.first;
			std::string const & messageText = payload.first.second;
			bool const & isRemove = payload.second;

			LOG("CustomerService", ("BroadcastVoiceChannel: CentralServer got AllCluserGlobalChannel from LoginServer, sending to all ChatServers chan(%s) text(%s) remove(%d)",
				channelName.c_str(), messageText.c_str(), (isRemove?1:0) ));

			CentralServer::getInstance().broadcastToChatServers(msg);
			
			break;
		}
		case constcrc("GcwScoreStatRaw") :
		{
			GenericValueTypeMessage<std::pair<std::string, std::pair<std::map<std::string, std::pair<int64, int64> >, std::map<std::string, std::pair<int64, int64> > > > > const msg(ri);
			if (_stricmp(msg.getValue().first.c_str(), ConfigCentralServer::getClusterName()))
			{
				GameServerConnection * universeGameServerConnection = CentralServer::getInstance().getGameServer(UniverseManager::getInstance().getUniverseProcess());
				if (universeGameServerConnection)
				{
					universeGameServerConnection->send(msg, true);
				}
			}
			break;
		}
		case constcrc("GcwScoreStatPct") :
		{
			GenericValueTypeMessage<std::pair<std::string, std::pair<std::map<std::string, int>, std::map<std::string, int> > > > const msg(ri);
			if (_stricmp(msg.getValue().first.c_str(), ConfigCentralServer::getClusterName()))
			{
				GameServerConnection * universeGameServerConnection = CentralServer::getInstance().getGameServer(UniverseManager::getInstance().getUniverseProcess());
				if (universeGameServerConnection)
				{
					universeGameServerConnection->send(msg, true);
				}
			}
			break;
		}
	}

	// emit the message in case someone else wants to use it
	ServerConnection::onReceive(message);
	
}

//-----------------------------------------------------------------------
