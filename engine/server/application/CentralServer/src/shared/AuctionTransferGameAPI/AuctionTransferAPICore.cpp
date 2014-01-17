#include "AuctionTransferAPICore.h"
#include "AuctionTransferAPI.h"
#include "Response.h"

//////////////////////////////////////////////////////////////////////////////////////
namespace AuctionTransfer
{

/*/////////////////////////////////////////////////////////////////////////////////////
AuctionTransferAPICore::AuctionTransferAPICore(const char *hostName[], const short port[], int count, AuctionTransferAPI *api, const char *identifier[], unsigned identifierCount)
	: GenericAPICore(identifier[0], hostName, port, count, 45, 5, 0, 90, 32, 32), 
	m_api(api), 
	m_mappedServerTrack(1000)
	//////////////////////////////////////////////////////////////////////////////////////
{
	for(unsigned i = 0; i < identifierCount; i++)
		addIdentifier(identifier[i]);
}
*/
//////////////////////////////////////////////////////////////////////////////////////
AuctionTransferAPICore::AuctionTransferAPICore(const char *hostName[], const short port[], int count, AuctionTransferAPI *api, const char *identifier[], unsigned identifierCount, unsigned reqTimeout, unsigned maxRecvMessageSizeInKB)
	: GenericAPICore(identifier[0], hostName, port, count, reqTimeout, 5, 0, 90, 32, 32, 1, maxRecvMessageSizeInKB), 
	m_api(api), 
	m_mappedServerTrack(1000)
	//////////////////////////////////////////////////////////////////////////////////////
{
	for(unsigned i = 0; i < identifierCount; i++)
		addIdentifier(identifier[i]);
}

//////////////////////////////////////////////////////////////////////////////////////
AuctionTransferAPICore::~AuctionTransferAPICore()
//////////////////////////////////////////////////////////////////////////////////////
{
}

//////////////////////////////////////////////////////////////////////////////////////
void AuctionTransferAPICore::OnConnect(GenericConnection *connection)
//////////////////////////////////////////////////////////////////////////////////////
{
	countOpenConnections();
	m_api->onConnect(connection->getHost(), connection->getPort(), (short)m_currentConnections, (short)m_maxConnections);
}

//////////////////////////////////////////////////////////////////////////////////////
void AuctionTransferAPICore::OnDisconnect(GenericConnection *connection)
//////////////////////////////////////////////////////////////////////////////////////
{
	countOpenConnections();
	m_api->onDisconnect(connection->getHost(), connection->getPort(), (short)m_currentConnections, (short)m_maxConnections);
}

//////////////////////////////////////////////////////////////////////////////////////
void AuctionTransferAPICore::responseCallback(GenericResponse *response)
//////////////////////////////////////////////////////////////////////////////////////
// these callbacks are a result of a call initiated by the game server.
//////////////////////////////////////////////////////////////////////////////////////
{
	switch(response->getType())
	{
		case GAME_REPLY_RECEIVE_PREPARE_TRANSACTION:
			m_api->onReplyReceivePrepareTransaction(response->getTrack(), 
													response->getResult(), 
													response->getUser());
			break;
		case GAME_REPLY_RECEIVE_COMMIT:
			m_api->onReplyReceiveCommitTransaction(	response->getTrack(), 
													response->getResult(), 
													response->getUser());
			break;
		case GAME_REPLY_RECEIVE_ABORT:
			m_api->onReplyReceiveAbortTransaction(	response->getTrack(), 
													response->getResult(), 
													response->getUser());
			break;
		case GAME_REQUEST_SEND_PREPARE_TRANSACTION:
			m_api->onSendPrepareTransaction(response->getTrack(), 
											response->getResult(), 
											response->getUser());
			break;
		case GAME_REQUEST_SEND_PREPARE_TRANSACTION_COMPRESSED:
			m_api->onSendPrepareTransactionCompressed(response->getTrack(), 
				response->getResult(), 
				response->getUser());
			break;
		case GAME_REQUEST_SEND_COMMIT:
			m_api->onSendCommitTransaction(	response->getTrack(), 
											response->getResult(), 
											response->getUser());
			break;
		case GAME_REQUEST_SEND_ABORT:
			m_api->onSendAbortTransaction(	response->getTrack(), 
											response->getResult(), 
											response->getUser());
			break;
		case GAME_REPLY_RECEIVE_GET_CHARACTER_LIST:
			m_api->onReplyReceiveGetCharacterList(	response->getTrack(), 
													response->getResult(), 
													response->getUser());
			break;
		case GAME_SEND_AUDIT_MESSAGE:
			m_api->onSendAuditAssetTransfer(	response->getTrack(), 
												response->getResult(), 
												response->getUser());
			break;
		case REQUEST_SET_SERVER_LIST:
			m_api->onIdentifyHost( response->getTrack(),
									response->getResult(),
									response->getUser());
			break;
		case GAME_REQUEST_GET_TRANSACTION_ID:
		{
			GetIDResponse *r = static_cast<GetIDResponse *>(response);
			m_api->onGetNewTransactionID( r->getTrack(),
										  r->getResult(),
										  r->getNewID(),
										  r->getUser());
		}
		break;
		default:
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void AuctionTransferAPICore::responseCallback(short type, Base::ByteStream::ReadIterator &iter, GenericConnection *connection)
//////////////////////////////////////////////////////////////////////////////////////
// these are the callbacks that are initiated from the Auction System.  These callbacks
// will be called to initiate a series of communication between the auction system and
// the game servers.
//////////////////////////////////////////////////////////////////////////////////////
{
	unsigned real_server_track;
	get(iter, real_server_track);
	ServerTrackObject *stobj = new ServerTrackObject(++m_mappedServerTrack, real_server_track, connection);
	m_serverTracks.insert(std::pair<unsigned, ServerTrackObject *>(m_mappedServerTrack, stobj));

	long long transactionID;
	get(iter, transactionID);
	switch (type)
	{
		case GAME_REQUEST_RECEIVE_PREPARE_TRANSACTION:
		{
			unsigned stationID;
			unsigned characterID;
			long long assetID;
			std::string newName;
			get(iter, stationID);
			get(iter, characterID);
			get(iter, assetID);
			get(iter, newName);
			m_api->onReceivePrepareTransaction(m_mappedServerTrack, transactionID, stationID, characterID, assetID, newName.c_str());
			break;
		}
		case GAME_REQUEST_RECEIVE_COMMIT:
		{
			m_api->onReceiveCommitTransaction(m_mappedServerTrack, transactionID);
			break;
		}
		case GAME_REQUEST_RECEIVE_ABORT:
		{
			m_api->onReceiveAbortTransaction(m_mappedServerTrack, transactionID);
			break;
		}
		case GAME_REQUEST_RECEIVE_GET_CHARACTER_LIST:
		{
			// this had the stationID read in as the transactionID
			std::string serverID;
			get(iter, serverID);
			m_api->onReceiveGetCharacterList(m_mappedServerTrack, (unsigned int)transactionID, serverID.c_str());
		}
		default:
			break;
	}
}



}; // namespace AuctionTransfer
//////////////////////////////////////////////////////////////////////////////////////

