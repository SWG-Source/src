// CTSAPIClient.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall
//-----------------------------------------------------------------------

#include "FirstTransferServer.h"
#include "CentralServerConnection.h"
#include "ConfigTransferServer.h"
#include "CTSAPIClient.h"
#include "CTCommon/CTServiceServer.h"
#include <map>
#include "sharedLog/Log.h"
#include "TransferServer.h"
#include "UnicodeUtils.h"
#include <set>
#include <vector>

class CentralServerConnection;

//-----------------------------------------------------------------------

namespace CTSAPIClientNamespace
{
	struct TransactionTrackData
	{
		const CentralServerConnection *  m_sourceCentralServerConnection;
		const CentralServerConnection *  m_destinationCentralServerConnection;
		unsigned int                     m_track;
	};
	
	std::map<unsigned int, TransactionTrackData> s_activeTransfers;
	std::map<unsigned int, int> s_completedTransfers;
}
using namespace CTSAPIClientNamespace;

//-----------------------------------------------------------------------

CTSAPIClient::CTSAPIClient(const std::string & hostName, const std::string & game) :
CTServiceAPI(hostName.c_str(), game.c_str())
{
}

//-----------------------------------------------------------------------

CTSAPIClient::~CTSAPIClient()
{
}

//-----------------------------------------------------------------------

void CTSAPIClient::onTest(const unsigned track, const int resultCode, const unsigned value, void *user)
{
	LOG("CTSAPI", ("CTSAPIClient::onTest(%d, %d, %d, %p)", track, resultCode, value, user));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onReplyTest(const unsigned track, const int resultCode, void *user)
{
	LOG("CTSAPI", ("CTSAPIClient::onReplyTest(%d, %d, %p)", track, resultCode, user));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onReplyMoveStatus(const unsigned track, const int resultCode, void *user)
{
	LOG("CTSAPI", ("CTSAPIClient::onReplyMoveStatus(%d, %d, %p)", track, resultCode, user));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onReplyValidateMove(const unsigned track, const int resultCode, void *user)
{
	LOG("CTSAPI", ("CTSAPIClient::onReplyValidateMove(%d, %d, %p)", track, resultCode, user));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onReplyMove(const unsigned track, const int resultCode, void *user)
{
	LOG("CTSAPI", ("CTSAPIClient::onReplyMove(%d, %d, %p)", track, resultCode, user));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onReplyTransferAccount(const unsigned track, const int resultCode, void *user)
{
	LOG("CTSAPI", ("CTSAPIClient::onReplyTransferAccount(%d, %d, %p)", track, resultCode, user));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onReplyCharacterList(const unsigned track, const int resultCode, void *user)
{
	LOG("CTSAPI", ("CTSAPIClient::onReplyCharacterList(%d, %d, %p)", track, resultCode, user));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onReplyServerList(const unsigned track, const int resultCode, void *user)
{
	LOG("CTSAPI", ("CTSAPIClient::onReplyServerList(%d, %d, %p)",track, resultCode, user));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onReplyDestinationServerList(const unsigned track, const int resultCode, void *user)
{
	LOG("CTSAPI", ("CTSAPIClient::onReplyDestinationServerList(%d, %d, %p)", track, resultCode, user));
}

//-----------------------------------------------------------------------
void CTSAPIClient::onServerTest(const unsigned server_track, const char *game, const char *param)
{
	LOG("CTSAPI", ("CTSAPIClient::onServerTest(%d, %s, %s)", server_track, game, param));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onRequestMoveStatus(const unsigned server_track, const char *language, const unsigned transactionID)
{
	LOG("CTSAPI", ("CTSAPIClient::onRequestMoveStatus(%d, %s, %d)", server_track, language, transactionID));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onRequestValidateMove(const unsigned server_track, const char *language, const CTService::CTUnicodeChar *sourceServer, const CTService::CTUnicodeChar *destServer, const CTService::CTUnicodeChar *sourceCharacter, const CTService::CTUnicodeChar *destCharacter, const unsigned uid, const unsigned destuid, bool /*withItems*/, bool /*allowOverride*/)
{
	LOG("CTSAPI", ("CTSAPIClient::onRequestValidateMove(%d, %s, %s, %s, %s, %s, %d, %d", server_track, language, Unicode::wideToNarrow(sourceServer).c_str(), Unicode::wideToNarrow(destServer).c_str(), Unicode::wideToNarrow(sourceCharacter).c_str(), Unicode::wideToNarrow(destCharacter).c_str(), uid, destuid));
	TransferServer::requestMoveValidation(server_track, uid, destuid, Unicode::wideToNarrow(Unicode::String(sourceServer)), Unicode::wideToNarrow(Unicode::String(destServer)), Unicode::wideToNarrow(Unicode::String(sourceCharacter)), Unicode::wideToNarrow(Unicode::String(destCharacter)), std::string(language));
}

//-----------------------------------------------------------------------

void CTSAPIClient::onRequestMove(const unsigned server_track, const char *language, const CTService::CTUnicodeChar *sourceServer, const CTService::CTUnicodeChar *destServer, const CTService::CTUnicodeChar *sourceCharacter, const CTService::CTUnicodeChar *destCharacter, const unsigned uid, const unsigned destuid, const unsigned transactionID, bool withItems, bool allowOverride)
{
	std::map<unsigned int, TransactionTrackData>::const_iterator f = s_activeTransfers.find(uid);
	if(f == s_activeTransfers.end())
	{
		std::string isWithItems = "false";
		if(withItems)
			isWithItems = "true";
		LOG("CTSAPI", ("CTSAPIClient::onRequestMove(%d, %s, %s, %s, %s, %s, %d, %d, %d, %s)", server_track, language, Unicode::wideToNarrow(sourceServer).c_str(), Unicode::wideToNarrow(destServer).c_str(), Unicode::wideToNarrow(sourceCharacter).c_str(), Unicode::wideToNarrow(destCharacter).c_str(), uid, destuid, transactionID, isWithItems.c_str()));
		TransferServer::requestMove(server_track, std::string(language), Unicode::wideToNarrow(Unicode::String(sourceServer)), Unicode::wideToNarrow(Unicode::String(destServer)), Unicode::wideToNarrow(Unicode::String(sourceCharacter)), Unicode::wideToNarrow(Unicode::String(destCharacter)), uid, destuid, transactionID, withItems, allowOverride);
	}
	else
	{
		LOG("CTSAPI", ("Received a transfer request for station ID %d, but a transfer for that id is already in progress", uid));
		const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_SOFTERROR);
		IGNORE_RETURN(replyMove(server_track, result, nullptr, nullptr));
	}
}

//-----------------------------------------------------------------------

void CTSAPIClient::onRequestTransferAccount(const unsigned server_track, const unsigned uid, const unsigned destuid, const unsigned transactionID)
{
	std::map<unsigned int, TransactionTrackData>::const_iterator f = s_activeTransfers.find(uid);
	if(f == s_activeTransfers.end())
	{
		LOG("CTSAPI", ("CTSAPIClient::onRequestTransferAccount(%d, d, %d, %d)", server_track, uid, destuid, transactionID));
		TransferServer::requestTransferAccount(server_track, uid, destuid, transactionID);
	}
	else
	{
		LOG("CTSAPI", ("Received a transfer request for station ID %d, but a transfer for that id is already in progress", uid));
		const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_SOFTERROR);
		IGNORE_RETURN(replyTransferAccount(server_track, result, nullptr, nullptr));
	}
}

//-----------------------------------------------------------------------

void CTSAPIClient::onRequestCharacterList(const unsigned server_track, const char *language, const CTService::CTUnicodeChar *server, const unsigned uid)
{
	LOG("CTSAPI", ("CTSAPIClient::onRequestCharacterList(%d, %s, %s, %d)", server_track, language, Unicode::wideToNarrow(server).c_str(), uid));
	TransferServer::requestCharacterList(server_track, uid, Unicode::wideToNarrow(Unicode::String(server)), std::string(language)); 
}

//-----------------------------------------------------------------------

void CTSAPIClient::onRequestServerList(const unsigned server_track, const char *language)
{
	LOG("CTSAPI", ("CTSAPIClient::onRequestServerList(%d, %s)", server_track, language));
	std::map<std::string, CentralServerConnection *> galaxies = CentralServerConnection::getGalaxies();
	std::vector<CTService::CTServiceServer> servers;
	std::vector<std::string>::const_iterator i;
	
	for (i = ConfigTransferServer::getServersAllowedToUploadCharacterData().begin(); i != ConfigTransferServer::getServersAllowedToUploadCharacterData().end(); ++i)
	{
		std::map<std::string, CentralServerConnection *>::iterator f = galaxies.find(*i);
		if(f != galaxies.end())
		{
			servers.push_back(CTService::CTServiceServer(Unicode::narrowToWide(i->c_str()).c_str()));
		}
	}
	if(! servers.empty())
	{
		const unsigned int result = static_cast<unsigned int>(CTService::CT_RESULT_SUCCESS);
		IGNORE_RETURN(replyServerList(server_track, result, servers.size(), &servers[0], nullptr));
	}
	else
	{
		const unsigned int result = static_cast<unsigned int>(CTService::CT_RESULT_FAILURE);
		IGNORE_RETURN(replyServerList(server_track, result, servers.size(), nullptr, nullptr));
	}
}

//-----------------------------------------------------------------------

void CTSAPIClient::onRequestDestinationServerList(const unsigned server_track, const char *language, const CTService::CTUnicodeChar *character, const CTService::CTUnicodeChar *server)
{
	LOG("CTSAPI", ("CTSAPIClient::onRequestDestinationServerList(%d, %s, %s, %s)", server_track, language, Unicode::wideToNarrow(character).c_str(), Unicode::wideToNarrow(server).c_str()));
	std::map<std::string, CentralServerConnection *> galaxies = CentralServerConnection::getGalaxies();
	std::vector<CTService::CTServiceServer> servers;
	std::vector<std::string>::const_iterator i;
	
	for (i = ConfigTransferServer::getServersAllowedToDownloadCharacterData().begin(); i != ConfigTransferServer::getServersAllowedToDownloadCharacterData().end(); ++i)
	{
		std::map<std::string, CentralServerConnection *>::iterator f = galaxies.find(*i);
		if(f != galaxies.end())
		{
			servers.push_back(CTService::CTServiceServer(Unicode::narrowToWide(i->c_str()).c_str()));
		}
	}
	if(! servers.empty())
	{
		const unsigned int result = static_cast<unsigned int>(CTService::CT_RESULT_SUCCESS);
		IGNORE_RETURN(replyDestinationServerList(server_track, result, servers.size(), &servers[0], nullptr));
	}
	else
	{
		const unsigned int result = static_cast<unsigned int>(CTService::CT_RESULT_FAILURE);
		IGNORE_RETURN(replyDestinationServerList(server_track, result, servers.size(), nullptr, nullptr));
	}
}

//-----------------------------------------------------------------------

void CTSAPIClient::onConnect(const char *host, const short port, const short current, const short max)
{
	LOG("CTSAPI", ("CTSAPIClient::onConnect(%s, %d, %d, %d)", host, port, current, max));
	REPORT_LOG(true, ("CTSAPIClient::onConnect(%s, %d, %d, %d)\n", host, port, current, max));
}

//-----------------------------------------------------------------------


void CTSAPIClient::onDisconnect(const char *host, const short port, const short current, const short max)
{
	LOG("CTSAPI", ("CTSAPIClient::onDisconnect(%s, %d, %d, %d)", host, port, current, max));
	REPORT_LOG(true, ("CTSAPIClient::onDisconnect(%s, %d, %d, %d)\n", host, port, current, max));
}

//-----------------------------------------------------------------------

void CTSAPIClient::moveStart(unsigned int stationId, unsigned int track, const CentralServerConnection * sourceCentralServerConnection, const CentralServerConnection * destinationCentralServerConnection)
{
	// first check to see if this track has been completed
	std::map<unsigned int, int>::iterator f = s_completedTransfers.find(track);
	if(f != s_completedTransfers.end())
	{
		IGNORE_RETURN(replyMove(track, f->second, nullptr, nullptr));
	}
	else
	{
		TransactionTrackData trackData;
		trackData.m_destinationCentralServerConnection = destinationCentralServerConnection;
		trackData.m_sourceCentralServerConnection = sourceCentralServerConnection;
		trackData.m_track = track;
		IGNORE_RETURN(s_activeTransfers.insert(std::make_pair(stationId, trackData)));
	}
}

//-----------------------------------------------------------------------

void CTSAPIClient::lostCentralServerConnection(const CentralServerConnection * centralServerConnection)
{
	// send a failure response to the CTS backend for all id's with
	// a transaction in progress on the dead server
	std::map<unsigned int, TransactionTrackData>::iterator i;
	const unsigned int result = static_cast<unsigned int>(CTService::CT_GAMERESULT_SERVER_IS_DOWN);
	for(i = s_activeTransfers.begin(); i != s_activeTransfers.end();)
	{
		if(i->second.m_sourceCentralServerConnection == centralServerConnection || i->second.m_destinationCentralServerConnection == centralServerConnection)
		{
			IGNORE_RETURN(replyMove(i->second.m_track, result, nullptr, nullptr));
			s_activeTransfers.erase(i++);
		}
		else
		{
			++i;
		}
	}
}

//-----------------------------------------------------------------------
// NOT IMPLEMENTED IN SWG TRANSFER SERVICE
void CTSAPIClient::moveComplete(unsigned int id, unsigned int track, int result) const
{
	std::map<unsigned int, TransactionTrackData>::iterator f = s_activeTransfers.find(id);
	if(f != s_activeTransfers.end())
	{
		s_activeTransfers.erase(f);
	}
	if(track != 0)
	{
		s_completedTransfers.insert(std::make_pair(track, result));
	}
}

//-----------------------------------------------------------------------
// NOT IMPLEMENTED IN SWG TRANSFER SERVICE
void CTSAPIClient::onReplyDelete(const unsigned int, const int, void *)
{
}

//-----------------------------------------------------------------------
// NOT IMPLEMENTED IN SWG TRANSFER SERVICE
void CTSAPIClient::onReplyRestore(const unsigned int, const int, void *)
{
}

//-----------------------------------------------------------------------
// NOT IMPLEMENTED IN SWG TRANSFER SERVICE
void CTSAPIClient::onRequestDelete(const unsigned int, const char *, const CTService::CTUnicodeChar *, const CTService::CTUnicodeChar *, const CTService::CTUnicodeChar *, const CTService::CTUnicodeChar *, const unsigned int, const unsigned int, const unsigned int, bool, bool)
{
}

//-----------------------------------------------------------------------
// NOT IMPLEMENTED IN SWG TRANSFER SERVICE
void CTSAPIClient::onRequestRestore(const unsigned int, const char *, const CTService::CTUnicodeChar *, const CTService::CTUnicodeChar *, const CTService::CTUnicodeChar *, const CTService::CTUnicodeChar *, const unsigned int, const unsigned int, const unsigned int, bool, bool)
{
}
	
//-----------------------------------------------------------------------
