// ConnectionServerConnection.cpp
// copyright 2001 Verant Interactive



//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "ConnectionServerConnection.h"

#include "serverNetworkMessages/AccountFeatureIdRequest.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdRequest.h"
#include "serverNetworkMessages/CentralConnectionServerMessages.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/EnumerateServers.h"
#include "serverNetworkMessages/LoginConnectionServerAddress.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "serverNetworkMessages/ToggleAvatarLoginStatus.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/UpdateConnectionServerStatus.h"
#include "serverNetworkMessages/UpdateLoginConnectionServerStatus.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

//-----------------------------------------------------------------------

namespace ConnectionServerConnectionNamespace
{
	std::map<unsigned int, std::pair<TransferRequestMoveValidation::TransferRequestSource, ConnectionServerConnection *> > s_pseudoClientConnectionMap;
}

using namespace ConnectionServerConnectionNamespace;

int ConnectionServerConnection::ms_connectionServerNumber = 0;
struct OnConnectionServerConnectionClosed {};

//-----------------------------------------------------------------------

ConnectionServerConnection::ConnectionServerConnection(const std::string & a, const uint16 p) :
ServerConnection            (a, p, NetworkSetupData()),
m_chatServicePort           (0),
m_csServicePort             (0),
m_clientServicePortPrivate  (0),
m_clientServicePortPublic   (0),
m_gameServicePort           (0),
m_id                        (0),
m_pingPort                  (0),
m_connectionServerNumber    (0),
m_gameServiceAddress        (),
m_playerCount               (0),
m_freeTrialCount            (0),
m_emptySceneCount           (0),
m_tutorialSceneCount        (0),
m_falconSceneCount          (0),
m_clientServiceAddress      (),
m_chatServiceAddress        (),
m_customerServiceAddress    ()
{
}

//-----------------------------------------------------------------------

ConnectionServerConnection::ConnectionServerConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection            (u, t),
m_chatServicePort           (0),
m_csServicePort             (0),
m_clientServicePortPrivate  (0),
m_clientServicePortPublic   (0),
m_gameServicePort           (0),
m_id                        (0),
m_pingPort                  (0),
m_connectionServerNumber    (0),
m_gameServiceAddress        (),
m_playerCount               (0),
m_freeTrialCount            (0),
m_emptySceneCount           (0),
m_tutorialSceneCount        (0),
m_falconSceneCount          (0),
m_clientServiceAddress      (),
m_chatServiceAddress        (),
m_customerServiceAddress    ()
{
}

//-----------------------------------------------------------------------

ConnectionServerConnection::~ConnectionServerConnection()
{
	// remove ConnectionServerConnection *'s from the 
	// s_pseudoClientConnectionMap
	std::map<unsigned int, std::pair<TransferRequestMoveValidation::TransferRequestSource, ConnectionServerConnection *> >::iterator i;
	for(i = s_pseudoClientConnectionMap.begin(); i != s_pseudoClientConnectionMap.end();)
	{
		if(i->second.second == this)
		{
			if (i->second.first == TransferRequestMoveValidation::TRS_transfer_server)
			{
				GenericValueTypeMessage<unsigned int> failed("TransferFailConnectionServerClosedConnectionWithCentralServer", i->first);
				CentralServer::getInstance().sendToTransferServer(failed);
			}
			else
			{
				LOG("CustomerService", ("CharacterTransfer: Transfer failed: ConnectionServer connection closed with CentralServer while transfer was in progress for station id %u", i->first));
			}

			s_pseudoClientConnectionMap.erase(i++);
		}
		else
		{
			++i;
		}
	}
}

//-----------------------------------------------------------------------

bool ConnectionServerConnection::sendToPseudoClientConnection(unsigned int stationId, const GameNetworkMessage & message)
{
	bool result = false;
	std::map<unsigned int, std::pair<TransferRequestMoveValidation::TransferRequestSource, ConnectionServerConnection *> >::iterator f = s_pseudoClientConnectionMap.find(stationId);
	if(f != s_pseudoClientConnectionMap.end())
	{
		result = true;
		f->second.second->send(message, true);
	}
	return result;
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onConnectionClosed()
{
	static MessageDispatch::Transceiver<const OnConnectionServerConnectionClosed &> t;
	static OnConnectionServerConnectionClosed c;
	t.emitMessage(c);//lint !e738 // I want the default initialization

	ServerConnection::onConnectionClosed();
	CentralServer::getInstance().removeConnectionServerConnection(this);
	static MessageConnectionCallback m("ConnectionServerConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();

	m_id = ++ms_connectionServerNumber;
	ConnectionServerId message(m_id);

	send(message, true);
	// push encryption keys to the connection server
	CentralServer::getInstance().pushAllKeys(this);

	static MessageConnectionCallback m("ConnectionServerConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);
	ri = message.begin(); 

	if(m.isType("NewCentralConnectionServer"))
	{
		const NewCentralConnectionServer ncs(ri);

		m_chatServicePort = ncs.getChatServicePort();
		m_csServicePort = ncs.getCustomerServicePort();

		m_clientServicePortPrivate = ncs.getClientServicePortPrivate();
		m_clientServicePortPublic = ncs.getClientServicePortPublic();
		m_gameServicePort = ncs.getGameServicePort();
		m_pingPort = ncs.getPingPort ();
		m_connectionServerNumber = ncs.getConnectionServerNumber();
		m_gameServiceAddress = ncs.getGameServiceAddress();
		m_clientServiceAddress = ncs.getClientServiceAddress();
		m_chatServiceAddress = ncs.getChatServiceAddress();
		m_customerServiceAddress = ncs.getCustomerServiceAddress();

		CentralServer::getInstance().addConnectionServerConnection(this);

		//Now broadcast to gameservers
		ConnectionServerAddress const addressMessage(m_gameServiceAddress, m_gameServicePort);
		CentralServer::getInstance().broadcastToGameServers(addressMessage);

		//Send to chat server
		const int ct = static_cast<int>(EnumerateServers::CONNECTION_SERVER);
		EnumerateServers e(true, getChatServiceAddress(), getChatServicePort(), ct);

		if (!getChatServiceAddress().empty() && (getChatServicePort() != 0))
		{
			CentralServer::getInstance().broadcastToChatServers(e);
		}
		else
		{
			LOG("ChatServ", ("ConnectionServerConnection::onReceive() Chat service address and port are invalid, unable to CentralServer::getInstance().broadcastToChatServers()"));
		}

		//Send to CS servers
		const EnumerateServers e2(true, getCustomerServiceAddress(), getCustomerServicePort(), ct);

		if (   !getCustomerServiceAddress().empty()
		    && (getCustomerServicePort() != 0))
		{
			CentralServer::getInstance().broadcastToCustomerServiceServers(e2);
		}
		else
		{
			LOG("ChatServ", ("ConnectionServerConnection::onReceive() Customer service address and port are invalid, unable to CentralServer::getInstance().broadcastToCustomerServiceServers()"));
		}

		//Send to login Servers
		if ( (getClientServicePortPrivate() != 0) || (getClientServicePortPublic() != 0) )
		{
			const LoginConnectionServerAddress csa(m_id, getClientServiceAddress(), getClientServicePortPrivate(),
				getClientServicePortPublic(), getPlayerCount(), getPingPort ());
			CentralServer::getInstance().sendToAllLoginServers(csa);
		}
	}
	else if (m.isType("UpdateConnectionServerStatus"))
	{
		DEBUG_REPORT_LOG(true, ("Got update status for connection server\n"));
		const UpdateConnectionServerStatus ucs(ri);
		m_clientServicePortPublic = ucs.getPublicPort();
		m_clientServicePortPrivate = ucs.getPrivatePort();
		UpdateLoginConnectionServerStatus ulc(getId(), m_clientServicePortPublic, m_clientServicePortPrivate, getPlayerCount());
		CentralServer::getInstance().sendToAllLoginServers(ulc);
	}

	else if(m.isType("TaskSpawnProcess"))
	{
		const TaskSpawnProcess spawn(ri);
		CentralServer::getInstance().sendTaskMessage(spawn);
	}
	else if(m.isType("NewPseudoClientConnection"))
	{
		const GenericValueTypeMessage<std::pair<unsigned int, int8> > info(ri);
		s_pseudoClientConnectionMap[info.getValue().first] = std::make_pair(static_cast<TransferRequestMoveValidation::TransferRequestSource>(info.getValue().second), this);

		// remove corresponding "non pseudo client connection"
		CentralServer::getInstance().removeFromAccountConnectionMap(static_cast<StationId>(info.getValue().first));
	}
	else if(m.isType("DestroyPseudoClientConnection"))
	{
		const GenericValueTypeMessage<unsigned int> info(ri);
		std::map<unsigned int, std::pair<TransferRequestMoveValidation::TransferRequestSource, ConnectionServerConnection *> >::iterator f = s_pseudoClientConnectionMap.find(info.getValue());
		if(f != s_pseudoClientConnectionMap.end())
		{
			s_pseudoClientConnectionMap.erase(f);
		}
	}
	else if(m.isType("TransferReceiveDataFromGameServer"))
	{
		const GenericValueTypeMessage<TransferCharacterData> transferReply(ri);

		if(transferReply.getValue().getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
		{
			CentralServer::getInstance().sendToTransferServer(transferReply);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: Got TransferReceiveDataFromGameServer: %s", transferReply.getValue().toString().c_str()));

			// send character to ConnectionServer for creation on the destination server
			GenericValueTypeMessage<TransferCharacterData> login("TransferLoginCharacterToDestinationServer", transferReply.getValue());
			CentralServer::getInstance().sendToArbitraryLoginServer(login);
			LOG("CustomerService", ("CharacterTransfer: Sending TransferLoginCharacterToDestinationServer to CentralServer (via LoginServer) (%s) for (%s)", transferReply.getValue().getDestinationGalaxy().c_str(), login.getValue().toString().c_str()));
		}
	}
	else if(m.isType("ApplyTransferDataSuccess"))
	{
		const GenericValueTypeMessage<TransferCharacterData> success(ri);

		// send message to source galaxy (via LoginServer) to mark
		// the source character as having been transferred to prevent
		// all future login of the character
		{
			const GenericValueTypeMessage<std::pair<std::string, NetworkId> > msg("CtsCompletedForcharacter", std::make_pair(success.getValue().getSourceGalaxy(), success.getValue().getCharacterId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(msg));
		}

		// send the message back to the transfer server, which then
		// sends a disable login request to the source central server,
		// "removing" the account on the source galaxy.
		if(success.getValue().getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
		{
			CentralServer::getInstance().sendToTransferServer(success);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: Received ApplyTransferDataSuccess from ConnectionServer. %s", success.getValue().toString().c_str()));
			LOG("CustomerService", ("CharacterTransfer: Sending ToggleAvatarLoginStatus request to LoginServer"));

			const ToggleAvatarLoginStatus toggleLoginStatus(success.getValue().getSourceGalaxy(), success.getValue().getSourceStationId(), success.getValue().getCharacterId(), false);
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(toggleLoginStatus));
		}
	}
	else if(m.isType("ApplyTransferDataFail"))
	{
		const GenericValueTypeMessage<TransferCharacterData> fail(ri);
		
		// send the message back to the transfer server, which then
		// sends a delete request to the destination central server,
		// "removing" the account on the destination galaxy.
		if(fail.getValue().getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
		{
			CentralServer::getInstance().sendToTransferServer(fail);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: Received ApplyTransferDataFail from ConnectionServer. %s", fail.getValue().toString().c_str()));
			LOG("CustomerService", ("CharacterTransfer: failed to apply transfer data. Deleting character on destination server. %s", fail.getValue().toString().c_str()));

			GenericValueTypeMessage<TransferCharacterData> const deleteCharacter("DeleteFailedTransfer", fail.getValue());
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(deleteCharacter));

			GenericValueTypeMessage<std::pair<std::string, unsigned int> > const closeRequestSource("RequestTransferClosePseudoClientConnection", std::make_pair(fail.getValue().getSourceGalaxy(), fail.getValue().getSourceStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(closeRequestSource));

			GenericValueTypeMessage<std::pair<std::string, unsigned int> > const closeRequestTarget("RequestTransferClosePseudoClientConnection", std::make_pair(fail.getValue().getDestinationGalaxy(), fail.getValue().getDestinationStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(closeRequestTarget));
		}
	}
	else if(m.isType("TransferCreateCharacterFailed"))
	{
		const GenericValueTypeMessage<TransferCharacterData> fail(ri);

		if(fail.getValue().getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
		{
			CentralServer::getInstance().sendToTransferServer(fail);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: Received TransferCreateCharacterFailed. %s", fail.getValue().toString().c_str()));

			GenericValueTypeMessage<std::pair<std::string, unsigned int> > const closeRequestSource("RequestTransferClosePseudoClientConnection", std::make_pair(fail.getValue().getSourceGalaxy(), fail.getValue().getSourceStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(closeRequestSource));

			GenericValueTypeMessage<std::pair<std::string, unsigned int> > const closeRequestTarget("RequestTransferClosePseudoClientConnection", std::make_pair(fail.getValue().getDestinationGalaxy(), fail.getValue().getDestinationStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(closeRequestTarget));
		}
	}
	else if(m.isType("ReplyTransferDataFail"))
	{
		const GenericValueTypeMessage<TransferCharacterData> reply(ri);

		if(reply.getValue().getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
		{
			CentralServer::getInstance().sendToTransferServer(reply);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: Transfer failed: Failed to retrieve character data. %s", reply.getValue().toString().c_str()));

			GenericValueTypeMessage<std::pair<std::string, unsigned int> > const closeRequestSource("RequestTransferClosePseudoClientConnection", std::make_pair(reply.getValue().getSourceGalaxy(), reply.getValue().getSourceStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(closeRequestSource));

			GenericValueTypeMessage<std::pair<std::string, unsigned int> > const closeRequestTarget("RequestTransferClosePseudoClientConnection", std::make_pair(reply.getValue().getDestinationGalaxy(), reply.getValue().getDestinationStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(closeRequestTarget));
		}
	}
	else if(m.isType("TransferFailGameServerClosedConnectionWithConnectionServer"))
	{
		const GenericValueTypeMessage<TransferCharacterData> fail(ri);

		if(fail.getValue().getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
		{
			CentralServer::getInstance().sendToTransferServer(fail);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: Transfer failed: GameServer connection closed while transfer was in progress. %s", fail.getValue().toString().c_str()));

			GenericValueTypeMessage<std::pair<std::string, unsigned int> > const closeRequestSource("RequestTransferClosePseudoClientConnection", std::make_pair(fail.getValue().getSourceGalaxy(), fail.getValue().getSourceStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(closeRequestSource));

			GenericValueTypeMessage<std::pair<std::string, unsigned int> > const closeRequestTarget("RequestTransferClosePseudoClientConnection", std::make_pair(fail.getValue().getDestinationGalaxy(), fail.getValue().getDestinationStationId()));
			IGNORE_RETURN(CentralServer::getInstance().sendToArbitraryLoginServer(closeRequestTarget));
		}
	}
	else if(m.isType("AccountFeatureIdRequest"))
	{
		const AccountFeatureIdRequest msg(ri);
		CentralServer::getInstance().sendToArbitraryLoginServer(msg);
	}
	else if(m.isType("AdjustAccountFeatureIdRequest"))
	{
		const AdjustAccountFeatureIdRequest msg(ri);
		CentralServer::getInstance().sendToArbitraryLoginServer(msg);
	}
	else
	{
		ServerConnection::onReceive(message);
	}
}

//-----------------------------------------------------------------------

ConnectionServerConnection * ConnectionServerConnection::getConnectionForAccount(unsigned int stationId)
{
	ConnectionServerConnection * result = 0;
	std::map<unsigned int, std::pair<TransferRequestMoveValidation::TransferRequestSource, ConnectionServerConnection *> >::iterator f = s_pseudoClientConnectionMap.find(stationId);
	if(f != s_pseudoClientConnectionMap.end())
	{
		result = f->second.second;
	}
	return result;
}

//-----------------------------------------------------------------------

void ConnectionServerConnection::removeFromAccountConnectionMap(unsigned int stationId)
{
	std::map<unsigned int, std::pair<TransferRequestMoveValidation::TransferRequestSource, ConnectionServerConnection *> >::iterator f = s_pseudoClientConnectionMap.find(stationId);
	if (f != s_pseudoClientConnectionMap.end())
	{
		s_pseudoClientConnectionMap.erase(f);
	}
}
