#include "GenericAPI/GenericApiCore.h"
#include "GenericAPI/GenericConnection.h"
#include "GenericAPI/GenericMessage.h"

using namespace UdpLibrary;

#ifdef USE_SERIALIZE_LIB
#include <Base/serialize.h>
#endif

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE
{
#endif
	namespace GenericAPI
	{
		using namespace std;
		using namespace Base;

		unsigned GenericConnection::ms_crcBytes = 0;

		GenericConnection::GenericConnection(const char *host, short port, GenericAPICore *apiCore, unsigned reconnectTimeout, unsigned noDataTimeoutSecs, unsigned noAckTimeoutSecs, unsigned incomingBufSizeInKB, unsigned outgoingBufSizeInKB, unsigned keepAlive, unsigned maxRecvMessageSizeInKB, unsigned holdTime)
			: m_bConnected(false),
			m_apiCore(apiCore),
			m_con(nullptr),
			m_host(host),
			m_nextHost(host),
			m_port(port),
			m_nextPort(port),
			m_lastTrack(123455), //random choice != 1
			m_conState(CON_DISCONNECT),
			m_reconnectTimeout(reconnectTimeout),
			m_conTimeout(100)
		{
#ifdef USE_TCP_LIBRARY
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
#else //default to UDP_LIBRARY
			UdpManager::Params params;

			params.keepAliveDelay = keepAlive * 1000;
			params.maxDataHoldTime = holdTime;
			params.incomingBufferSize = incomingBufSizeInKB * 1024;
			params.outgoingBufferSize = outgoingBufSizeInKB * 1024;
			params.maxConnections = 1;
			params.port = 0;
			params.noDataTimeout = noDataTimeoutSecs * 1000;
			params.oldestUnacknowledgedTimeout = noAckTimeoutSecs * 1000;
			params.crcBytes = ms_crcBytes;

			m_manager = new UdpManager(&params);
#endif //USE_TCP_LIBRARY
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

		void GenericConnection::changeHostPort(const char *host, short port)
		{
			if (host &&
				strcmp(host, "") != 0)
			{
				m_nextHost = host;
				m_nextPort = port;
			}
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
			m_bConnected = false;
		}

#ifdef USE_TCP_LIBRARY
		void GenericConnection::OnTerminated(TcpConnection *con)
#else //default to UDP_LIBRARY
		void GenericConnection::OnTerminated(UdpConnection *con)
#endif
		{
			m_apiCore->OnDisconnect(m_host.c_str(), m_port);
			if (m_con)
			{
				m_con->Release();
				m_con = nullptr;
			}
			m_conState = CON_DISCONNECT;
			m_bConnected = false;
		}

#ifdef USE_TCP_LIBRARY
		void GenericConnection::OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen)
#else //default to UDP_LIBRARY
		void GenericConnection::OnRoutePacket(UdpConnection *con, const unsigned char *data, int dataLen)
#endif
		{
			short type;
			unsigned track;

#ifdef USE_SERIALIZE_LIB
			unsigned bytes = 0;
			unsigned fieldLen = soe::Read(data, dataLen, type);
			if (fieldLen == 0)
				return;//invalid message
			bytes += fieldLen;

			fieldLen = soe::Read(data + bytes, dataLen - bytes, track);
			if (fieldLen == 0)
				return;//invalid message
			bytes += fieldLen;
#else
			ByteStream msg(data, dataLen);
			ByteStream::ReadIterator iter = msg.begin();

			get(iter, type);
			get(iter, track);
#endif
			GenericResponse *res = nullptr;

			// the following if block is a temporary fix that prevents
			// a crash with a game team in which they occasionally find
			// themselves receiving a dupe track in consecutive calls to
			// OnRoutePacket (which then leads to a callback being called
			// twice and data being invalid on the second call -> crash!).
			if (track != 0 &&
				track == m_lastTrack)
			{
				printf("!!! ERROR !!! Got a duplicate track ID %u\n", track);
				return;
			}
			m_lastTrack = track;

			// end temporary fix.

			if (track == 0)
			{
#ifdef USE_SERIALIZE_LIB
				m_apiCore->responseCallback(type, data + bytes, dataLen - bytes);
#else
				m_apiCore->responseCallback(type, iter);
#endif
			}
			else
			{
				map<unsigned, GenericResponse *>::iterator mapIter = m_apiCore->m_pending.find(track);

				if (mapIter != m_apiCore->m_pending.end())
				{
					res = (*mapIter).second;
#ifdef USE_SERIALIZE_LIB
					res->unpack(data, dataLen);
#else
					iter = msg.begin();
					res->unpack(iter);
#endif
					m_apiCore->m_pending.erase(mapIter);
					m_apiCore->m_pendingCount--;
					m_apiCore->responseCallback(res);
					delete res;
				}
			}
		}

		void GenericConnection::process(bool giveTime)
		{
			switch (m_conState)
			{
			case CON_DISCONNECT:
				// if host/port was changed, it takes effect here
				m_host = m_nextHost;
				m_port = m_nextPort;

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

#ifdef USE_TCP_LIBRARY
				if (m_con->GetStatus() == TcpConnection::StatusConnected)
#else //default to UDP_LIBRARY
				if (m_con->GetStatus() == UdpConnection::cStatusConnected)
#endif
				{
					// we're connected
					m_conState = CON_CONNECT;
					m_apiCore->OnConnect(m_host.c_str(), m_port);
					m_bConnected = true;
				}
				else if (time(nullptr) > m_conTimeout)
				{
					// we did not connect
					m_con->Disconnect();
					//no need to release, since callback to onTerminated releases it, and callback is allways made   m_con->Release();
					m_con = nullptr;
					m_conState = CON_DISCONNECT;
					m_bConnected = false;
				}
				break;
			case CON_CONNECT:
				// do nothing
				break;
			default:
				// this should not occur, but we revert to CON_DISCONNECT if it does
				m_conState = CON_DISCONNECT;
				m_bConnected = false;
				if (m_con)
				{
					m_con->Disconnect();
					//no need to release, since callback to onTerminated releases it, and callback is allways made   m_con->Release();
					m_con = nullptr;
				}
			}

			if (giveTime)
			{
				m_manager->GiveTime();
			}
		}

#ifdef USE_SERIALIZE_LIB
		void GenericConnection::Send(const unsigned char *data, int dataLen)
		{
#ifdef USE_TCP_LIBRARY
			if (m_con && m_con->GetStatus() == TcpConnection::StatusConnected)
			{
				m_con->Send((const char *)data, dataLen);
			}
#else//USE_TCP_LIBRARY
			if (m_con && m_con->GetStatus() == UdpConnection::cStatusConnected)
			{
				m_con->Send(cUdpChannelReliable1, data, dataLen);
			}
#endif//USE_TCP_LIBRARY
		}
#else //USE_SERIALIZE_LIB
		void GenericConnection::Send(Base::ByteStream &msg)
		{
#ifdef USE_TCP_LIBRARY
			if (m_con && m_con->GetStatus() == TcpConnection::StatusConnected)
			{
				m_con->Send((const char *)msg.getBuffer(), msg.getSize());
			}
#else //USE_TCP_LIBRARY
			if (m_con && m_con->GetStatus() == UdpConnection::cStatusConnected)
			{
				m_con->Send(cUdpChannelReliable1, msg.getBuffer(), msg.getSize());
			}
#endif//USE_TCP_LIBRARY
		}
#endif //USE_SERIALIZE_LIB
	};

#ifdef EXTERNAL_DISTRO
};
#endif