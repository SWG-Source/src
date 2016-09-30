// PseudoClientConnection.cpp
// copyright 2001 Verant Interactive

//-----------------------------------------------------------------------

#include "FirstConnectionServer.h"
#include "PseudoClientConnection.h"

#include "CentralConnection.h"
#include "ConnectionServer.h"
#include "UnicodeUtils.h"
#include "serverNetworkMessages/CentralConnectionServerMessages.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/GameServerForLoginMessage.h"
#include "serverNetworkMessages/NewClient.h"
#include "serverNetworkMessages/RequestGameServerForLoginMessage.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "sharedLog/Log.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedUtility/StartingLocationData.h"
#include "sharedUtility/StartingLocationManager.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

namespace PseudoClientConnectionNamespace
{
	std::map<unsigned int, PseudoClientConnection *> s_pseudoClientConnectionMap;
	std::map<NetworkId, PseudoClientConnection *> s_pseudoClientConnectionMapByCharacterId;
}

using namespace PseudoClientConnectionNamespace;

//-----------------------------------------------------------------------
/*
	Constructed by way of CentralServerConnection to create a psueudo-client.
	The CentralServer sends TransferCharacterData when it receives a 
	TransferLoginRequest from the TransferServer
*/
PseudoClientConnection::PseudoClientConnection(const TransferCharacterData & transferDataFromCentralServer, unsigned int stationId) :
m_transferCharacterData(transferDataFromCentralServer),
m_gameConnection(0),
m_trackStationId(stationId)
{
	REPORT_LOG(true, ("ConnectionServer: creating a PseudoClientConnection from transferCharacterData: %s\n", transferDataFromCentralServer.toString().c_str()));
								    
	// tell CentralServer about this PseudoClientConnection
	GenericValueTypeMessage<std::pair<unsigned int, int8> > info("NewPseudoClientConnection", std::make_pair(m_trackStationId, static_cast<int8>(m_transferCharacterData.getTransferRequestSource())));

	CentralConnection * centralServerConnection = ConnectionServer::getCentralConnection();
	if(centralServerConnection)
	{
		centralServerConnection->send(info, true);
	}

	s_pseudoClientConnectionMap[m_trackStationId] = this;
	s_pseudoClientConnectionMapByCharacterId[m_transferCharacterData.getCharacterId()] = this;
}

//-----------------------------------------------------------------------

PseudoClientConnection::~PseudoClientConnection()
{
	// tell CentralServer that this PseudoClientConnection is no longer relevant
	GenericValueTypeMessage<unsigned int> info("DestroyPseudoClientConnection", m_trackStationId);

	CentralConnection * centralServerConnection = ConnectionServer::getCentralConnection();
	if(centralServerConnection)
	{
		centralServerConnection->send(info, true);
	}

	std::map<unsigned int, PseudoClientConnection *>::iterator f = s_pseudoClientConnectionMap.find(m_trackStationId);
	if(f != s_pseudoClientConnectionMap.end())
	{
		s_pseudoClientConnectionMap.erase(f);
	}

	std::map<NetworkId, PseudoClientConnection *>::iterator cf = s_pseudoClientConnectionMapByCharacterId.find(m_transferCharacterData.getCharacterId());
	if(cf !=s_pseudoClientConnectionMapByCharacterId.end())
	{
		s_pseudoClientConnectionMapByCharacterId.erase(cf);
	}

	cf = s_pseudoClientConnectionMapByCharacterId.find(m_transferCharacterData.getDestinationCharacterId());
	if(cf !=s_pseudoClientConnectionMapByCharacterId.end())
	{
		s_pseudoClientConnectionMapByCharacterId.erase(cf);
	}

	m_gameConnection = 0;
}

//-----------------------------------------------------------------------

const TransferCharacterData & PseudoClientConnection::getTransferCharacterData() const
{
	return m_transferCharacterData;
}

//-----------------------------------------------------------------------

void PseudoClientConnection::controlAssumed()
{
	if(m_transferCharacterData.getCSToolId() > 0)
	{
		LOG("CustomerService", ("CharacterTransfer: Received ControlAssumedMessage from GameServer for CS Tool login character request (or from \"remote object loginCharacter\" console command).  The request has been completed and the character is now logged in."));

		// disconnect them, we logged them in for the CS Tool.
		DropClient dropMsg(m_transferCharacterData.getCharacterId());
		if(m_gameConnection)
		{
			dropMsg.setImmediate(true);
			m_gameConnection->send(dropMsg, true);
		}
		delete this;
		return;
	}

	LOG("CustomerService", ("CharacterTransfer: Received ControlAssumedMessage from GameServer, ready to request bank to be loaded (if source character) or to apply transfer data (if target character)!"));

	if(m_gameConnection)
	{
		if(m_transferCharacterData.getScriptDictionaryData().size())
		{
			LOG("CustomerService", ("CharacterTransfer: controlAssumed, already have transfer data, sending ApplyTransferData"));
			// already have the transfer data, this character is headed to the destination galaxy
			GenericValueTypeMessage<TransferCharacterData> applyTransferData("ApplyTransferData", m_transferCharacterData);
			m_gameConnection->send(applyTransferData, true);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: controlAssumed, sending RequestLoadCTSBank message"));
			GenericValueTypeMessage<NetworkId> requestTransferData("RequestLoadCTSBank", m_transferCharacterData.getCharacterId());
			m_gameConnection->send(requestTransferData, true);
		}
	}
}

//-----------------------------------------------------------------------

void PseudoClientConnection::onBankLoaded()
{
	LOG("CustomerService", ("CharacterTransfer: Received BankLoaded from GameServer, ready to request transfer data!"));

	if(m_gameConnection)
	{
		if(!m_transferCharacterData.getScriptDictionaryData().size())
		{
			GenericValueTypeMessage<NetworkId> requestTransferData("RequestLoadPackedHouses", m_transferCharacterData.getCharacterId());
			m_gameConnection->send(requestTransferData, true);
		}
		else
			LOG("CustomerService", ("CharacterTransfer: OnBankLoaded, RequestTransferData already has data"));
	}
}

//-----------------------------------------------------------------------

void PseudoClientConnection::onPackedHousesLoaded()
{
	LOG("CustomerService", ("CharacterTransfer: Received PackedHousesLoadedMessage from GameServer, ready to request transfer data!"));

	if(m_gameConnection)
	{
		if(!m_transferCharacterData.getScriptDictionaryData().size())
		{
			GenericValueTypeMessage<TransferCharacterData> requestTransferData("RequestTransferData", m_transferCharacterData);
			m_gameConnection->send(requestTransferData, true);
		}
		else
			LOG("CustomerService", ("CharacterTransfer: onPackedHousesLoaded, RequestTransferData already has data"));
	}
}
//-----------------------------------------------------------------------

PseudoClientConnection * PseudoClientConnection::getPseudoClientConnection(const NetworkId & characterId)
{
	PseudoClientConnection * result = 0;
	std::map<NetworkId, PseudoClientConnection *>::iterator f = s_pseudoClientConnectionMapByCharacterId.find(characterId);
	if(f != s_pseudoClientConnectionMapByCharacterId.end())
	{
		result = f->second;
	}
	return result;
}

//-----------------------------------------------------------------------

PseudoClientConnection * PseudoClientConnection::getPseudoClientConnection  (unsigned int stationId)
{
	PseudoClientConnection * result = 0;
	std::map<unsigned int, PseudoClientConnection *>::iterator f = s_pseudoClientConnectionMap.find(stationId);
	if(f != s_pseudoClientConnectionMap.end())
	{
		result = f->second;
	}
	return result;
}

//-----------------------------------------------------------------------
/**
	Find a game server to connect the pseudo client using the source
	station id indicated by m_transferCharacterData.m_sourceStationId
*/
void PseudoClientConnection::requestGameServerForLogin() const
{
	NetworkId characterId = m_transferCharacterData.getCharacterId();
	unsigned int stationId = m_trackStationId;
	NetworkId containerId = m_transferCharacterData.getContainerId();

	if(m_transferCharacterData.getScriptDictionaryData().size())
	{
		characterId = m_transferCharacterData.getDestinationCharacterId();
		containerId = NetworkId::cms_invalid;
	}

	RequestGameServerForLoginMessage requestmsg(stationId, characterId, containerId, m_transferCharacterData.getScene(), m_transferCharacterData.getStartingCoordinates(), (m_transferCharacterData.getScriptDictionaryData().empty() && (m_transferCharacterData.getCSToolId() == 0)));
	if(ConnectionServer::getCentralConnection())
	{
		ConnectionServer::getCentralConnection()->send(requestmsg, true);
		LOG("CustomerService", ("CharacterTransfer: ***ConnectionServer: sending RequestGameServerForLoginMessage(%d, %s, %s, %s)\n", m_transferCharacterData.getSourceStationId(), m_transferCharacterData.getCharacterId().getValueString().c_str(), m_transferCharacterData.getContainerId().getValueString().c_str(), m_transferCharacterData.getScene().c_str()));
	}
}

//-----------------------------------------------------------------------

void PseudoClientConnection::receiveMessage(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	const GameNetworkMessage msg(ri);
	ri = message.begin();

	const uint32 messageType = msg.getType();
	
	switch(messageType) {
		case constcrc("GameServerForLoginMessage") :
		{
			const GameServerForLoginMessage gameServerForLogin(ri);
			LOG("CustomerService", ("CharacterTransfer: *** ConnectionServer: Received GameServerForLoginMessage for %d, server=%d\n", gameServerForLogin.getStationId(), gameServerForLogin.getServer()));
			m_gameConnection = ConnectionServer::getGameConnection(gameServerForLogin.getServer());
			if(m_gameConnection)
			{
				NetworkId characterId = m_transferCharacterData.getCharacterId();
				unsigned int stationId = m_trackStationId;
				if(m_transferCharacterData.getScriptDictionaryData().size())
				{
					LOG("CustomerService", ("CharacterTransfer: Setting up login for destination server"));
					characterId = m_transferCharacterData.getDestinationCharacterId();
				}
				std::vector<std::pair<NetworkId, std::string> > static const emptyStringVector;
				NewClient m(characterId, "TransferServer", NetworkHandler::getHostName(), true, false, stationId, nullptr, 0, 0, 0, 0, 0, 0, 0, emptyStringVector, emptyStringVector, m_transferCharacterData.getCSToolId() != 0, true);
				m_gameConnection->send(m, true);
				LOG("CustomerService", ("CharacterTransfer: Sent NewClient(%s, \"TransferServer\", \"%s\", true, false, %d, nullptr, 0, 0)\n", characterId.getValueString().c_str(), NetworkHandler::getHostName().c_str(), stationId));
			}
			break;
		}
		case constcrc("TransferLoginCharacterToSourceServer") :
		{
			// this is a message that will create a new pseudoclient connection
			LOG("CustomerService", ("CharacterTransfer: requestGameServerForLogin()"));
			GenericValueTypeMessage<TransferCharacterData> loginRequest(ri);
			const NetworkId & requestCharacterId = loginRequest.getValue().getCharacterId(); 
			if(requestCharacterId == NetworkId::cms_invalid || requestCharacterId != m_transferCharacterData.getCharacterId())
			{
				LOG("CustomerService", ("CharacterTransfer: *** ERROR *** Received a request to login to source server for %s, but a transfer for %s is already in progress. Sending failure.", loginRequest.getValue().toString().c_str(), m_transferCharacterData.toString().c_str()));
				GenericValueTypeMessage<TransferCharacterData> fail("ReplyTransferDataFail", m_transferCharacterData);
				ConnectionServer::sendToCentralProcess(fail);
			}
			else
			{
				requestGameServerForLogin();
			}
			break;
		}
		case constcrc("TransferLoginCharacterToDestinationServer") :
		{
			GenericValueTypeMessage<TransferCharacterData> loginRequest(ri);
			const NetworkId & requestCharacterId = loginRequest.getValue().getCharacterId(); 
			if(requestCharacterId == NetworkId::cms_invalid || requestCharacterId != m_transferCharacterData.getCharacterId())
			{
				LOG("CustomerService", ("CharacterTransfer: *** ERROR *** Received a request to login to destination server for %s, but a transfer for %s is already in progress. Sending failure.", loginRequest.getValue().toString().c_str(), m_transferCharacterData.toString().c_str()));
				GenericValueTypeMessage<TransferCharacterData> fail("ReplyTransferDataFail", m_transferCharacterData);
				ConnectionServer::sendToCentralProcess(fail);
			}
			else
			{
				// find a valid starting location
				std::vector<StartingLocationData> startingLocations;
				StartingLocationData bestLocation;
				startingLocations = StartingLocationManager::getLocations();
				if(! startingLocations.empty())
				{
					bestLocation = *(startingLocations.begin());
				}
				m_transferCharacterData.setScene(bestLocation.planet);
				m_transferCharacterData.setStartingCoordinates(Vector(bestLocation.x, bestLocation.y, bestLocation.z));

				// start character creation process
				ConnectionCreateCharacter connectionCreate(
					m_transferCharacterData.getDestinationStationId(),
					Unicode::narrowToWide(m_transferCharacterData.getDestinationCharacterName()),
					m_transferCharacterData.getObjectTemplateName(),
					m_transferCharacterData.getScaleFactor(),
					bestLocation.name,
					m_transferCharacterData.getCustomizationData(),
					m_transferCharacterData.getHairTemplateName(),
					m_transferCharacterData.getHairAppearanceData(),
					m_transferCharacterData.getProfession(),
					false,
					m_transferCharacterData.getBiography(),
					false,
					m_transferCharacterData.getSkillTemplate(),
					m_transferCharacterData.getWorkingSkill(),
					false,
					true,
					0xFFFFFFFF); // assume all feature bits set, so that character creation will not be blocked by account features
				LOG("CustomerService", ("CharacterTransfer: Sending ConnectionCreateCharacter to CentralServer : %s", m_transferCharacterData.toString().c_str()));
				ConnectionServer::sendToCentralProcess(connectionCreate);
			}
			break;
		}
		case constcrc("CtsSrcCharWrongPlanet") :
		{
			LOG("CustomerService", ("CharacterTransfer: *** ERROR *** Source character is not one of the 10 original ground planets. Sending failure for %s.", m_transferCharacterData.toString().c_str()));
			GenericValueTypeMessage<TransferCharacterData> fail("ReplyTransferDataFail", m_transferCharacterData);
			ConnectionServer::sendToCentralProcess(fail);
			break;
		}
		case constcrc("ReplyTransferData") :
		{
			// the game server has responded with valid transfer information.
			// upload the data to the transfer server (by way of the CentralServer)
			// and disconnect the client since there's nothing more needed to be
			// done here
			GenericValueTypeMessage<TransferCharacterData> reply(ri);
			m_transferCharacterData = reply.getValue();
			LOG("CustomerService", ("CharacterTransfer: Received ReplyTransferData %s", m_transferCharacterData.toString().c_str()));

			CentralConnection * centralServerConnection = ConnectionServer::getCentralConnection();
			if(centralServerConnection)
			{
				const GenericValueTypeMessage<TransferCharacterData> transferReply("TransferReceiveDataFromGameServer", m_transferCharacterData);
				centralServerConnection->send(transferReply, true);
			}

			DropClient dropMsg(m_transferCharacterData.getCharacterId());
			if(m_gameConnection)
			{
				m_gameConnection->send(dropMsg, true);
			}
			delete this;
			break;
		}
		case constcrc("ConnectionCreateCharacterSuccess") :
		{
			const ConnectionCreateCharacterSuccess success(ri);
			// woohoo! character has been created on the server!
			// Log the character back in, run the transfer scripts to
			// apply data in the script dictionary associated with the character
			LOG("CustomerService", ("CharacterTransfer: Received ConnectionCreateCharacterSuccess for %s", m_transferCharacterData.toString().c_str()));
			m_transferCharacterData.setDestinationCharacterId(success.getNetworkId());
			requestGameServerForLogin();
			s_pseudoClientConnectionMapByCharacterId[m_transferCharacterData.getDestinationCharacterId()] = this;
			break;
		}
		case constcrc("ConnectionCreateCharacterFailed") :
		{
			const ConnectionCreateCharacterFailed failed(ri);
			LOG("CustomerService", ("CharacterTransfer: Received ConnectionCreateCharacterFailed [%s, %s] for %s", failed.getErrorMessage().getDebugString().c_str(), failed.getOptionalDetailedErrorMessage().c_str(), m_transferCharacterData.toString().c_str()));
			s_pseudoClientConnectionMapByCharacterId[m_transferCharacterData.getDestinationCharacterId()] = this;
			GenericValueTypeMessage<TransferCharacterData> reply("TransferCreateCharacterFailed", m_transferCharacterData);
			CentralConnection * centralServerConnection = ConnectionServer::getCentralConnection();
			if(centralServerConnection)
			{
				centralServerConnection->send(reply, true);
			}
			break;
		}
		case constcrc("ApplyTransferDataSuccess") :
		{
			GenericValueTypeMessage<TransferCharacterData> success(ri);
			LOG("CustomerService", ("CharacterTransfer: Received ApplyTransferDataSuccess from GameServer! %s", success.getValue().toString().c_str()));
			CentralConnection * centralServerConnection = ConnectionServer::getCentralConnection();
			if(centralServerConnection)
			{
				centralServerConnection->send(success, true);
			}
			break;
		}
		case constcrc("ApplyTransferDataFail") :
		{
			GenericValueTypeMessage<TransferCharacterData> fail(ri);
			LOG("CustomerService", ("CharacterTransfer: Received ApplyTransferDataFail from GameServer! %s", fail.getValue().toString().c_str()));
			CentralConnection * centralServerConnection = ConnectionServer::getCentralConnection();
			if(centralServerConnection)
			{
				centralServerConnection->send(fail, true);
			}
			break;
		}
		case constcrc("ReplyBankCTSLoaded") :
		{
			LOG("CustomerService", ("CharacterTransfer: Received ReplyBankCTSLoaded from GameServer!"));
			onBankLoaded();
			break;
		}
		case constcrc("PackedHousesLoaded") :
		{
			onPackedHousesLoaded();
			break;
		}
	}
}
//-----------------------------------------------------------------------

void PseudoClientConnection::destroyAllPseudoClientConnectionInstances()
{
	std::map<unsigned int, PseudoClientConnection *> instances = s_pseudoClientConnectionMap;
	std::map<unsigned int, PseudoClientConnection *>::iterator i = instances.begin();
	for(; i != instances.end(); ++i)
	{
		PseudoClientConnection * c = i->second;
		delete c;
	}
}

//-----------------------------------------------------------------------

bool PseudoClientConnection::tryToDeliverMessageTo(const NetworkId & characterId, const Archive::ByteStream & msg)
{
	bool result = false;
	std::map<NetworkId, PseudoClientConnection *>::iterator f = s_pseudoClientConnectionMapByCharacterId.find(characterId);
	if(f != s_pseudoClientConnectionMapByCharacterId.end())
	{
		result = true;
		f->second->receiveMessage(msg);
	}
	return result;
}

//-----------------------------------------------------------------------

bool PseudoClientConnection::tryToDeliverMessageTo(unsigned int stationId, const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	const GameNetworkMessage msg(ri);
	ri = message.begin();

	bool result = false;
	const uint32 messageType = msg.getType();
	
	if(messageType == constcrc("TransferLoginCharacterToSourceServer") || messageType == constcrc("TransferLoginCharacterToDestinationServer"))
	{
		// this is a message that will create a new pseudoclient connection
		const GenericValueTypeMessage<TransferCharacterData> login(ri);
		unsigned int trackStationId = login.getValue().getSourceStationId();
		if(msg.isType("TransferLoginCharacterToSourceServer"))
		{
			LOG("CustomerService", ("CharacterTransfer: Creating a PseudoClientConnection on source server for %s", login.getValue().toString().c_str()));
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: Creating a PseudoClientConnection on destination server for %s", login.getValue().toString().c_str()));
			trackStationId = login.getValue().getDestinationStationId();
		}

		// remove old sessions if they exist
		PseudoClientConnection * oldConnection = PseudoClientConnection::getPseudoClientConnection(trackStationId);
		delete oldConnection;

		// setup new pseudoclient connection
		PseudoClientConnection * newConnection = new PseudoClientConnection(login.getValue(), trackStationId);
		newConnection->receiveMessage(message);
	}//lint !e429 // leak // suppressed because this is tracked in the map and it is the responsibility of the sender to send a destroy message or the receiving connection to be destroyed, in this case CentralConnection
	else
	{
		std::map<unsigned int, PseudoClientConnection *>::const_iterator const f = s_pseudoClientConnectionMap.find(stationId);
		if(f != s_pseudoClientConnectionMap.end())
		{
			result = true;
			f->second->receiveMessage(message);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: *** FAILED TO DELIVER MESSAGE TO PseudoClientConnection(%d)!! ***", stationId));
		}
	}

	return result;
}

//-----------------------------------------------------------------------

void PseudoClientConnection::gameConnectionClosed(const GameConnection * gameConnection)
{
	std::map<unsigned int, PseudoClientConnection *>::iterator i;
	for(i = s_pseudoClientConnectionMap.begin(); i != s_pseudoClientConnectionMap.end(); ++i)
	{
		if(i->second->m_gameConnection == gameConnection)
		{
			i->second->gameConnectionClosed();
		}
	}
}

//-----------------------------------------------------------------------

void PseudoClientConnection::gameConnectionClosed()
{
	CentralConnection * centralConnection = ConnectionServer::getCentralConnection();
	if(centralConnection)
	{
		GenericValueTypeMessage<TransferCharacterData> gameServerDown("TransferFailGameServerClosedConnectionWithConnectionServer", m_transferCharacterData);
		centralConnection->send(gameServerDown, true);
	}
}

// ======================================================================
