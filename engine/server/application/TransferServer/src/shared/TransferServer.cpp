// TransferServer.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTransferServer.h"
#include "CentralServerConnection.h"
#include "ConfigTransferServer.h"
#include "ConsoleManager.h"
#include "CTSAPIClient.h"
#include "CTCommon/CTServiceCharacter.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "serverNetworkMessages/TransferAccountDataArchive.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/TransferReplyCharacterList.h"
#include "serverNetworkMessages/TransferReplyMoveValidation.h"
#include "serverNetworkMessages/TransferRequestMoveValidation.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedMath/Vector.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/Service.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedUtility/DataTableManager.h"
#include "TransferServer.h"
#include "CentralServerConnection.h"
#include <cstdio>
#include <string>
#include <sstream>
#include <ostream>
#include "Unicode.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

namespace TransferServerNamespace
{
	Service *                                      s_transferServerService;
	std::map<unsigned int, CharacterTransferData>  s_transferData;
	std::set<std::string>                          s_serversAllowedToUpload;
	std::set<std::string>                          s_serversAllowedToDownload;
	std::set<unsigned int>                         s_authorizedDownloads;
	bool                                           s_done = false;
	CTSAPIClient *                                 s_apiClient = 0;

	// as we receive the source character's template crc that comes back from the LoginServer
	// in the TransferReplyMoveValidation message, save it here, and send it back in subseqnent
	// TransferRequestMoveValidation messages as an optimization so the LoginServer doesn't have
	// to continually query the DB to fetch the same information over and over again, because
	// the character's template crc (i.e. the character's species) should never, ever change
	std::map<std::pair<unsigned int, std::pair<std::string, std::string> >, uint32> s_cacheSourceCharacterTemplateCrc;

	void closePseudoClientConnection(const std::string & galaxyName, const unsigned int stationId)
	{
		CentralServerConnection * centralServerConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(galaxyName);
		if(centralServerConnection)
		{
			GenericValueTypeMessage<unsigned int> closeRequest("TransferClosePseudoClientConnection", stationId);
			centralServerConnection->send(closeRequest, true);
		}
	}

	void closePseudoClientConnections(const std::string & sourceGalaxyName, const unsigned int sourceStationId, const std::string & destinationGalaxyName, const unsigned int destinationStationId)
	{
        // close pseudoclientconnections
		closePseudoClientConnection(sourceGalaxyName, sourceStationId);

		unsigned int destId = destinationStationId;
		if(destId == 0)
			destId = sourceStationId;
		closePseudoClientConnection(destinationGalaxyName, destId);
		
	}
	
	std::string resultToString(CTService::CTMoveResult resultCode)
	{
		std::string resultString;

		switch (resultCode)
		{
			case CTService::CT_GAMERESULT_SUCCESS:
				resultString = "CTService::CT_GAMERESULT_SUCCESS";
				break;
			case CTService::CT_GAMERESULT_SOFTERROR:
				resultString = "CTService::CT_GAMERESULT_SOFTERROR";
				break;
			case CTService::CT_GAMERESULT_HARDERROR:
				resultString = "CTService::CT_GAMERESULT_HARDERROR";
				break;
			case CTService::CT_GAMERESULT_INVALID_NAME:
				resultString = "CTService::CT_GAMERESULT_INVALID_NAME";
				break;
			case CTService::CT_GAMERESULT_NAME_ALREADY_TAKE:
				resultString = "CTService::CT_GAMERESULT_NAME_ALREADY_TAKE";
				break;
			case CTService::CT_GAMERESULT_HAS_CORPSE:
				resultString = "CTService::CT_GAMERESULT_NAME_HAS_CORPSE";
				break;
			case CTService::CT_GAMERESULT_SERVER_IS_DOWN:
				resultString = "CTService::CT_GAMERESULT_SERVER_IS_DOWN";
				break;
			case CTService::CT_GAMERESULT_MAX_CHAR_ON_DEST_SERVER:
				resultString = "CTService::CT_GAMERESULT_MAX_CHAR_ON_DEST_SERVER";
				break;
			case CTService::CT_GAMERESULT_CHAR_HAS_ITEM_NOT_ALLOWED_TO_LEAVE_SRC_SERVER:
				resultString = "CTService::CT_GAMERESULT_CHAR_HAS_ITEM_NOT_ALLOWED_TO_LEAVE_SRC_SERVER";
				break;
			case CTService::CT_GAMERESULT_CHAR_HAS_ITEM_NOT_ALLOWED_ON_DEST_SERVER:
				resultString = "CTService::CT_GAMERESULT_CHAR_HAS_ITEM_NOT_ALLOWED_ON_DEST_SERVER";
				break;
			case CTService::CT_GAMERESULT_CHAR_NOT_ALLOWED_TO_MOVE_FROM_SRC_SERVER:
				resultString = "CTService::CT_GAMERESULT_CHAR_NOT_ALLOWED_TO_MOVE_FROM_SRC_SERVER";
				break;
			case CTService::CT_GAMERESULT_CHAR_NOT_ALLOWED_TO_MOVE_TO_DEST_SERVER:
				resultString = "CTService::CT_GAMERESULT_CHAR_NOT_ALLOWED_TO_MOVE_TO_DEST_SERVER";
				break;
			case CTService::CT_GAMERESULT_CHAR_NOT_ALLOWED_TO_MOVE_FROM_THIS_SRC_SERVER_TO_THIS_DEST_SERVER:
				resultString = "CTService::CT_GAMERESULT_CHAR_NOT_ALLOWED_TO_MOVE_FROM_THIS_SRC_SERVER_TO_THIS_DEST_SERVER";
				break;
			case CTService::CT_GAMERESULT_CHAR_TYPE_NOT_ALLOWED_TO_LEAVE_SRC_SERVER:
				resultString = "CTService::CT_GAMERESULT_CHAR_TYPE_NOT_ALLOWED_TO_LEAVE_SRC_SERVER";
				break;
			case CTService::CT_GAMERESULT_CHAR_TYPE_NOT_ALLOWED_ON_DEST_SERVER:
				resultString = "CTService::CT_GAMERESULT_CHAR_TYPE_NOT_ALLOWED_ON_DEST_SERVER";
				break;
			case CTService::CT_GAMERESULT_SOFTERROR_FOR_LOCALIZED_TEXT:
				resultString = "CTService::CT_GAMERESULT_SOFTERROR_FOR_LOCALIZED_TEXT";
				break;
			case CTService::CT_GAMERESULT_HARDERROR_FOR_LOCALIZED_TEXT:
				resultString = "CTService::CT_GAMERESULT_HARDERROR_FOR_LOCALIZED_TEXT";
				break;
			default:
				{
					std::ostringstream buffer;
					buffer << "unknown code (" << resultCode << ")" << std::ends;
					resultString = buffer.str();
				}
				break;
		}

		return resultString;
	}

	void replyMove(const TransferCharacterData & reply, CTService::CTMoveResult resultCode)
	{
		if(s_apiClient)
		{
			const std::string resultString = resultToString(resultCode);
			LOG("CustomerService", ("CharacterTransfer: transactionId=%u replyMove(%u, %s, nullptr, nullptr)", reply.getTransactionId(), reply.getTrack(), resultString.c_str()));
			const unsigned int result = static_cast<unsigned int>(resultCode);
			s_apiClient->moveComplete(reply.getSourceStationId(), reply.getTrack(), result);
			IGNORE_RETURN(s_apiClient->replyMove(reply.getTrack(), result, Unicode::narrowToWide(resultString).c_str(), nullptr));
		}
	}

}

using namespace TransferServerNamespace;

//-----------------------------------------------------------------------

TransferServer::TransferServer()
{
}

//-----------------------------------------------------------------------

TransferServer::~TransferServer()
{
}

//-----------------------------------------------------------------------

void TransferServer::authorizeDownload(unsigned int stationId)
{
	IGNORE_RETURN(s_authorizedDownloads.insert(stationId));
	REPORT_LOG(true, ("Received download authorization for station id %d\n", stationId));
}

//-----------------------------------------------------------------------

void TransferServer::unauthorizeDownload(unsigned int stationId)
{
	std::set<unsigned int>::iterator f = s_authorizedDownloads.find(stationId);
	if(f != s_authorizedDownloads.end())
	{
		s_authorizedDownloads.erase(f);
		REPORT_LOG(true, ("Received request to remove download authorization for station id %d\n", stationId));
	}
}

//-----------------------------------------------------------------------

const CharacterTransferData * TransferServer::getCharacterTransferData(unsigned int stationId, const std::string & fromGalaxy, bool administrativeRequest)
{
	const CharacterTransferData * result = 0;

	std::map<unsigned int, CharacterTransferData>::const_iterator f = s_transferData.find(stationId);
	if(f != s_transferData.end())
		result = &(*f).second;

	if(result)
	{
		bool authorized = false;
		// was the request administratively initiated?
		if(administrativeRequest)
		{
			// admins always get what they want
			authorized = true;
		}
		else
		{
			// users, however, must live up to a higher standard
			// is the request coming from a server that is allowed
			// to download character data?
			std::set<std::string>::const_iterator g = s_serversAllowedToDownload.find(fromGalaxy);
			if(g != s_serversAllowedToDownload.end())
			{
				// the request is coming from a server that a user is allowed to download to
				if(ConfigTransferServer::getAllowSameServerTransfers())
				{
					authorized = true;
				}
				else
				{
					if(fromGalaxy != result->fromGalaxy)
					{
						authorized = true;
					}
				}
			}
			else
			{
				// was this transfer pre-authorized administratively?
				std::set<unsigned int>::const_iterator a = s_authorizedDownloads.find(stationId);
				if(a != s_authorizedDownloads.end())
				{
					authorized = true;
				}
			}
		}

		// were all requirements met?
		if(! authorized)
			result = 0;
	}
	else
	{
		REPORT_LOG(true, ("Download failed: Transfer data for SUID %d was not found.\n", stationId));
	}
	return result;
}

//-----------------------------------------------------------------------

void TransferServer::requestCharacterList(unsigned int track, unsigned int stationId, const std::string & serverName, const std::string & customerLocalizedLanguage)
{
	CentralServerConnection * centralConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(serverName);
	if(centralConnection)
	{
		// this request is routed to the central server for the target
		// galaxy, which in turn queries a LoginServer for the character
		// list. The LoginServer returns the list, with the tracking
		// ID, to the CentralServer, and ultimately responds to this
		// transfer server with the listing.
		TransferCharacterData characterData(TransferRequestMoveValidation::TRS_transfer_server);
		characterData.setCustomerLocalizedLanguage(customerLocalizedLanguage);
		characterData.setSourceGalaxy(serverName);
		characterData.setSourceStationId(stationId);
		characterData.setTrack(track);
		const GenericValueTypeMessage<TransferCharacterData> request("TransferRequestCharacterList", characterData);
		centralConnection->send(request, true);
	}
	else
	{
		// send failure to CTS API backend
		if(s_apiClient)
		{
			const unsigned int result = static_cast<int>(CTService::CT_RESULT_FAILURE);
			   
			IGNORE_RETURN(s_apiClient->replyCharacterList(track, result, 0, nullptr, nullptr));
			s_apiClient->moveComplete(stationId, track, result);
		}
	}
}

//-----------------------------------------------------------------------

void TransferServer::getLoginLocationData(const TransferCharacterData & characterData)
{
	CentralServerConnection * sourceCentralConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(characterData.getSourceGalaxy());
	if(sourceCentralConnection)
	{
		LOG("CustomerService", ("CharacterTransfer: Sending getLoginLocationData for %s", characterData.getSourceCharacterName().c_str()));
		const GenericValueTypeMessage<TransferCharacterData> getLoginLocation("TransferGetLoginLocationData", characterData);
		sourceCentralConnection->send(getLoginLocation, true);
	}
}

//-----------------------------------------------------------------------

void TransferServer::requestMove(unsigned int track, const std::string & customerLocalizedLanguage, const std::string & sourceGalaxy, const std::string & destinationGalaxy, const std::string & sourceCharacter, const std::string & destinationCharacter, unsigned int sourceStationId, unsigned int destinationStationId, unsigned int transactionId, const bool withItems, const bool allowOverride)
{
	std::string withItemsString = "UNINITIALIZED";
	if(withItems)
		withItemsString = "true";
	else
		withItemsString = "false";

	std::string allowOverrideString = "UNINITIALIZED";
	if(allowOverride)
		allowOverrideString = "true";
	else
		allowOverrideString = "false";
	
	LOG("CustomerService", ("CharacterTransfer: TransferServer::requestMove(%d, %s, %s, %s, %s, %s, %d, %d, %d, %s, %s)", track, customerLocalizedLanguage.c_str(), sourceGalaxy.c_str(), destinationGalaxy.c_str(), sourceCharacter.c_str(), destinationCharacter.c_str(), sourceStationId, destinationStationId, transactionId, withItemsString.c_str(), allowOverrideString.c_str()));
	// this request is routed to the central server for the target
	// galaxy, which in turn queries a LoginServer for the character
	// list. The LoginServer populates the TransferCharacterData
	// with the source characterID. After the response is received
	// in CentralServerConnection.cpp, TransferServer::getLoginLocationData
	// is called.

	// prequalification: if both Galaxies are available, send request, otherwise send failure
	CentralServerConnection * sourceCentralConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(sourceGalaxy);
	CentralServerConnection * destinationCentralConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(destinationGalaxy);

	if(destinationStationId == 0)
		destinationStationId = sourceStationId;

	std::string destinationCharacterName = destinationCharacter;
	if(destinationCharacter.empty())
		destinationCharacterName = sourceCharacter;

	TransferCharacterData characterData(TransferRequestMoveValidation::TRS_transfer_server);
	characterData.setCustomerLocalizedLanguage(customerLocalizedLanguage);
	characterData.setSourceGalaxy(sourceGalaxy);
	characterData.setDestinationGalaxy(destinationGalaxy);
	characterData.setSourceCharacterName(sourceCharacter);
	characterData.setDestinationCharacterName(destinationCharacterName);
	characterData.setSourceStationId(sourceStationId);
	characterData.setDestinationStationId(destinationStationId);
	characterData.setTransactionId(transactionId);
	characterData.setIsMoveRequest(true);
	characterData.setWithItems(withItems);
	characterData.setAllowOverride(allowOverride);
	characterData.setTrack(track);

	/*
	TransferCharacterData characterData(track, // track
		customerLocalizedLanguage,     // customerLocalizedLanguage
		sourceGalaxy,                  // sourceGalaxy
		destinationGalaxy,             // destinationGalaxy
		sourceCharacter,               // sourceCharacterName
		destinationCharacterName,      // destinationCharacterName
		sourceStationId,               // sourceStationId
		destinationStationId,          // destinationStationId
		transactionId,                 // transactionId
		NetworkId::cms_invalid,        // characterId
		NetworkId::cms_invalid,        // containerId
		std::string(""),               // scene
		0,                             // clusterId
		Vector(),                      // starting coordinates
		std::string(""),               // customization data
		std::vector<unsigned char>(),  // script dictionary buffer
		std::string(""),               // object template name
		1.0f,                          // scale factor
		std::string(""),               // hair template name
		std::string(""),               // hair appearance data
		std::string(""),               // profession
		false,                         // is a jedi
		Unicode::String(),             // Biography
		NetworkId::cms_invalid,        // destination character id
		false,                         // is name validated?
		true,                          // is this a move request?
		withItems);                    // does the move include items?
	*/

	if(sourceCentralConnection && destinationCentralConnection)
	{
		// transfer the character
		const GenericValueTypeMessage<TransferCharacterData> getCharacterData("TransferGetCharacterDataFromLoginServer", characterData);
		sourceCentralConnection->send(getCharacterData, true);
		s_apiClient->moveStart(sourceStationId, track, sourceCentralConnection, destinationCentralConnection);
		LOG("CustomerService", ("CharacterTransfer: sent TransferGetCharacterDataFromLoginServer message : %s\n", characterData.toString().c_str()));

		const GenericValueTypeMessage<std::pair<unsigned int, unsigned int> > kick("TransferRequestKickConnectedClients", std::make_pair(sourceStationId, destinationStationId));
		sourceCentralConnection->send(kick, true);
		destinationCentralConnection->send(kick, true);
	}
	else
	{
		replyMove(characterData, CTService::CT_GAMERESULT_SERVER_IS_DOWN);
	}
}

//-----------------------------------------------------------------------

void TransferServer::requestTransferAccount(unsigned int track, unsigned int sourceStationId, unsigned int destinationStationId, unsigned int transactionId)
{
	LOG("CustomerService", ("CharacterTransfer: TransferServer::requestAccount(%d, %d, %d, %d)", track, sourceStationId, destinationStationId, transactionId));

	if(!ConfigTransferServer::getAllowAccountTransfers())
	{
		LOG("CustomerService", ("Cannot transfer account, account transfers are not enabled\n"));
		if(s_apiClient)
		{
			const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_HARDERROR);
			IGNORE_RETURN(s_apiClient->replyValidateMove(track, result, nullptr, nullptr, nullptr));
			s_apiClient->moveComplete(sourceStationId, track, result);
		}

		return;
	}

	// this request is routed to a login server
	// which in turn updates the DatabaseProcess in the CentralServer, and finally updates the ChatServer
	// prequalification: if a galaxy is available, send request, otherwise send failure
	std::map<std::string, CentralServerConnection *> galaxies = CentralServerConnection::getGalaxies();
	std::map<std::string, CentralServerConnection *>::iterator firstGalaxy = galaxies.begin();

	if (firstGalaxy == galaxies.end())
	{
		LOG("CustomerService", ("CharacterTransfer: Account to account move request made, but no central server connections are available in transfer server source station id: %lu destination station id: %lu\n", sourceStationId, destinationStationId));

		if(s_apiClient)
		{
			const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_SERVER_IS_DOWN);
			IGNORE_RETURN(s_apiClient->replyTransferAccount(track, result, nullptr, nullptr));
			s_apiClient->moveComplete(sourceStationId, track, result);
		}

		return;
	}

	const std::string galaxyName = firstGalaxy->first;
	CentralServerConnection * sourceCentralConnection = firstGalaxy->second;

	if(destinationStationId == 0 || sourceStationId == 0)
	{
		LOG("CustomerService", ("CharacterTransfer: Account move request made with a nullptr destination station id: or source station id: %lu\n", sourceStationId));

		if(s_apiClient)
		{
			const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_HARDERROR);
			IGNORE_RETURN(s_apiClient->replyTransferAccount(track, result, nullptr, nullptr));
			s_apiClient->moveComplete(sourceStationId, track, result);
		}

		return;
	}

	TransferAccountData transferData;
	transferData.setSourceStationId(sourceStationId);
	transferData.setDestinationStationId(destinationStationId);
	transferData.setTransactionId(transactionId);
	transferData.setStartGalaxy(galaxyName);
	transferData.setTrack(track);

	// verify neither of the station ids are connected
	const GenericValueTypeMessage<std::pair<unsigned int, unsigned int> > kick("TransferRequestKickConnectedClients", std::make_pair(sourceStationId, destinationStationId));
	sourceCentralConnection->send(kick, true);

	// start the transfer process
	const GenericValueTypeMessage<TransferAccountData> getAccountData("TransferAccountRequestLoginServer", transferData);
	sourceCentralConnection->send(getAccountData, true);

	if (s_apiClient)
		s_apiClient->moveStart(sourceStationId, track, sourceCentralConnection, 0);

	LOG("CustomerService", ("CharacterTransfer: sent TransferAccountRequestLoginServer message : %s\n", transferData.toString().c_str()));
}

//-----------------------------------------------------------------------

void TransferServer::requestMoveValidation(unsigned int track, unsigned int sourceStationId, unsigned int destinationStationId, const std::string & sourceGalaxy, const std::string & destinationGalaxy, const std::string & sourceCharacter, const std::string & destinationCharacter, const std::string & customerLocalizedLanguage)
{
	// prequalification: if both Galaxies are available, send request, otherwise send failure
	const CentralServerConnection * sourceCentralConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(sourceGalaxy);
	CentralServerConnection * destinationCentralConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(destinationGalaxy);

	if(sourceStationId != destinationStationId && (!ConfigTransferServer::getAllowAccountTransfers()))
	{
		if(s_apiClient)
		{
			const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_HARDERROR);
			IGNORE_RETURN(s_apiClient->replyValidateMove(track, result, nullptr, nullptr, nullptr));
			s_apiClient->moveComplete(sourceStationId, track, result);
		}
	}
	else
	{
		if(sourceCentralConnection && destinationCentralConnection)
		{
			// This is sent to the CentralServer on the destination galaxy
			// which then forwards it to an arbitrary LoginServer
			// the validation response is routed back to the TransferServer
			// and is handled in CentralServerConnection as a TransferReplyMoveValidation

			// see if we already have the source character's template crc from a previous query
			// and if so, pass that along as an optimization so the LoginServer doesn't have
			// to continually query the DB to fetch the same information over and over again, because
			// the character's template crc (i.e. the character's species) should never, ever change
			std::map<std::pair<unsigned int, std::pair<std::string, std::string> >, uint32>::const_iterator const iterFind = s_cacheSourceCharacterTemplateCrc.find(std::make_pair(sourceStationId, std::make_pair(sourceGalaxy, sourceCharacter)));

			const TransferRequestMoveValidation request(TransferRequestMoveValidation::TRS_transfer_server, track, sourceStationId, destinationStationId, sourceGalaxy, destinationGalaxy, sourceCharacter, NetworkId::cms_invalid, ((iterFind == s_cacheSourceCharacterTemplateCrc.end()) ? 0 : iterFind->second), destinationCharacter, customerLocalizedLanguage);
	
			destinationCentralConnection->send(request, true);
		}
		else
		{
			if(s_apiClient)
			{
				const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_SERVER_IS_DOWN);
				IGNORE_RETURN(s_apiClient->replyValidateMove(track, result, nullptr, nullptr, nullptr));
				s_apiClient->moveComplete(sourceStationId, track, result);
			}
		}
	}

}

//-----------------------------------------------------------------------

void TransferServer::cacheSourceCharacterTemplateCrc(unsigned int stationId, const std::string & galaxy, const std::string & characterName, uint32 characterTemplateCrc)
{
	s_cacheSourceCharacterTemplateCrc[std::make_pair(stationId, std::make_pair(galaxy, characterName))] = characterTemplateCrc;
}

//-----------------------------------------------------------------------

void TransferServer::run()
{
	SetupSharedLog::install("TransferServer");
	ConsoleManager::install();
	DataTableManager::install();

	std::vector<std::string>::const_iterator i;
	
	for (i = ConfigTransferServer::getServersAllowedToUploadCharacterData().begin(); i != ConfigTransferServer::getServersAllowedToUploadCharacterData().end(); ++i)
	{
		IGNORE_RETURN(s_serversAllowedToUpload.insert(*i));
		REPORT_LOG(true, ("%s allows players to upload character data\n", i->c_str()));
	}

	for (i = ConfigTransferServer::getServersAllowedToDownloadCharacterData().begin(); i != ConfigTransferServer::getServersAllowedToDownloadCharacterData().end(); ++i)
	{
		IGNORE_RETURN(s_serversAllowedToDownload.insert(*i));
		REPORT_LOG(true, ("%s allows players to download character data without administrative intervention\n", i->c_str()));
	}

	NetworkSetupData setup;
	setup.port = ConfigTransferServer::getCentralServerServiceBindPort();
	setup.bindInterface = ConfigTransferServer::getCentralServerServiceBindInterface();
	setup.maxConnections=500;

	if(strlen(ConfigTransferServer::getApiServerHostAddress()))
	{
		s_apiClient = new CTSAPIClient(ConfigTransferServer::getApiServerHostAddress(), "SWG");
		REPORT_LOG(true, ("Attempting to connect to CTS Backend %s\n", ConfigTransferServer::getApiServerHostAddress()));
		LOG("CTSAPI", ("Attempting to connect to CTS Backend %s", ConfigTransferServer::getApiServerHostAddress()));
	}
	else
	{
		REPORT_LOG(true, ("Running without CTS backend connection because none is configured.\n"));
		LOG("CTSAPI", ("Running without CTS backend connection because none is configured."));
	}
	
	s_transferServerService = new Service(ConnectionAllocator<CentralServerConnection>(), setup);
	while(! s_done)
	{
		if(s_apiClient)
		{
			s_apiClient->process();
		}
		
		NetworkHandler::update();
		NetworkHandler::dispatch();
		NetworkHandler::clearBytesThisFrame();
		Os::sleep(1);
	}
	delete s_apiClient;
}

//-----------------------------------------------------------------------

void TransferServer::setDone(bool isDone)
{
	s_done = isDone;
}

//-----------------------------------------------------------------------

bool TransferServer::uploadCharacterTransferData(const CharacterTransferData & data, bool administrative)
{
	bool result = false;
	std::set<std::string>::const_iterator f = s_serversAllowedToUpload.find(data.fromGalaxy);

	bool authorized = administrative;
	if(! authorized)
	{
		std::set<unsigned int>::const_iterator a = s_authorizedDownloads.find(data.stationId);
		if(a != s_authorizedDownloads.end())
		{
			authorized = true;
		}
	}

	if(f != s_serversAllowedToUpload.end() || authorized)
	{
		s_transferData[data.stationId] = data;
		REPORT_LOG(true, ("%d bytes of data for SUID %d uploaded\n", data.packedData.length(), data.stationId));
		result = true;
	}
	else
	{
		REPORT_LOG(true, ("An attempt to upload character data from %s was denied by the TransferServer configuration\n", data.fromGalaxy.c_str()));
	}
	return result;
}

//-----------------------------------------------------------------------

void TransferServer::replyCharacterList(const TransferReplyCharacterList & reply)
{
	if(s_apiClient)
	{
		std::vector<AvatarRecord>::const_iterator i;
		std::vector<CTService::CTServiceCharacter> chars;
		for(i = reply.getAvatarList().begin(); i != reply.getAvatarList().end(); ++i)
		{
			CTService::CTServiceCharacter character;
			character.SetCharacter(i->m_name.c_str());
			chars.push_back(character);
			LOG("CTSAPI", ("adding CTService::CTServiceCharacter(%s) to replyCharacterList", Unicode::wideToNarrow(i->m_name).c_str()));
		}
		
		CTService::CTServiceCharacter * characters = 0;
		unsigned int result = static_cast<unsigned int>(CTService::CT_RESULT_SUCCESS);
		
		if(! reply.getAvatarList().empty())
		{
			characters = &chars[0];
		}
		else
		{
			result = static_cast<unsigned int>(CTService::CT_RESULT_FAILURE);
			s_apiClient->moveComplete(reply.getStationId(), reply.getTrack(), result);
		}
		LOG("CTSAPI", ("invoking CTServiceAPI::replyCharacterList(%d, %d, %d, characters, nullptr)", reply.getTrack(), result, chars.size()));
		IGNORE_RETURN(s_apiClient->replyCharacterList(reply.getTrack(), result, reply.getAvatarList().size(), characters, nullptr));
	}
	// else use another interface if available
}

//-----------------------------------------------------------------------

void TransferServer::replyValidateMove(const TransferCharacterData & reply)
{
	LOG("CustomerService", ("CharacterTransfer: Received replyValidateMove for %s", reply.toString().c_str()));

	if(s_apiClient)
	{
		unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_SUCCESS);
		if(! reply.getIsValidName())
		{
			LOG("CustomerService", ("CharacterTransfer: replyValidateMove failed name validation for %s", reply.toString().c_str()));
			result = static_cast<unsigned int>(CTService::CT_GAMERESULT_NAME_ALREADY_TAKE);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: replyValidateMove passed name validation for %s", reply.toString().c_str()));
		}
		IGNORE_RETURN(s_apiClient->replyValidateMove(reply.getTrack(), result, nullptr, nullptr, nullptr));
		s_apiClient->moveComplete(reply.getSourceStationId(), reply.getTrack(), result);
	}
}

//-----------------------------------------------------------------------

void TransferServer::replyMoveSuccess(const TransferCharacterData & reply)
{
	// send RequestTransferAvatar to ChatServer via CentralServer
	CentralServerConnection * centralServerConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(reply.getSourceGalaxy());
	if(centralServerConnection)
	{
		if(ConfigTransferServer::getTransferChatAvatar())
		{
			const GenericValueTypeMessage<TransferCharacterData> chatTransfer("RequestChatTransferAvatar", reply);
			LOG("CustomerService", ("CharacterTransfer: Sending RequestChatTransferAvatar to Central Server %s. %s", chatTransfer.getValue().getSourceGalaxy().c_str(), chatTransfer.getValue().toString().c_str()));
			centralServerConnection->send(chatTransfer, true);
		}
		else
		{
			LOG("CustomerService", ("Not transfering chat avatar. Chat transfers are currently disabled"));
		}
	}
	
	replyMove(reply, CTService::CT_GAMERESULT_SUCCESS);
}

//-----------------------------------------------------------------------

void TransferServer::replyTransferAccountSuccess(const TransferAccountData & reply)
{
	// send RequestTransferAvatar to ChatServer via CentralServer for each of the cluster ids
	if(ConfigTransferServer::getTransferChatAvatar())
	{
		const std::vector<AvatarData> avatarData(reply.getSourceAvatarData());
		for (std::vector<AvatarData>::const_iterator i = avatarData.begin(); i != avatarData.end(); ++i)
		{
			LOG("CustomerService", ("CharacterTransfer: Sending request to update chat server (cluster: %s) for account transfer from %lu to %lu\n", (i->first).c_str(), reply.getSourceStationId(), reply.getDestinationStationId()));

			// need to make a small transferCharacterData object to send with the message
			TransferCharacterData chatRequest(TransferRequestMoveValidation::TRS_transfer_server);
			chatRequest.setSourceStationId(reply.getSourceStationId());
			chatRequest.setSourceGalaxy(i->first);
			chatRequest.setSourceCharacterName(i->second);
			chatRequest.setDestinationStationId(reply.getDestinationStationId());
			chatRequest.setDestinationGalaxy(i->first);
			chatRequest.setDestinationCharacterName(i->second);

			CentralServerConnection * centralServerConnection = CentralServerConnection::getCentralServerConnectionForGalaxy(i->first);

			if (centralServerConnection)
			{
				const GenericValueTypeMessage<TransferCharacterData> message("RequestChatTransferAvatar", chatRequest);
				centralServerConnection->send(message, true);
			}
			else
			{
				LOG("CustomerService", ("CharacterTransfer: Could not transfer chat avatars for character %s on cluster %s from %lu to %lu: central connection does not exist in transferserver", i->second.c_str(), i->first.c_str(), reply.getSourceStationId(), reply.getDestinationStationId()));
				const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_HARDERROR);
				s_apiClient->moveComplete(reply.getSourceStationId(), reply.getTrack(), result);
				IGNORE_RETURN(s_apiClient->replyTransferAccount(reply.getTrack(), result, nullptr, nullptr));
			}
		}
	}
	else
	{
		LOG("CustomerService", ("CharacterTransfer: Not transfering any chat avatars. Chat transfers are currently disabled"));
	}

	if(s_apiClient)
	{
		const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_SUCCESS);
		IGNORE_RETURN(s_apiClient->replyTransferAccount(reply.getTrack(), result, nullptr, nullptr));
		s_apiClient->moveComplete(reply.getSourceStationId(), reply.getTrack(), result);
	}
}

//-----------------------------------------------------------------------

void TransferServer::transferCreateCharacterFailed(const TransferCharacterData & failed)
{
	replyMove(failed, CTService::CT_GAMERESULT_INVALID_NAME);
		
	// close pseudoclientconnections
	closePseudoClientConnections(failed.getSourceGalaxy(), failed.getSourceStationId(), failed.getDestinationGalaxy(), failed.getDestinationStationId());
}

//-----------------------------------------------------------------------

bool TransferServer::isRename(const TransferCharacterData & request)
{
	// IF:
	// the source and destination station ID are identcal (not a transfer to another account)
	// the source and destination galaxy are identicay (not a transfer to another server)
	// the destination name is not empty
	// the source and destination name are different
	// This is a character rename request
	return (request.getSourceStationId() == request.getDestinationStationId() && request.getSourceGalaxy() == request.getDestinationGalaxy() && !request.getDestinationCharacterName().empty() && request.getSourceCharacterName() != request.getDestinationCharacterName());
}

//-----------------------------------------------------------------------

void TransferServer::failedToRetrieveTransferData(const TransferCharacterData & failed)
{
	LOG("CustomerService", ("CharacterTransfer: Transfer failed: Failed to retrieve character data. Sending HARD ERROR to CTS API. %s", failed.toString().c_str()));
	replyMove(failed, CTService::CT_GAMERESULT_HARDERROR);

	// close pseudoclientconnections
	closePseudoClientConnections(failed.getSourceGalaxy(), failed.getSourceStationId(), failed.getDestinationGalaxy(), failed.getDestinationStationId());
}

//-----------------------------------------------------------------------

void TransferServer::failedToApplyTransferData(const TransferCharacterData & failed)
{
	LOG("CustomerService", ("CharacterTransfer: Transfer failed: Failed to apply character data. Sending HARD ERROR to CTS API. %s", failed.toString().c_str()));

	replyMove(failed, CTService::CT_GAMERESULT_HARDERROR);

	// close pseudoclientconnections
	closePseudoClientConnections(failed.getSourceGalaxy(), failed.getSourceStationId(), failed.getDestinationGalaxy(), failed.getDestinationStationId());
}

//-----------------------------------------------------------------------

void TransferServer::failedToTransferAccountNoCentralConnection(const TransferAccountData & failed)
{
	if(s_apiClient)
	{
		LOG("CustomerService", ("CharacterTransfer: Transfer failed: Could not connect to central server to update game database. Sending HARD ERROR to CTS API. %s", failed.toString().c_str()));
		const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_HARDERROR);
		s_apiClient->moveComplete(failed.getSourceStationId(), failed.getTrack(), result);
		IGNORE_RETURN(s_apiClient->replyTransferAccount(failed.getTrack(), result, nullptr, nullptr));
	}
}

//-----------------------------------------------------------------------

void TransferServer::failedToTransferAccountDestinationNotEmpty (const TransferAccountData & failed)
{
	if(s_apiClient)
	{
		LOG("CustomerService", ("CharacterTransfer: Transfer failed: Destination stationId has avatars. Sending HARD ERROR to CTS API. %s", failed.toString().c_str()));
		const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_HARDERROR);
		s_apiClient->moveComplete(failed.getSourceStationId(), failed.getTrack(), result);
		IGNORE_RETURN(s_apiClient->replyTransferAccount(failed.getTrack(), result, nullptr, nullptr));
	}
}

//-----------------------------------------------------------------------

void TransferServer::failedToTransferCharacterGameConnectionClosed(const TransferCharacterData & failed)
{
	LOG("CustomerService", ("CharacterTransfer: Transfer failed: GameServer connection closed while transfer was in progress. Sending SOFT ERROR to CTS API. %s", failed.toString().c_str()));

	replyMove(failed, CTService::CT_GAMERESULT_SOFTERROR);

	// close pseudoclientconnections
	closePseudoClientConnections(failed.getSourceGalaxy(), failed.getSourceStationId(), failed.getDestinationGalaxy(), failed.getDestinationStationId());
}

//-----------------------------------------------------------------------

void TransferServer::failedToTransferCharacterConnectionServerConnectionClosed(unsigned int stationId)
{
	if(s_apiClient)
	{
		LOG("CustomerService", ("CharacterTransfer: Transfer failed: ConnectionServer connection closed with CentralServer while transfer was in progress for station id %d", stationId));
		const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_SOFTERROR);
		s_apiClient->moveComplete(stationId, 0, result);
		// tracking information is not availalable in this case, so an
		// API call can not be made. This is a SOFT error, however, and
		// the API will retry the transfer if there is a timeout.
	}
}

//-----------------------------------------------------------------------

void TransferServer::failedToValidateTransfer(const TransferReplyMoveValidation & reply)
{
	if(s_apiClient)
	{
		const unsigned int result = ((reply.getResult() == TransferReplyMoveValidation::TRMVR_cannot_create_regular_character) ? static_cast<unsigned int>(CTService::CT_GAMERESULT_MAX_CHAR_ON_DEST_SERVER) : static_cast<unsigned int>(CTService::CT_GAMERESULT_SERVER_IS_DOWN));

		IGNORE_RETURN(s_apiClient->replyValidateMove(reply.getTrack(), result, nullptr, nullptr, nullptr));
		s_apiClient->moveComplete(reply.getSourceStationId(), reply.getTrack(), result);

		// close pseudoclientconnections
		closePseudoClientConnections(reply.getSourceGalaxy(), reply.getSourceStationId(), reply.getDestinationGalaxy(), reply.getDestinationStationId());
	}
}

//-----------------------------------------------------------------------
