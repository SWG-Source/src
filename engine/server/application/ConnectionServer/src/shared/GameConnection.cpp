// GameConnection.cpp
// copyright 2001 Verant Interactive

//-----------------------------------------------------------------------

#include "FirstConnectionServer.h"
#include "GameConnection.h"

#include "Archive/ByteStream.h"
#include "Client.h"
#include "ClientConnection.h"
#include "ConfigConnectionServer.h"
#include "ConnectionServer.h"
#include "CustomerServiceConnection.h"
#include "PseudoClientConnection.h"
#include "serverKeyShare/KeyShare.h"
#include "serverNetworkMessages/AccountFeatureIdRequest.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdRequest.h"
#include "serverNetworkMessages/ClaimRewardsMessage.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/NewClient.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "SessionApiClient.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/CommandChannelMessages.h"
#include "sharedNetworkMessages/CreateTicketMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

GameConnection::GameConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData()),
gameServerId(0),
sceneName()
{
}

//-----------------------------------------------------------------------

GameConnection::GameConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t),
gameServerId(0),
sceneName()
{
}

//-----------------------------------------------------------------------

GameConnection::~GameConnection()
{
}

//-----------------------------------------------------------------------

void GameConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("GameConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void GameConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
	static MessageConnectionCallback m("GameConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void GameConnection::onReceive(const Archive::ByteStream & message)
{
	ServerConnection::onReceive(message);
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);
	ri = message.begin();

	if (m.isType("GameClientMessage"))
	{
		//we're receiving a message to forward to the client.
		//it is prefixed with NetworkId and reliable.
		const GameClientMessage msg(ri);
		const std::vector<NetworkId> & v = msg.getDistributionList();
		std::vector<NetworkId>::const_iterator i;
		const bool reliable = msg.getReliable();

		Service *service = ConnectionServer::getClientServicePrivate();
		LogicalPacket const * p = service->createPacket(msg.getByteStream().getBuffer(), static_cast<int>(msg.getByteStream().getSize()));
		for(i = v.begin(); i != v.end(); ++i)
		{
			Client* client = ConnectionServer::getClient((*i));
			if (client)
			{
				client->getClientConnection()->sendSharedPacket(p, reliable);
			}
		}
		service->releasePacket(p);
	}
	else if (m.isType("CreateTicketMessage"))
	{
		Archive::ReadIterator cri(m.getByteStream());
		CreateTicketMessage const c(cri);
		CustomerServiceConnection * const customerServiceConnection =
			const_cast<CustomerServiceConnection *>(ConnectionServer::getCustomerServiceConnection());
		if (customerServiceConnection)
		{
			customerServiceConnection->send(c, true);
		}
	}
	else if(m.isType("ControlAssumed"))
	{
		ControlAssumed ca(ri);

		static const std::string loginTrace("TRACE_LOGIN");
		LOG(loginTrace, ("Received Control Assumed Message from game server %lu for %s skipLoadScreen=%s", getGameServerId(), ca.getNetworkId().getValueString().c_str(), (ca.getSkipLoadScreen() ? "yes" : "no")));
        
		Client *client = ConnectionServer::getClient(ca.getNetworkId());
		if (!client)
		{
			// perhaps it's a transfer client?
			PseudoClientConnection * pseudoClient = PseudoClientConnection::getPseudoClientConnection(ca.getNetworkId());
			if(! pseudoClient)
			{
				DEBUG_REPORT_LOG(true, ("Client %s was already dropped, notifying GameServer.\n", ca.getNetworkId().getValueString().c_str()));
				DropClient const drop(ca.getNetworkId());
				send(drop, true);
				return;
			}
			else
			{
				pseudoClient->controlAssumed();
				return;
			}
		}
		if (!client->getClientConnection())
		{
			WARNING_STRICT_FATAL(true, ("We have a client with no client connection\n"));
			return;
		}

		if (ca.getSkipLoadScreen())
			client->skipLoadScreen();

		if (!client->getSkipLoadScreen())
		{
			CmdStartScene const startScene(
				ca.getNetworkId(),
				ca.getSceneName(),
				ca.getStartPosition(),
				ca.getStartYaw(),
				ca.getTemplateName(),
				ca.getTimeSeconds(),
				static_cast<int32>(::time(nullptr)),
				ConfigConnectionServer::getDisableWorldSnapshot());
			client->getClientConnection()->send(startScene, true);
		}
		client->handleTransfer(ca.getSceneName(), this);

		// record the time when play started for the character
		if (client->getClientConnection()->getStartPlayTime() == 0)
		{
			client->getClientConnection()->setStartPlayTime(::time(nullptr));
		}

		// update the play time info on the game server
		// must be called after client->handleTransfer() so the
		// client object has been updated to point to the correct
		// game server for the client->sendPlayTimeInfoToGameServer()
		// call to work properly
		client->getClientConnection()->sendPlayTimeInfoToGameServer();
	}
	else if (m.isType("ReplyBankCTSLoaded"))
	{
		LOG("CustomerService", ("CharacterTransfer: Game Connection received ReplyBankCTSLoaded message"));
		GenericValueTypeMessage<NetworkId> characterId(ri);

		IGNORE_RETURN(PseudoClientConnection::tryToDeliverMessageTo(characterId.getValue(), message));
	}
	else if (m.isType("PackedHousesLoaded"))
	{
		LOG("CustomerService", ("CharacterTransfer: Game Connection received PackedHousesLoaded message"));
		GenericValueTypeMessage<NetworkId> characterId(ri);

		IGNORE_RETURN(PseudoClientConnection::tryToDeliverMessageTo(characterId.getValue(), message));
	}	
	else if (m.isType("NewGameServer"))
	{
		DEBUG_REPORT_LOG(true, ("Received NewGameServerMessage.\n"));
		// a game server has connected.  Add it to the map
		const NewGameServer newGameServer(ri);
        
		setGameServerId(newGameServer.getServerId());
		setSceneName(newGameServer.getSceneName());
        
		// set the GameServerId @todo
		// add it to the map if a game process
		ConnectionServer::addGameConnection(newGameServer.getServerId(), this);
	}
   
	else if (m.isType("KickPlayer"))
	{
		const KickPlayer kickPlayer(ri);
		Client *client = ConnectionServer::getClient(kickPlayer.getNetworkId());
		if (client)
			client->kick(kickPlayer.getReason());
	}
 
	else if (m.isType("TransferControlMessage"))
	{
		// a game server is giving up authority for an object we control
		const TransferControlMessage transferControl(ri);
        
		DEBUG_REPORT_LOG(true, ("Received TransferControlMessage for %s.\n", transferControl.getNetworkId().getValueString().c_str()));
		//question, do we need to check scene here?
		Client* client = ConnectionServer::getClient(transferControl.getNetworkId());

		if(client)
		{
			if (transferControl.getSkipLoadScreen())
				client->skipLoadScreen();
			if (client->getClientConnection())
			{
				ClientConnection* cconn = client->getClientConnection();
				GameConnection* newConnection = ConnectionServer::getGameConnection(transferControl.getGameServerId());
				if(cconn && newConnection)
				{				
					//We have a client, and a game connection, so send the newClient message off to the game server in question.
					//It will respond with a control assumed message.
					NewClient const newClient(transferControl.getNetworkId(), cconn->getAccountName(), cconn->getRemoteAddress(), cconn->getIsSecure(), transferControl.getSkipLoadScreen(), cconn->getSUID(), &transferControl.getObservedObjects(), cconn->getGameFeatures(), cconn->getSubscriptionFeatures(), cconn->getEntitlementTotalTime(), cconn->getEntitlementEntitledTime(), cconn->getEntitlementTotalTimeSinceLastLogin(), cconn->getEntitlementEntitledTimeSinceLastLogin(), cconn->getBuddyPoints(), cconn->getConsumedRewardEvents(), cconn->getClaimedRewardItems(), cconn->isUsingAdminLogin(), cconn->getCanSkipTutorial());
					newConnection->send(newClient, true);
				}
				else
				{
					WARNING_STRICT_FATAL(true, ("A TransferControl message was received to transfer client %s to GameServer %lu, but that game server is no longer available. The client will be dropped.", transferControl.getNetworkId().getValueString().c_str(), transferControl.getGameServerId()));
					DropClient const drop(transferControl.getNetworkId());
					send(drop, true);
				}
			}
		}
		else
		{
			DEBUG_WARNING(true, ("Received a TransferControllMessage but the client is no longer available. This might be a lost race between the connection server notifiying a game server that the client has disconnected, and the game server notifying the connection server that the client is transferring."));
			DropClient const drop(transferControl.getNetworkId());
			send(drop, true);
		}
	}
	else if(m.isType("ReplyTransferData") || m.isType("ApplyTransferDataSuccess") || m.isType("ApplyTransferDataFail"))
	{
		GenericValueTypeMessage<TransferCharacterData> reply(ri);

		IGNORE_RETURN(PseudoClientConnection::tryToDeliverMessageTo(reply.getValue().getCharacterId(), message));
	}
	else if (m.isType("ChatEnterRoomValidationResponse"))
	{
		GenericValueTypeMessage<std::pair<std::pair<NetworkId, unsigned int>, unsigned int> > const reply(ri);

		Client* client = ConnectionServer::getClient(reply.getValue().first.first);
		if (client && client->getClientConnection())
		{
			client->getClientConnection()->handleChatEnterRoomValidationResponse(reply.getValue().second, reply.getValue().first.second);
		}
	}
	else if (m.isType("ChatQueryRoomValidationResponse"))
	{
		GenericValueTypeMessage<std::pair<std::pair<NetworkId, bool>, unsigned int> > const reply(ri);

		Client* client = ConnectionServer::getClient(reply.getValue().first.first);
		if (client && client->getClientConnection())
		{
			client->getClientConnection()->handleChatQueryRoomValidationResponse(reply.getValue().second, reply.getValue().first.second);
		}
	}
	else if (m.isType("AccountFeatureIdRequest"))
	{
		AccountFeatureIdRequest * const msg = new AccountFeatureIdRequest(ri);

		SessionApiClient * const sessionApiClient = ConnectionServer::getSessionApiClient();
		if (sessionApiClient)
		{
			// SessionApiClient will own (and delete) msg
			sessionApiClient->getFeatures(msg->getTargetStationId(), msg->getGameCode(), msg);
		}
		else
		{
			// a cluster running without session authentication stores
			// account feature id information on the LoginServer, so
			// pass request to the LoginServer (via CentralServer)
			CentralConnection * const cc = ConnectionServer::getCentralConnection();
			if (cc)
				cc->send(*msg, true);

			delete msg;
		}
	}
	else if (m.isType("AdjustAccountFeatureIdRequest"))
	{
		AdjustAccountFeatureIdRequest * const msg = new AdjustAccountFeatureIdRequest(ri);

		SessionApiClient * const sessionApiClient = ConnectionServer::getSessionApiClient();
		if (sessionApiClient)
		{
			// SessionApiClient will own (and delete) msg
			sessionApiClient->getFeatures(msg->getTargetStationId(), msg->getGameCode(), msg);
		}
		else
		{
			// a cluster running without session authentication stores
			// account feature id information on the LoginServer, so
			// pass request to the LoginServer (via CentralServer)
			CentralConnection * const cc = ConnectionServer::getCentralConnection();
			if (cc)
				cc->send(*msg, true);

			delete msg;
		}
	}
	else if (m.isType("ClaimRewardsMessage"))
	{
		ClaimRewardsMessage * const msg = new ClaimRewardsMessage(ri);

		SessionApiClient * const sessionApiClient = ConnectionServer::getSessionApiClient();
		if ((msg->getAccountFeatureId() > 0) && sessionApiClient)
		{
			// SessionApiClient will own (and delete) msg
			sessionApiClient->getFeatures(msg->getStationId(), PlatformGameCode::SWG, msg);
		}
		else
		{
			// a cluster running without session authentication stores
			// account feature id information on the LoginServer, so
			// pass request to the LoginServer (via CentralServer)
			CentralConnection * const cc = ConnectionServer::getCentralConnection();
			if (cc)
				cc->send(*msg, true);

			delete msg;
		}
	}
}

//-----------------------------------------------------------------------
