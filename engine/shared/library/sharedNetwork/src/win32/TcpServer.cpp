// TcpServer.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include <winsock2.h>
#include <mswsock.h>
#include "sharedNetwork/Address.h"
#include "sharedNetwork/Service.h"
#include "OverlappedTcp.h"
#include "TcpClient.h"
#include "TcpServer.h"

//-----------------------------------------------------------------------

TcpServer::TcpServer(Service * service, const std::string & bindAddress, const unsigned short bindPort) :
m_handle(),
m_localIOCP(),
m_pendingConnections(),
m_bindAddress(bindAddress),
m_bindPort(bindPort),
m_service(service)
{
	static PROTOENT * p = getprotobyname("tcp");
	if(p)
	{
		static int entry = p->p_proto;
		m_handle = WSASocket(AF_INET, SOCK_STREAM, entry, NULL, 0, WSA_FLAG_OVERLAPPED);
		if(m_handle != INVALID_SOCKET)
		{
			m_localIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_handle), 0, 0, 0);
			Address a(bindAddress, bindPort);

			int result = bind(m_handle, reinterpret_cast<const sockaddr *>(&a.getSockAddr4()), sizeof(struct sockaddr_in));
			if(result == 0)
			{
				struct sockaddr_in b;
				int addrlen = sizeof(struct sockaddr_in);
				getsockname(m_handle, (struct sockaddr *)(&b), &addrlen);
				m_bindPort = ntohs(b.sin_port);
				result = listen(m_handle, 256);
				queueAccept();
			}
		}
	}
}

//-----------------------------------------------------------------------

TcpServer::~TcpServer()
{
	closesocket(m_handle);
	CloseHandle(m_localIOCP);
}

//-----------------------------------------------------------------------

TcpClient * TcpServer::accept()
{
	TcpClient * result = 0;
	if(! m_pendingConnections.empty())
	{
		result = m_pendingConnections.back();
		m_pendingConnections.pop_back();
	}
	return result;
}

//-----------------------------------------------------------------------

const unsigned short TcpServer::getBindPort() const
{
	return m_bindPort;
}

//-----------------------------------------------------------------------

void TcpServer::onConnectionClosed(TcpClient *)
{
}

//-----------------------------------------------------------------------

void TcpServer::queueAccept()
{
	OverlappedTcp * op = getFreeOverlapped();

	// this will contain struct sockaddr data with additional
	// book keeping when Windows returns an overlapped 
	// accept operation (nevermind that +16 or * 2, I don't have the MS
	// kb article handy, but it's a gross workaround for the documented
	// API and what acceptData really expects to have).
	op->m_acceptData = new unsigned char[(sizeof( struct sockaddr_in ) + 16 ) * 2]; 
	op->m_operation = OverlappedTcp::ACCEPT;
	op->m_tcpServer = this;
	op->m_tcpClient = new TcpClient(m_localIOCP);
	AcceptEx(
		m_handle, 
		op->m_tcpClient->getSocket(),
		op->m_acceptData,
		0, 
		sizeof(struct sockaddr_in) + 16, 
		sizeof(struct sockaddr_in) + 16, 
		&op->m_bytes,
		&op->m_overlapped);
}

//-----------------------------------------------------------------------

void TcpServer::update()
{
	OVERLAPPED * overlapped = 0;
	OverlappedTcp * op = 0;
	unsigned long int bytesTransferred = 0;
	unsigned long int completionKey = 0;
	bool success = false;

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
				case OverlappedTcp::ACCEPT:
					{
						if(op->m_tcpServer == this)
						{
							if(op->m_acceptData != 0)
							{
								// Extremely lame hack to keep things safe from Winsock stacktrashing
								//struct sockaddr_in local;
								//struct sockaddr_in remote;
								//memcpy(&local, reinterpret_cast<struct sockaddr_in *>(op->m_acceptData + 10), sizeof(struct sockaddr_in));
								//memcpy(&remote, reinterpret_cast<struct sockaddr_in *>(op->m_acceptData + 38), sizeof(struct sockaddr_in));
								delete[] op->m_acceptData;
								TcpClient * newClient = op->m_tcpClient;
								newClient->addRef();
								newClient->onConnectionOpened();
								m_pendingConnections.push_back(newClient);
								success = true;
								queueAccept();
								if(m_service)
								{
									m_service->onConnectionOpened(newClient);
								}
								newClient->release();
							}
						}
					}
					break;
				case OverlappedTcp::RECV:
					{
						if(op->m_tcpClient != 0)
						{
							op->m_tcpClient->onReceive((const unsigned char * const)op->m_recvBuf.buf, bytesTransferred);
						}
					}
					success = true;
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

const std::string & TcpServer::getBindAddress() const
{
	return m_bindAddress;
}

//---------------------------------------------------------------------


