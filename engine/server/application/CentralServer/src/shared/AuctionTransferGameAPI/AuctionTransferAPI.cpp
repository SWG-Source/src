#include "AuctionTransferAPI.h"
#include "AuctionTransferAPICore.h"
#include "Request.h"
#include "Response.h"
#include "zip/GZipHelper.h"

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT 5901
#define DEFAULT_IDENTIFIER "GAME+UNKNOWN"
//////////////////////////////////////////////////////////////////////////////////////
namespace AuctionTransfer
{

//////////////////////////////////////////////////////////////////////////////////////
AuctionTransferAPI::AuctionTransferAPI(const char *hostName[], const short port[], int count, const char *identifier[], unsigned identifierCount, unsigned reqTimeout, unsigned maxRecvMessageSizeInKB)
{
	m_apiCore = new AuctionTransferAPICore(hostName, port, count, this, identifier, identifierCount, reqTimeout, maxRecvMessageSizeInKB);
}

//////////////////////////////////////////////////////////////////////////////////////
AuctionTransferAPI::AuctionTransferAPI(const char *hostNames, const char *identifiers)
{
	std::vector<const char *> hostArray;
	std::vector<const char *> identifierArray;
	std::vector<short> portArray;
	char hostConfig[4096];
	char identifierConfig[4096];
	if (hostNames == nullptr) 
		hostNames = DEFAULT_HOST;
	if(identifiers == nullptr)
		identifiers = DEFAULT_IDENTIFIER;

	// parse the hosts and ports out :
	strncpy(hostConfig, hostNames, 4096); hostConfig[4095] = 0;
	char *ptr;
	if ((ptr = strtok(hostConfig, " ")) != 0)
	{
		do
		{
			char * host = ptr;
			char * portStr = strchr(host, ':');
			unsigned short port = DEFAULT_PORT;
			if (portStr)
			{
				*portStr++ = 0;
				port = (short)atoi(portStr);
			}

			if (::strlen(host) && port)
			{
				hostArray.push_back(host);
				portArray.push_back(port);
			}
		}
		while ((ptr = strtok(nullptr, " ")) != nullptr);
	}

	strncpy(identifierConfig, identifiers, 4096); identifierConfig[4095] = 0;
	// parse the identifiers out
	if ((ptr = strtok(identifierConfig, ";")) != 0)
	{
		do
		{
			char * identifier = ptr;
			if (::strlen(identifier))
			{
				identifierArray.push_back(identifier);
			}
		}
		while ((ptr = strtok(nullptr, ";")) != nullptr);
	}

	if (hostArray.empty())
	{
		hostArray.push_back(DEFAULT_HOST);
		portArray.push_back(DEFAULT_PORT);
	}
	if(identifierArray.empty())
		identifierArray.push_back(DEFAULT_IDENTIFIER);

	m_apiCore = new AuctionTransferAPICore(&hostArray[0], &portArray[0], hostArray.size(), this, &identifierArray[0], identifierArray.size());
}

//////////////////////////////////////////////////////////////////////////////////////
AuctionTransferAPI::~AuctionTransferAPI()
{
	delete m_apiCore;
}

//////////////////////////////////////////////////////////////////////////////////////
void AuctionTransferAPI::process()
{
	m_apiCore->process();
}

//////////////////////////////////////////////////////////////////////////////////////
//	Calls and replies
//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::replyReceiveAbortTransaction(unsigned trackingNumber, unsigned responseCode, void *user)
{
	ReplyRequest *req = new ReplyRequest( GAME_REPLY_RECEIVE_ABORT, trackingNumber, responseCode );
	return m_apiCore->submitRequest( req, new CommonResponse( GAME_REPLY_RECEIVE_ABORT, user ));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::replyReceiveCommitTransaction(unsigned trackingNumber, unsigned responseCode, void *user)
{
	ReplyRequest *req = new ReplyRequest( GAME_REPLY_RECEIVE_COMMIT, trackingNumber, responseCode );
	return m_apiCore->submitRequest( req, new CommonResponse( GAME_REPLY_RECEIVE_COMMIT, user ));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::replyReceivePrepareTransaction(unsigned trackingNumber, unsigned responseCode, void *user)
{
	ReplyRequest *req = new ReplyRequest( GAME_REPLY_RECEIVE_PREPARE_TRANSACTION, trackingNumber, responseCode );
	return m_apiCore->submitRequest( req, new CommonResponse( GAME_REPLY_RECEIVE_PREPARE_TRANSACTION, user ));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::sendAbortTransaction(long long transactionID, void *user)
{
	CommonRequest *req = new CommonRequest( GAME_REQUEST_SEND_ABORT, 0, transactionID );
	return m_apiCore->submitRequest( req, new CommonResponse( GAME_REQUEST_SEND_ABORT, user ));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::sendCommitTransaction(long long transactionID, void *user)
{
	CommonRequest *req = new CommonRequest( GAME_REQUEST_SEND_COMMIT, 0, transactionID );
	return m_apiCore->submitRequest( req, new CommonResponse( GAME_REQUEST_SEND_COMMIT, user ));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::sendPrepareTransaction(const char *serverIdentifier, long long transactionID, unsigned stationID, unsigned characterID, long long assetID, const char *xmlAsset, void *user, bool compress)
{
	RequestTypes requestEnum = GAME_REQUEST_SEND_PREPARE_TRANSACTION;
	GenericRequest *req = nullptr;
	if( compress )
	{
		requestEnum = GAME_REQUEST_SEND_PREPARE_TRANSACTION_COMPRESSED;
		// compress and create the request here
		char *data = (char *)xmlAsset;
		CA2GZIP zippedData(data, strlen(xmlAsset));
		req = new SendPrepareCompressedRequest( requestEnum, 0, serverIdentifier, transactionID, stationID, characterID, assetID, zippedData.pgzip, zippedData.Length );
	}
	else
	{
		req = new SendPrepareRequest( requestEnum, 0, serverIdentifier, transactionID, stationID, characterID, assetID, xmlAsset );
	}

	return m_apiCore->submitRequest( req, new CommonResponse( requestEnum, user ));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::sendPrepareTransactionCompressed (const char *serverIdentifier, long long transactionID, unsigned stationID, unsigned characterID, long long assetID, const unsigned char *zippedXmlAsset, unsigned length, void *user)
{
	SendPrepareCompressedRequest *req = new SendPrepareCompressedRequest( GAME_REQUEST_SEND_PREPARE_TRANSACTION_COMPRESSED, 0, serverIdentifier, transactionID, stationID, characterID, assetID, zippedXmlAsset, length );

	return m_apiCore->submitRequest( req, new CommonResponse( GAME_REQUEST_SEND_PREPARE_TRANSACTION_COMPRESSED, user ));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::replyReceiveGetCharacterList(unsigned trackingNumber, unsigned responseCode, const Character characters[], unsigned numCharacters, void *user)
{
	ReplyGetCharacterListRequest *req = new ReplyGetCharacterListRequest(GAME_REPLY_RECEIVE_GET_CHARACTER_LIST, trackingNumber, responseCode, characters, numCharacters);
	return m_apiCore->submitRequest(req, new CommonResponse(GAME_REPLY_RECEIVE_GET_CHARACTER_LIST, user));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::identifyHost( const char *serverID[], unsigned idCount, void *user)
{
	IdentifyServerRequest *req = new IdentifyServerRequest(REQUEST_SET_SERVER_LIST, 0, serverID, idCount);
	return m_apiCore->submitRequest( req, new CommonResponse(REQUEST_SET_SERVER_LIST, user));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::sendAuditAssetTransfer(const char *gameCode, const char *serverCode, long long inGameAssetID, unsigned stationID, const char *event, const char *message, void *user)
{
	SendAuditRequest *req = new SendAuditRequest( GAME_SEND_AUDIT_MESSAGE, 0, gameCode, serverCode, inGameAssetID, stationID, event, message );
	return m_apiCore->submitRequest(req, new CommonResponse(GAME_SEND_AUDIT_MESSAGE, user));
}

//////////////////////////////////////////////////////////////////////////////////////
unsigned AuctionTransferAPI::getNewTransactionID(void *user)
{
	GetIDRequest *req = new GetIDRequest( GAME_REQUEST_GET_TRANSACTION_ID, 0);
	return m_apiCore->submitRequest(req, new GetIDResponse( GAME_REQUEST_GET_TRANSACTION_ID, user));
}

}; // namespace
//////////////////////////////////////////////////////////////////////////////////////


