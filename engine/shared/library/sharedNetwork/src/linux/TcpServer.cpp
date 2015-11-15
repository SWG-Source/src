// TcpServer.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//---------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "TcpServer.h"

#include "sharedNetwork/Connection.h"
#include "sharedNetwork/Service.h"
#include "TcpClient.h"
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>

//---------------------------------------------------------------------

TcpServer::TcpServer(Service * service, const std::string & a, const unsigned short port) :
m_bindAddress(a, port),
m_handle(-1),
m_service(service),
m_connections(),
m_connectionSockets(),
m_inputBuffer(0),
m_inputBufferSize(0)
{
	protoent * p = getprotobyname("tcp");
	if(p)
	{
		int entry = p->p_proto;
		m_handle = socket(AF_INET, SOCK_STREAM, entry);
		FATAL(m_handle == -1, ("Failed to create a server socket %s:%d", a.c_str(), port));
		int optval = 1;
		if(m_handle != -1)
		{
			int optResult = setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
			FATAL(optResult == -1, ("Failed to set socket option SO_REUSEADDR"));
			unsigned long nb = 1;
			int ctlResult = ioctl(m_handle, FIONBIO, &nb);
			FATAL(ctlResult == -1, ("Failed to set socket non-blocking"));
			socklen_t addrlen = sizeof(struct sockaddr_in);
			int bindResult = bind(m_handle, (struct sockaddr *)(&m_bindAddress.getSockAddr4()), addrlen);
			FATAL(bindResult != 0, ("Failed to bind to port %d", port));
			if(bindResult == 0)
			{
				int result = listen(m_handle, 256);
				if(result == -1)
					perror("listen ");
				FATAL(result == -1, ("Failed to start listening on port %d", port));
				struct sockaddr_in b;
				getsockname(m_handle, (struct sockaddr *)(&b), &addrlen);
				m_bindAddress = b;
			}
		}
	}
}

//---------------------------------------------------------------------

TcpServer::~TcpServer()
{
	close(m_handle);
	for (std::map<int, TcpClient *>::const_iterator i = m_connections.begin(); i != m_connections.end(); ++i)
		(*i).second->clearTcpServer();
}

//---------------------------------------------------------------------

const std::string & TcpServer::getBindAddress() const
{
	return m_bindAddress.getHostAddress();
}

//---------------------------------------------------------------------

const unsigned short TcpServer::getBindPort() const
{
	return m_bindAddress.getHostPort();
}

//---------------------------------------------------------------------

void TcpServer::onConnectionClosed(TcpClient * c)
{
	if(c)
	{
		std::set<TcpClient *>::iterator f = m_pendingDestroys.find(c);
		if(f == m_pendingDestroys.end())
		{
			m_pendingDestroys.insert(c);
		}
	}
}

//---------------------------------------------------------------------
void TcpServer::removeClient(TcpClient * c)
{
	std::map<int, TcpClient *>::iterator f = m_connections.find(c->getSocket());
	if(f != m_connections.end())
	{
//		c->release();
		m_connections.erase(f);
	}
	
	std::vector<pollfd>::iterator s;
	for(s = m_connectionSockets.begin(); s != m_connectionSockets.end(); ++s)
	{
		if((*s).fd == c->getSocket())
		{
			m_connectionSockets.erase(s);
			break;
		}
	}
}

//---------------------------------------------------------------------

void TcpServer::update()
{
	struct pollfd pfd;
	pfd.fd = m_handle;
	pfd.events = POLLIN;
	pfd.revents = 0;

	int result = poll(&pfd, 1, 0);
	if(result > 0)
	{
		// connection established
		struct sockaddr_in addr;
		socklen_t len = sizeof(struct sockaddr_in);
		int newSock = accept(m_handle, (struct sockaddr *)(&addr), &len);
		if(newSock != -1)
		{
			TcpClient * ptc = new TcpClient(newSock, this);
			ptc->addRef();
			m_connections.insert(std::make_pair(newSock, ptc));
			struct pollfd readFd;
			readFd.fd = ptc->getSocket();
			readFd.events = POLLIN|POLLERR|POLLHUP;
			readFd.revents = 0;
			m_connectionSockets.push_back(readFd);
			ptc->onConnectionOpened();
			if(m_service)
				m_service->onConnectionOpened(ptc);
			ptc->release();
		}
	}

	struct pollfd * clients = &m_connectionSockets[0];
	
	int readResult = poll (clients, m_connectionSockets.size (), 0);
	if (readResult > 0)
	{
		std::vector<struct pollfd>::iterator i;
		std::vector<struct pollfd> cs = m_connectionSockets;
		for (i = cs.begin (); i != cs.end (); ++i)
		{
			if ((*i).revents & POLLERR)
			{
				std::map<int, TcpClient *>::iterator f = m_connections.find ( (*i).fd );
				if (f != m_connections.end ())
				{
					TcpClient * c = (*f).second;
					if (c->m_connection)
						c->m_connection->setDisconnectReason("TcpServer::update POLLERR");
					c->onConnectionClosed();
				}
			}
			// POLLERR is mutually exclusive with POLLIN and POLLHUP.
			// POLLIN and POLLHUP are not consistent cross-platform.  Additionally,
			// a POLLHUP doesn't mean that there's not data available.  The best,
			// cross-platform way to check for disconnection is to see if either
			// POLLHUP or POLLIN are set, and then read from the socket.
			// In cases where there's data, we'll just end up checking it next
			// frame anyway.  In cases where there's no data, it means we've
			// disconnected so we can handle cleanup.
			else if( (*i).revents & (POLLIN|POLLHUP) )
			{	

				if (m_inputBuffer == 0)
				{
					m_inputBuffer = new unsigned char[1500];
					m_inputBufferSize = 1500;
				}
				int bytesReceived = recv ( (*i).fd, m_inputBuffer, m_inputBufferSize, 0);
				std::map<int, TcpClient *>::iterator f = m_connections.find ( (*i).fd );
				if (f != m_connections.end ())
				{
					TcpClient * c = (*f).second;
					std::set<TcpClient *>::iterator pd = m_pendingDestroys.find(c);
					if(pd == m_pendingDestroys.end())
					{
						c->addRef();
						if(bytesReceived > 0)
						{
							c->onReceive(m_inputBuffer, bytesReceived);
						}
						else if (bytesReceived == -1)
						{
							if (c->m_connection)
								c->m_connection->setDisconnectReason("TcpServer::update recv returned %d, errno=%d", bytesReceived, errno);
							c->onConnectionClosed();
						}
						else if (bytesReceived == 0)
						{
							// 0 bytes received on a read is the guaranteed signal for a
							// closed connection.
							if (c->m_connection)
								c->m_connection->setDisconnectReason("TcpServer::update recv returned no bytes, singaling a closed socket (bytes=%d, errno=%d", bytesReceived, errno);
							c->onConnectionClosed();
						}
						c->release();
					}
				}
				if (bytesReceived == m_inputBufferSize)
				{
					delete [] m_inputBuffer;
					m_inputBufferSize = m_inputBufferSize * 2;
					m_inputBuffer = new unsigned char [m_inputBufferSize];
				}
			}
		}
		std::set<TcpClient *>::iterator pdIter;
		for(pdIter = m_pendingDestroys.begin(); pdIter != m_pendingDestroys.end(); ++pdIter)
		{
			removeClient((*pdIter));
		}
		m_pendingDestroys.clear();
	}
}

//---------------------------------------------------------------------

