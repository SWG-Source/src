// ======================================================================
//
// GameServerConnection.cpp
//
// copyright 2000 Verant Interactive
// Author: Justin Randall
// ======================================================================

#include "FirstCentralServer.h"
#include "ConsoleConnection.h"
#include "CentralCSHandler.h"
#include "CharacterCreationTracker.h"
#include "ConfigCentralServer.h"
#include "GameServerConnection.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/CSDBNetMessages.h"
#include "serverNetworkMessages/CSToolResponse.h"
#include "serverNetworkMessages/DownloadCharacterMessage.h"
#include "serverNetworkMessages/FeatureIdTransactionRequest.h"
#include "serverNetworkMessages/FeatureIdTransactionSyncUpdate.h"
#include "serverNetworkMessages/GameServerCSResponseMessage.h"
#include "serverNetworkMessages/GameServerUniverseLoadedMessage.h"
#include "serverNetworkMessages/LocateObjectResponseMessage.h"
#include "serverNetworkMessages/LocatePlayerResponseMessage.h"
#include "serverNetworkMessages/SPCharacterProfileMessage.h"
#include "serverNetworkMessages/StructureListMessage.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "serverNetworkMessages/TeleportToMessage.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/TransferReplyCharacterList.h"
#include "serverNetworkMessages/TransferRequestMoveValidation.h"
#include "serverNetworkMessages/UploadCharacterMessage.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "sharedNetworkMessages/GenericValueTypeMessage.h"

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

GameServerConnection::GameServerConnection(UdpConnectionMT *u, TcpClient *t) :
	ServerConnection(u, t),
	clientServiceAddress(""),
	clientServicePort(0),
	gameServiceAddress(""),
	gameServicePort(0),
	m_sceneId(),
	m_ready(false)
{
}

// ----------------------------------------------------------------------

GameServerConnection::~GameServerConnection()
{
	CentralServer::getInstance().removeGameServer(this);
}

// ----------------------------------------------------------------------

void GameServerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();

	static MessageConnectionCallback const m("GameConnectionClosed");
	emitMessage(m);
}

// ----------------------------------------------------------------------

void GameServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();

	FATAL((CentralServer::getInstance().getClusterId() == 0),("Got GameServerConnection::onConnectionOpened() before receiving cluster Id from LoginServer"));

	GenericValueTypeMessage<uint32> const msg("ClusterId", static_cast<uint32>(CentralServer::getInstance().getClusterId()));
	send(msg, true);

	static MessageConnectionCallback const m("GameConnectionOpened");
	emitMessage(m);
}

// ----------------------------------------------------------------------

void GameServerConnection::onReceive(Archive::ByteStream const &message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage const m(ri);
	ri = message.begin();
	
	const uint32 messageType = m.getType();

	switch(messageType) {
		case constcrc("CentralGameServerConnect") :
		{
			CentralGameServerConnect const c(ri);
			setClientServiceAddress(c.getClientServiceAddress());
			setClientServicePort(c.getClientServicePort());
			setGameServiceAddress(c.getGameServiceAddress());
			setGameServicePort(c.getGameServicePort());
			setSceneId(c.getVolumeName());
			break;
		}
		case constcrc("GameServerUniverseLoadedMessage") :
		{
			m_ready = true;

			if (ConfigCentralServer::getGameServerConnectionPendingAllocatedSizeLimit() != 0)
				setTcpClientPendingSendAllocatedSizeLimit(0);
			break;
		}
		case constcrc("TaskSpawnProcess") :
		{
			TaskSpawnProcess const spawn(ri);
			CentralServer::getInstance().sendTaskMessage(spawn);
			break;
		}
		case constcrc("ConGenericMessage") :
		{
			ConGenericMessage const con(ri);
			ConsoleConnection::onCommandComplete(con.getMsg(), static_cast<int>(con.getMsgId()));
			break;
		}
		case constcrc("TransferReplyNameValidation") :
		{
			GenericValueTypeMessage<std::map<std::string, TransferCharacterData> > const replyNameValidation(ri);
			auto i = replyNameValidation.getValue().begin();

			if (i == replyNameValidation.getValue().end()) {
				break;
			}

			if (i->second.getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
			{
				LOG("CustomerService", ("CharacterTransfer: Received TransferReplyNameValidation from GameServer, forwarding to TransferServer : %s", i->second.toString().c_str()));
				CentralServer::getInstance().sendToTransferServer(replyNameValidation);
			}
			else
			{
				// pass reply back to the source galaxy for handling, which is to
				// either display an error message to the user if the request failed,
				// or to start the transfer process if the request succeeds
				LOG("CustomerService", ("CharacterTransfer: Received TransferReplyNameValidation from GameServer, forwarding to source galaxy CentralServer : %s", i->second.toString().c_str()));
				CentralServer::getInstance().sendToArbitraryLoginServer(replyNameValidation);

				// if the request succeeded, also disconnect any clients with a connection to SWG services on this (the target) galaxy
				if (i->second.getIsValidName() && (i->second.getTransferRequestSource() != TransferRequestMoveValidation::TRS_ingame_freects_command_validate) && (i->second.getTransferRequestSource() != TransferRequestMoveValidation::TRS_ingame_cts_command_validate))
				{
					GenericValueTypeMessage<unsigned int> kickSource("TransferKickConnectedClients", i->second.getSourceStationId());
					GenericValueTypeMessage<unsigned int> kickDestination("TransferKickConnectedClients", i->second.getDestinationStationId());
					CentralServer::getInstance().sendToAllLoginServers(kickSource);
					CentralServer::getInstance().sendToAllLoginServers(kickDestination);
					CentralServer::getInstance().sendToAllConnectionServers(kickSource, true);
					CentralServer::getInstance().sendToAllConnectionServers(kickDestination, true);
				}
			}
			break;
		}
		case constcrc("TransferReplyCharacterList") :
		{
			// forward the message to the TransferServer
			TransferReplyCharacterList const reply(ri);
			CentralServer::getInstance().sendToTransferServer(reply);
			break;		
		}
		case constcrc("TransferRenameCharacterReplyFromDatabase") :
		{
			GenericValueTypeMessage<TransferCharacterData> const renameReplyFromDatabase(ri);
			LOG("CustomerService", ("CharacterTransfer: Received TransferRenameCharacterReplyFromDatabase %s", renameReplyFromDatabase.getValue().toString().c_str()));
			if (renameReplyFromDatabase.getValue().getIsValidName())
			{
				// forward to LoginServer
				LOG("CustomerService", ("CharacterTransfer: Sending TransferRenameCharacterInLoginDatabase request to Login Database"));
				GenericValueTypeMessage<TransferCharacterData> const request("TransferRenameCharacterInLoginDatabase", renameReplyFromDatabase.getValue());
				CentralServer::getInstance().sendToArbitraryLoginServer(request);
			}
			else
			{
				// send failure back to TransferServer
				LOG("CustomerService", ("CharacterTransfer: Character rename request failed in the game database. %s", renameReplyFromDatabase.getValue().toString().c_str()));
				GenericValueTypeMessage<TransferCharacterData> const transferRenameCharacterReply("TransferRenameCharacterReply", renameReplyFromDatabase.getValue());
			}
			break;
		}
		case constcrc("LocateObject") :
		{
			GenericValueTypeMessage<std::pair<uint32, std::pair<NetworkId, NetworkId> > > const msg(ri);
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess(msg, true);
			break;
		}
		case constcrc("LocateObjectByTemplateName") :
		{
			GenericValueTypeMessage<std::pair<uint32, std::pair<uint32, NetworkId> > > const msg(ri);
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess(msg, true);
			break;
		}
		case constcrc("LocateObjectResponseMessage") :
		{
			LocateObjectResponseMessage const msg(ri);
			CentralServer::getInstance().sendToGameServer(msg.getResponsePid(), msg, true);
			break;
		}
		case constcrc("LocatePlayerByPartialName") :
		{
			GenericValueTypeMessage<std::pair<uint32, std::pair<std::string, NetworkId> > > const msg(ri);
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess(msg, true);
			break;
		}
		case constcrc("LocateWarden") :
		{
			GenericValueTypeMessage<std::pair<uint32, NetworkId> > const msg(ri);
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess(msg, true);
			break;
		}
		case constcrc("LocateCreatureByCreatureName") :
		{
			GenericValueTypeMessage<std::pair<uint32, std::pair<std::string, NetworkId> > > const msg(ri);
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess(msg, true);
			break;
		}
		case constcrc("LocatePlayerResponseMessage") :
		{
			LocatePlayerResponseMessage const msg(ri);
			CentralServer::getInstance().sendToGameServer(msg.getResponsePid(), msg, true);
			break;
		}
		case constcrc("LSBOIReq") :
		{
			GenericValueTypeMessage<std::pair<std::pair<uint32, bool>, std::pair<std::pair<NetworkId, NetworkId>, NetworkId> > > const locateStructureByOwnerIdReq(ri);
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess(locateStructureByOwnerIdReq, true);
			break;
		}
		case constcrc("LSBOIRsp") :
		{
			GenericValueTypeMessage<std::pair<std::pair<uint32, NetworkId>, std::vector<std::string> > > const locateStructureByOwnerIdRsp(ri);
			CentralServer::getInstance().sendToGameServer(locateStructureByOwnerIdRsp.getValue().first.first, locateStructureByOwnerIdRsp, true);
			break;
		}
		case constcrc("ReportSystemClockTime") :
		{
			GenericValueTypeMessage<std::pair<uint32, NetworkId> > const msg(ri);
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess(msg, true);
			break;
		}
		case constcrc("ReportSystemClockTimeResponse") :
		{
			GenericValueTypeMessage<std::pair<uint32, std::pair<std::string, NetworkId> > > const msg(ri);
			CentralServer::getInstance().sendToGameServer(msg.getValue().first, msg, true);
			break;
		}
		case constcrc("ReportPlanetaryTime") :
		{
			GenericValueTypeMessage<std::pair<uint32, NetworkId> > const msg(ri);
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess(msg, true);
			break;
		}
		case constcrc("ReportPlanetaryTimeResponse") :
		{
			GenericValueTypeMessage<std::pair<uint32, std::pair<std::string, NetworkId> > > const msg(ri);
			CentralServer::getInstance().sendToGameServer(msg.getValue().first, msg, true);
			break;
		}
		case constcrc("TeleportToMessage") :
		{
			TeleportToMessage const msg(ri);
			CentralServer::getInstance().sendToAllGameServersExceptDBProcess(msg, true);
			break;
		}
		case constcrc("LoginToggleCompletedTutorial") :
		{
			GenericValueTypeMessage< std::pair<unsigned int, bool> > const msg(ri);
			CentralServer::getInstance().sendToArbitraryLoginServer(msg);
			break;
		}
		case constcrc("GameServerCSResponse" ) :
		{		
			GameServerCSResponseMessage const msg( ri );
			CSToolResponse rmsg( msg.getAccountId(),
						 msg.getResponse(),
						 msg.getToolId() );
			CentralServer::getInstance().sendToLoginServer(  msg.getLoginServerID(), rmsg );
			break;
		}
		case constcrc("GetCharactersForAccountCSReplyMsg" ) :
		{
			GetCharactersForAccountCSReplyMsg const msg( ri );
			// send repsonse to tool.
			char buf[ 256 ];
			snprintf( buf, 255, "Character search for %lu\r\n",  msg.getTargetAccountId() );
			std::string response = buf;
			response = response + msg.getResponse();
			
			CSToolResponse rmsg( msg.getAccountId(),
						 response,
						 msg.getToolId() );
			CentralServer::getInstance().sendToLoginServer( msg.getLoginServerId(), rmsg );
			break;
		}
		case constcrc("GetDeletedItemsReplyMessage" ) :
		{
			GetDeletedItemsReplyMessage const msg( ri );
			char buf[ 256 ];
			snprintf( buf, 255, "Deleted items for %s\r\n", msg.getCharacterId().getValueString().c_str() );
			std::string response = buf;
			response = response + msg.getResponse();
			CSToolResponse rmsg( msg.getAccountId(),
						 response,
						 msg.getToolId() );
			CentralServer::getInstance().sendToLoginServer( msg.getLoginServerId(), rmsg );
			break;
			
		}
		case constcrc("GetCharacterIdReplyMessage" ) :
		{
			GetCharacterIdReplyMessage const msg( ri );
			std::string response = "Character id for ";
			response = response + msg.getCharacterName() + "\r\n";
			response = response + "character id:" + msg.getCharacterId().getValueString() + "\r\n";
			char buf[ 64 ];
			
			snprintf( buf, 64, "%lu", msg.getAccountId() );
			
			response = response + "account id:" + buf + "\r\n";
			
			CSToolResponse rmsg( msg.getAccountId(), response, msg.getToolId() );
			CentralServer::getInstance().sendToLoginServer( msg.getLoginServerId(), rmsg );
			break;
			
		}
		case constcrc("CSFindAuthObjectReply" ) :
		{
			GenericValueTypeMessage< std::pair< unsigned int, bool > > const msg( ri );
			CentralCSHandler::getInstance().handleFindObjectResponse( msg.getValue().first, msg.getValue().second );
			break;
		}
		case constcrc("StructureListMessage") :
		{
			StructureListMessage msg(ri);
			CentralCSHandler::getInstance().handleStructureListResponse(msg);
			break;
		}
		case constcrc("SPCharacterProfileMessage") :
		{
			SPCharacterProfileMessage const msg(ri);
			CentralServer::getInstance().sendToStationPlayersCollector(msg);
			break;
		}
		case constcrc("OccupyUnlockedSlotReq") :
		{
			GenericValueTypeMessage<std::pair<std::pair<uint32, NetworkId>, uint32> > const occupyUnlockedSlotReq(ri);
			CentralServer::getInstance().sendToArbitraryLoginServer(occupyUnlockedSlotReq);
			break;
		}
		case constcrc("VacateUnlockedSlotReq") :
		{
			GenericValueTypeMessage<std::pair<std::pair<uint32, NetworkId>, uint32> > const vacateUnlockedSlotReq(ri);
			CentralServer::getInstance().sendToArbitraryLoginServer(vacateUnlockedSlotReq);

			// to safeguard against any sort of timing exploit to create another normal
			// slot character while this one is being converted to normal, thus allowing
			// the account to have more normal slot characters on this cluster than is
			// allowed, set the "creating characters too rapidly" flag to prevent any
			// character creation on the account on this cluster for a while
			CharacterCreationTracker::getInstance().setFastCreationLock(vacateUnlockedSlotReq.getValue().first.first);
			break;
		}
		case constcrc("SwapUnlockedSlotReq") :
		{
			GenericValueTypeMessage<std::pair<std::pair<uint32, NetworkId>, std::pair<uint32, NetworkId> > > const swapUnlockedSlotReq(ri);
			CentralServer::getInstance().sendToArbitraryLoginServer(swapUnlockedSlotReq);
			break;
		}
		case constcrc("FeatureIdTransactionRequest") :
		{
			FeatureIdTransactionRequest const fitr(ri);
			CentralServer::getInstance().sendToArbitraryLoginServer(fitr);
			break;
		}
		case constcrc("FeatureIdTransactionSyncUpdate") :
		{
			FeatureIdTransactionSyncUpdate const fitsu(ri);
			CentralServer::getInstance().sendToArbitraryLoginServer(fitsu);
			break;
		}
		case constcrc("AllCluserGlobalChannel") :
		{
			typedef std::pair<std::pair<std::string,std::string>, bool> PayloadType;
			GenericValueTypeMessage<PayloadType> msg(ri);

			PayloadType const & payload = msg.getValue();
			std::string const & channelName = payload.first.first;
			std::string const & messageText = payload.first.second;
			bool const & isRemove = payload.second;

			LOG("CustomerService", ("BroadcastVoiceChannel: CentralServer got AllCluserGlobalChannel from GameServer, sending to LoginServer chan(%s) text(%s) remove(%d)",
				channelName.c_str(), messageText.c_str(), (isRemove?1:0) ));

			CentralServer::getInstance().sendToArbitraryLoginServer(msg);
			break;
		}
		case constcrc("TransferRequestMoveValidation") :
		{
			TransferRequestMoveValidation const request(ri);
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(request));
			LOG("CustomerService", ("CharacterTransfer: Received TransferRequestMoveValidation from GameServer (%u) : [%u, %s, %s (%s)] to [%u, %s, %s]. Forwarding request to LoginServer.", request.getTrack(), request.getSourceStationId(), request.getSourceGalaxy().c_str(), request.getSourceCharacter().c_str(), request.getSourceCharacterId().getValueString().c_str(), request.getDestinationStationId(), request.getDestinationGalaxy().c_str(), request.getDestinationCharacter().c_str()));
			break;
		}
		case constcrc("GcwScoreStatRaw") :
		{
			GenericValueTypeMessage<std::pair<std::string, std::pair<std::map<std::string, std::pair<int64, int64> >, std::map<std::string, std::pair<int64, int64> > > > > const msg(ri);
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(msg));
			break;
		}
		case constcrc("GcwScoreStatPct") :
		{
			GenericValueTypeMessage<std::pair<std::string, std::pair<std::map<std::string, int>, std::map<std::string, int> > > > const msg(ri);
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(msg));
			break;
		}
	}

	ServerConnection::onReceive(message);
}

// ======================================================================
