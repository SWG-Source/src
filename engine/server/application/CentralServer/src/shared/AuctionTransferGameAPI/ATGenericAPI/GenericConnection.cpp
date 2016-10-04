#include "GenericApiCore.h"
#include "GenericConnection.h"
#include "GenericMessage.h"
#include "AuctionTransferAPICore.h"

//----------------------------------------
//
// WARNING: These files are NOT standard generic API files
// They have been modified for this project.
// Do NOT replace them with generic API files
//
//----------------------------------------

#define REQUEST_SET_API 0
#define GAME_RESOURCE 1

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE
{
#endif

	using namespace std;
	using namespace Base;

	GenericConnection::GenericConnection(const char *host, short port, GenericAPICore *apiCore, unsigned reconnectTimeout, unsigned noDataTimeoutSecs, unsigned, unsigned incomingBufSizeInKB, unsigned outgoingBufSizeInKB, unsigned keepAlive, unsigned maxRecvMessageSizeInKB)
		: m_bConnected(CON_NONE),
		m_apiCore(apiCore),
		m_con(nullptr),
		m_host(host),
		m_port(port),
		m_conState(CON_DISCONNECT),
		m_reconnectTimeout(reconnectTimeout),
		m_conTimeout(0)
	{
		TcpManager::TcpParams params;

		params.incomingBufferSize = incomingBufSizeInKB * 1024;
		params.outgoingBufferSize = outgoingBufSizeInKB * 1024;
		params.maxConnections = 1;
		params.port = 0;
		params.maxRecvMessageSize = maxRecvMessageSizeInKB * 1024;
		params.keepAliveDelay = keepAlive * 1000;
		params.noDataTimeout = noDataTimeoutSecs * 1000;
		//params.oldestUnacknowledgedTimeout = noAckTimeoutSecs * 1000;

		m_manager = new TcpManager(params);
	}

	GenericConnection::~GenericConnection()
	{
		if (m_con)
		{
			m_con->SetHandler(nullptr);
			m_con->Disconnect();//don't worry about onterminated being called, we've set it's handler to nullptr, so it wont
			m_con->Release();
		}

		m_manager->Release();
	}

	void GenericConnection::disconnect()
	{
		if (m_con)
		{
			m_con->Disconnect();
			//no need to release, since callback to onTerminated releases it, and callback is allways made   m_con->Release();
			m_con = nullptr;
		}
		m_conState = CON_DISCONNECT;
		m_bConnected = CON_NONE;
	}

	void GenericConnection::OnTerminated(TcpConnection *)
	{
		//	m_apiCore->OnDisconnect(m_host.c_str(), m_port);
		m_apiCore->OnDisconnect(this);
		if (m_con)
		{
			m_con->Release();
			m_con = nullptr;
		}
		m_conState = CON_DISCONNECT;
		m_bConnected = CON_NONE;
	}

	void GenericConnection::OnRoutePacket(TcpConnection *, const unsigned char *data, int dataLen)
	{
		short type;
		unsigned track;

		ByteStream msg(data, dataLen);
		ByteStream::ReadIterator iter = msg.begin();

		get(iter, type);
		get(iter, track);
		GenericResponse *res = nullptr;

		if (track == 0)		// notification message from the server, not as a response to a request from this API
		{
			if (type == ATGAME_REQUEST_CONNECT)
			{	// this is a special case, for when we have identified our game code to server
				m_bConnected = CON_IDENTIFIED;
				m_apiCore->OnConnect(this);
			}
			else
			{
				m_apiCore->responseCallback(type, iter, this);
			}
		}
		else
		{
			map<unsigned, GenericResponse *>::iterator mapIter = m_apiCore->m_pending.find(track);

			if (mapIter != m_apiCore->m_pending.end())
			{
				res = (*mapIter).second;
				iter = msg.begin();
				res->unpack(iter);
				m_apiCore->responseCallback(res);
				m_apiCore->m_pendingCount--;
				m_apiCore->m_pending.erase(mapIter);
				delete res;
			}
		}
	}

	void GenericConnection::process()
	{
		switch (m_conState)
		{
		case CON_DISCONNECT:
			// create connection object, attempting to connect and
			// checking for connection in next state, CON_NEGOTIATE
			m_con = m_manager->EstablishConnection(m_host.c_str(), m_port);
			if (m_con)
			{
				m_con->SetHandler(this);
				m_conState = CON_NEGOTIATE;
				m_conTimeout = time(nullptr) + m_reconnectTimeout;
			}
			break;
		case CON_NEGOTIATE:
			// check for connection

			if (m_con->GetStatus() == TcpConnection::StatusConnected)
			{
				// we're connected
				m_conState = CON_CONNECT;
				m_bConnected = CON_CONNECTED;
				// instead of calling OnConnect() right now, we are going to submit a connection packet
				// identifying us
	//			m_apiCore->OnConnect(this);
				Base::ByteStream msg;
				put(msg, (short)REQUEST_SET_API);
				put(msg, (unsigned)0);	// track
				put(msg, (unsigned)API_VERSION_CODE);
				put(msg, GAME_RESOURCE);	// identify us as a game connection resource

				// now add in the game identifiers
				put(msg, (unsigned)m_apiCore->m_gameIdentifiers.size()); // number of strings to read
				for (unsigned index = 0; index < m_apiCore->m_gameIdentifiers.size(); index++)
					put(msg, std::string(m_apiCore->m_gameIdentifiers[index]));
				Send(msg);
			}
			else if (time(nullptr) > m_conTimeout)
			{
				// we did not connect
				m_con->Disconnect();
				//no need to release, since callback to onTerminated releases it, and callback is allways made   m_con->Release();
				m_con = nullptr;
				m_conState = CON_DISCONNECT;
				m_bConnected = CON_NONE;
			}
			break;
		case CON_CONNECT:
			// do nothing
			break;
		default:
			// this should not occur, but we revert to CON_DISCONNECT if it does
			m_conState = CON_DISCONNECT;
			m_bConnected = CON_NONE;
			if (m_con)
			{
				m_con->Disconnect();
				//no need to release, since callback to onTerminated releases it, and callback is allways made   m_con->Release();
				m_con = nullptr;
			}
		}
		m_manager->GiveTime();
	}

	void GenericConnection::Send(Base::ByteStream &msg)
	{
		if (m_con && m_con->GetStatus() == TcpConnection::StatusConnected)
		{
			m_con->Send((const char *)msg.getBuffer(), msg.getSize());
		}
	}

#ifdef EXTERNAL_DISTRO
};
#endif