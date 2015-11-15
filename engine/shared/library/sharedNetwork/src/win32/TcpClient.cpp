// TcpClient.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedFoundation/Clock.h"
#include "sharedNetwork/Address.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/Service.h"
#include "OverlappedTcp.h"
#include "TcpClient.h"
#include <set>
#include <vector>

//-----------------------------------------------------------------------

const unsigned long KEEPALIVE_MS = 1000;

//-----------------------------------------------------------------------

namespace TcpClientNamespace
{
	std::set<TcpClient *> s_pendingConnectionSends;
	std::set<TcpClient *> s_pendingConnectionRemoves;
	std::set<TcpClient *> s_tcpClients;
	QOS                   s_sqos;
	QOS                   s_gqos;
}

using namespace TcpClientNamespace;

//-----------------------------------------------------------------------

TcpClient::TcpClient(HANDLE parentIOCP) :
m_connectEvent(INVALID_HANDLE_VALUE),
m_socket(),
m_tcpServer(0),
m_localIOCP(INVALID_HANDLE_VALUE),
m_pendingSend(),
m_connection(0),
m_refCount(0),
m_connected(false),
m_ownHandle(false),
m_lastSendTime(0),
m_bindPort(0),
m_rawTCP( false )
{
	static PROTOENT * p = getprotobyname ("tcp");
	if (p)
	{
		static int entry = p->p_proto;
		m_socket = WSASocket (AF_INET, SOCK_STREAM, entry, NULL, 0, WSA_FLAG_OVERLAPPED);
		char optval = 1;
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
		unsigned long opt = 1;
		ioctlsocket(m_socket, FIONBIO, &opt);

		struct sockaddr_in bindAddr;
		int addrLen = sizeof(struct sockaddr_in);
		if(getsockname(m_socket, reinterpret_cast<struct sockaddr *>(&bindAddr), &addrLen) == 0)
		{
			m_bindPort = ntohs(bindAddr.sin_port);
		}

		m_localIOCP = CreateIoCompletionPort (reinterpret_cast<HANDLE> (m_socket), parentIOCP, 0, 0);
		queueReceive();
	}
	s_tcpClients.insert(this);
}

//-----------------------------------------------------------------------

TcpClient::TcpClient(const std::string & remoteAddress, const unsigned short remotePort) :
m_connectEvent(INVALID_HANDLE_VALUE),
m_socket(),
m_localIOCP(INVALID_HANDLE_VALUE),
m_connection(0),
m_refCount(0),
m_connected(false),
m_ownHandle(true),
m_lastSendTime(0),
m_rawTCP( false )
{
	static PROTOENT * p = getprotobyname ("tcp");
	if (p)
	{
		static int entry = p->p_proto;
		m_socket = WSASocket (AF_INET, SOCK_STREAM, entry, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_socket != INVALID_SOCKET)
		{
			int nameLen = sizeof (struct sockaddr_in);
			static WSABUF emptyBuf;
			emptyBuf.buf = 0;
			emptyBuf.len = 0;
			
			Address a(remoteAddress, remotePort);
			char optval = 1;
			setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
			setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
			unsigned long opt = 1;
			ioctlsocket(m_socket, FIONBIO, &opt);

			int result;
			result = WSAConnect (m_socket, reinterpret_cast<const struct sockaddr *> (&a.getSockAddr4 () ), nameLen, 0, &emptyBuf, &s_sqos, &s_gqos);
			m_connectEvent = WSACreateEvent ();
			WSAEventSelect (m_socket, m_connectEvent, FD_CONNECT);
			m_localIOCP = CreateIoCompletionPort (reinterpret_cast<HANDLE> (m_socket), 0, 0, 0);
			struct sockaddr_in bindAddr;
			int addrLen = sizeof(struct sockaddr_in);
			if(getsockname(m_socket, reinterpret_cast<struct sockaddr *>(&bindAddr), &addrLen) == 0)
			{
				m_bindPort = ntohs(bindAddr.sin_port);
			}

		}
	}
	s_tcpClients.insert(this);
}

//-----------------------------------------------------------------------

TcpClient::~TcpClient()
{
	s_pendingConnectionRemoves.insert(this);
	std::set<TcpClient *>::iterator f = s_tcpClients.find(this);
	if(f != s_tcpClients.end())
		s_tcpClients.erase(f);

	closesocket (m_socket);
	if(m_ownHandle)
		CloseHandle(m_localIOCP);
}

//-----------------------------------------------------------------------

void TcpClient::addRef()
{
	m_refCount++;
}

//-----------------------------------------------------------------------

unsigned short TcpClient::getBindPort() const
{
	return m_bindPort;
}

//-----------------------------------------------------------------------

std::string const &TcpClient::getRemoteAddress() const
{
	// TODO: implement this
	static std::string dummy;
	return dummy;
}

//-----------------------------------------------------------------------

unsigned short TcpClient::getRemotePort() const
{
	// TODO: implement this
	return 0;
}

//-----------------------------------------------------------------------

void TcpClient::commit(const unsigned char * const buffer, const int bufferLen)
{
	WSABUF wsaBuf;

	// yuck, docs say this is actually going to be const for the send, 
	// but WSABUF::buf is not const!
	wsaBuf.buf =  (char *)buffer; 
	wsaBuf.len = bufferLen;

	OverlappedTcp * op = getFreeOverlapped ();
	if (op)
	{
		op->m_operation = OverlappedTcp::SEND;
		op->m_tcpClient = const_cast<TcpClient *>(this);
		int sent;
		sent = WSASend (m_socket, &wsaBuf, 1, &op->m_bytes, 0, &op->m_overlapped, NULL);
		if(sent == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			char * err;
			if(errCode != WSA_IO_PENDING)
			{
				switch(errCode)
				{
				case WSANOTINITIALISED:
					err = "A successful WSAStartup must occur before using this function.";
					break;
				case WSAENETDOWN:
					err = "The network subsystem has failed.";
					break;
				case WSAENOTCONN:
					err = "The socket is not connected.";
					break;
				case WSAEINTR:
					err = "The (blocking) call was canceled through WSACancelBlockingCall. \n";
					break;
				case WSAEINPROGRESS:
					err = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. \n";
					break;
				case WSAENETRESET:
					err = "The connection has been broken due to \"keep-alive\" activity detecting a failure while the operation was in progress.";
					break;
				case WSAENOTSOCK:
					err = "The descriptor is not a socket.";
					break;
				case WSAEFAULT:
					err = "The lpBuffers parameter is not completely contained in a valid part of the user address space.";
					break;
				case WSAEOPNOTSUPP:
					err = "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, out-of-band data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.";
					break;
				case WSAESHUTDOWN:
					err = "The socket has been shut down; it is not possible to call WSARecv on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH. \n";
					break;
				case WSAEWOULDBLOCK:
					err = "Overlapped sockets: There are too many outstanding overlapped I/O requests. Nonoverlapped sockets: The socket is marked as nonblocking and the receive operation cannot be completed immediately. \n";
					break;
				case WSAEMSGSIZE:
					err = "The message was too large to fit into the specified buffer and (for unreliable protocols only) any trailing portion of the message that did not fit into the buffer has been discarded.";
					break;
				case WSAEINVAL:
					err = "The socket has not been bound (for example, with bind).";
					break;
				case WSAECONNABORTED:
					err = "The virtual circuit was terminated due to a time-out or other failure. \n";
					break;
				case WSAECONNRESET:
					err = "The virtual circuit was reset by the remote side. \n";
					break;
				case WSAEDISCON:
					err = "Socket s is message oriented and the virtual circuit was gracefully closed by the remote side. \n";
					break;
				case WSA_IO_PENDING:
					err = "An overlapped operation was successfully initiated and completion will be indicated at a later time. \n";
					break;
				case WSA_OPERATION_ABORTED:
					err = "The overlapped operation has been canceled due to the closure of the socket. \n";
					break;
				default:
					err = "An unknown error occured while processing WSARecv().";
					break;
				}
				onConnectionClosed();
			}		
		}
	}
}

//-----------------------------------------------------------------------

void TcpClient::flush()
{
	if(m_connected && m_pendingSend.getSize() > 0)
	{
		// put it on the wire
		commit(m_pendingSend.getBuffer(), m_pendingSend.getSize());
		m_pendingSend.clear ();
	}
}

//-----------------------------------------------------------------------

void TcpClient::flushPendingWrites()
{
	std::set<TcpClient *>::iterator f;
	std::set<TcpClient *>::iterator i;
	for(i = s_pendingConnectionSends.begin(); i != s_pendingConnectionSends.end(); ++i)
	{
		if(s_pendingConnectionRemoves.empty())
		{
			(*i)->flush();
		}
		else
		{
			f = s_pendingConnectionRemoves.find((*i));
			if(f == s_pendingConnectionRemoves.end())
				(*i)->flush();
		}
	}
	s_pendingConnectionSends.clear();
}

//-----------------------------------------------------------------------

SOCKET TcpClient::getSocket() const
{
	return m_socket;
}

//-----------------------------------------------------------------------

void TcpClient::install()
{
	WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
	s_sqos.ProviderSpecific.buf = 0;
	s_sqos.ProviderSpecific.len = 0;
	
	s_sqos.ReceivingFlowspec.DelayVariation = 0;
	s_sqos.ReceivingFlowspec.Latency = 0;
	s_sqos.ReceivingFlowspec.MaxSduSize = 0;
	s_sqos.ReceivingFlowspec.MinimumPolicedSize = 0;
	s_sqos.ReceivingFlowspec.PeakBandwidth = 0;
	s_sqos.ReceivingFlowspec.ServiceType = 0;
	s_sqos.ReceivingFlowspec.TokenBucketSize = 0;
	s_sqos.ReceivingFlowspec.TokenRate = 0;
	
	s_sqos.SendingFlowspec.DelayVariation = 0;
	s_sqos.SendingFlowspec.Latency = 0;
	s_sqos.SendingFlowspec.MaxSduSize = 0;
	s_sqos.SendingFlowspec.MinimumPolicedSize = 0;
	s_sqos.SendingFlowspec.PeakBandwidth = 0;
	s_sqos.SendingFlowspec.ServiceType = 0;
	s_sqos.SendingFlowspec.TokenBucketSize = 0;
	s_sqos.SendingFlowspec.TokenRate = 0;
	
	s_gqos.ProviderSpecific.buf = 0;
	s_gqos.ProviderSpecific.len = 0;
	
	s_gqos.ReceivingFlowspec.DelayVariation = 0;
	s_gqos.ReceivingFlowspec.Latency = 0;
	s_gqos.ReceivingFlowspec.MaxSduSize = 0;
	s_gqos.ReceivingFlowspec.MinimumPolicedSize = 0;
	s_gqos.ReceivingFlowspec.PeakBandwidth = 0;
	s_gqos.ReceivingFlowspec.ServiceType = 0;
	s_gqos.ReceivingFlowspec.TokenBucketSize = 0;
	s_gqos.ReceivingFlowspec.TokenRate = 0;
	
	s_gqos.SendingFlowspec.DelayVariation = 0;
	s_gqos.SendingFlowspec.Latency = 0;
	s_gqos.SendingFlowspec.MaxSduSize = 0;
	s_gqos.SendingFlowspec.MinimumPolicedSize = 0;
	s_gqos.SendingFlowspec.PeakBandwidth = 0;
	s_gqos.SendingFlowspec.ServiceType = 0;
	s_gqos.SendingFlowspec.TokenBucketSize = 0;
	s_gqos.SendingFlowspec.TokenRate = 0;
}

//-----------------------------------------------------------------------

void TcpClient::onConnectionClosed()
{
	m_connected = false;
	if(m_connection)
	{
		NetworkHandler::onTerminate(m_connection);
	}
}

//-----------------------------------------------------------------------

void TcpClient::onConnectionOpened()
{
	m_connected = true;
	queueReceive();
	flush();
	if(m_connection)
		m_connection->onConnectionOpened();
}

//-----------------------------------------------------------------------

void TcpClient::onReceive(const unsigned char * const buffer, const int length)
{
	queueReceive();
	if(m_connection)
	{
		m_connection->receive(buffer, length);
	}
}

//-----------------------------------------------------------------------

void TcpClient::queryConnect()
{
	bool result = false;

	WSANETWORKEVENTS w;
	if (WSAEnumNetworkEvents (m_socket, m_connectEvent, &w) != SOCKET_ERROR)
	{
		if (w.lNetworkEvents == FD_CONNECT)
		{
  			if (w.iErrorCode[FD_CONNECT_BIT] == 0)
  			{
				CloseHandle (m_connectEvent);
				result = true;
				onConnectionOpened();
  			}
			else
			{
				onConnectionClosed();
			}
		}
	}
}

//-----------------------------------------------------------------------

void TcpClient::queueReceive()
{
	OverlappedTcp * op = getFreeOverlapped();
	op->m_operation = OverlappedTcp::RECV;
	op->m_tcpClient = const_cast<TcpClient *>(this);
	DWORD flags = 0;
	int result;
	result = WSARecv(m_socket, &op->m_recvBuf, 1, &op->m_bytes, &flags, &op->m_overlapped, NULL);
	
	if(result == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		char * err;
		if(errCode != WSA_IO_PENDING)
		{
			switch(errCode)
			{
			case WSANOTINITIALISED:
				err = "A successful WSAStartup must occur before using this function.";
				break;
			case WSAENETDOWN:
				err = "The network subsystem has failed.";
				break;
			case WSAENOTCONN:
				err = "The socket is not connected.";
				break;
			case WSAEINTR:
				err = "The (blocking) call was canceled through WSACancelBlockingCall. \n";
				break;
			case WSAEINPROGRESS:
				err = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. \n";
				break;
			case WSAENETRESET:
				err = "The connection has been broken due to \"keep-alive\" activity detecting a failure while the operation was in progress.";
				break;
			case WSAENOTSOCK:
				err = "The descriptor is not a socket.";
				break;
			case WSAEFAULT:
				err = "The lpBuffers parameter is not completely contained in a valid part of the user address space.";
				break;
			case WSAEOPNOTSUPP:
				err = "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, out-of-band data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.";
				break;
			case WSAESHUTDOWN:
				err = "The socket has been shut down; it is not possible to call WSARecv on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH. \n";
				break;
			case WSAEWOULDBLOCK:
				err = "Overlapped sockets: There are too many outstanding overlapped I/O requests. Nonoverlapped sockets: The socket is marked as nonblocking and the receive operation cannot be completed immediately. \n";
				break;
			case WSAEMSGSIZE:
				err = "The message was too large to fit into the specified buffer and (for unreliable protocols only) any trailing portion of the message that did not fit into the buffer has been discarded.";
				break;
			case WSAEINVAL:
				err = "The socket has not been bound (for example, with bind).";
				break;
			case WSAECONNABORTED:
				err = "The virtual circuit was terminated due to a time-out or other failure. \n";
				break;
			case WSAECONNRESET:
				err = "The virtual circuit was reset by the remote side. \n";
				break;
			case WSAEDISCON:
				err = "Socket s is message oriented and the virtual circuit was gracefully closed by the remote side. \n";
				break;
			case WSA_IO_PENDING:
				err = "An overlapped operation was successfully initiated and completion will be indicated at a later time. \n";
				break;
			case WSA_OPERATION_ABORTED:
				err = "The overlapped operation has been canceled due to the closure of the socket. \n";
				break;
			default:
				err = "An unknown error occured while processing WSARecv().";
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------

void TcpClient::send(const unsigned char * const buffer, const int length)
{
	if (length)
	{
		m_lastSendTime = Clock::getFrameStartTimeMs();
		s_pendingConnectionSends.insert(this);
		if( !m_rawTCP )
			Archive::put(m_pendingSend, length);
		m_pendingSend.put(buffer, length);

		static int const tcpMinimumFrame = ConfigSharedNetwork::getTcpMinimumFrame();
		if (static_cast<int>(m_pendingSend.getSize()) >= tcpMinimumFrame)
			flush();
	}
}

//-----------------------------------------------------------------------

void TcpClient::setConnection(Connection * c)
{
	m_connection = c;
}

//---------------------------------------------------------------------

void TcpClient::update()
{
	OVERLAPPED * overlapped = 0;
	OverlappedTcp * op = 0;
	unsigned long int bytesTransferred = 0;
	unsigned long int completionKey = 0;
	bool success = false;

	if (m_connected)
	{
		unsigned long timeNow = Clock::getFrameStartTimeMs();
		if (timeNow-m_lastSendTime > KEEPALIVE_MS)
		{
			m_lastSendTime = timeNow;
			s_pendingConnectionSends.insert(this);
			Archive::put(m_pendingSend, 0);
		}
	}

	if(! m_connected)
		queryConnect();

	//PlatformTcpClient::queryConnects();
	do
	{
		success = false;
		int ok = GetQueuedCompletionStatus(
			m_localIOCP, // completion port of interest
			&bytesTransferred, // number of bytes sent or received
			&completionKey,
			&overlapped,
			0 // timeout immediately if there are no completions
			);
		if(ok)
		{
			op = reinterpret_cast<OverlappedTcp *>(overlapped);
			if(op) 
			{
				switch(op->m_operation)
				{
				case OverlappedTcp::RECV:
					{
						if(op->m_tcpClient != 0)
						{
							if(bytesTransferred > 0)
							{
								op->m_tcpClient->onReceive((const unsigned char * const)op->m_recvBuf.buf, bytesTransferred);
								success = true;
							}
						}
					}
					
					break;
				case OverlappedTcp::SEND:
					success = true;
					break;
				default:
					break;
				}
				releaseOverlapped(op);
			}
		}
	} while(success);
}

//-----------------------------------------------------------------------

void TcpClient::setRawTCP( bool bNewValue )
{
	m_rawTCP = bNewValue;
}

//-----------------------------------------------------------------------

void TcpClient::release()
{
	m_refCount--;
	if(m_refCount < 1)
	{
		if(m_connected)
			onConnectionClosed();
		delete this;
	}
}

//-----------------------------------------------------------------------

void TcpClient::remove()
{
	std::set<TcpClient *>::iterator i;
	for(i = s_tcpClients.begin(); i != s_tcpClients.end(); ++i)
	{
		TcpClient * c = (*i);
		delete c;
	}
	s_tcpClients.clear();
	WSACleanup();
}

//-----------------------------------------------------------------------

void TcpClient::checkKeepalive()
{
	unsigned long const timeNow = Clock::getFrameStartTimeMs();
	if (timeNow-m_lastSendTime > KEEPALIVE_MS)
	{
		m_lastSendTime = timeNow;
		s_pendingConnectionSends.insert(this);
		Archive::put(m_pendingSend, 0);
	}
}

//-----------------------------------------------------------------------

