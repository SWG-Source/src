//---------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "TcpClient.h"
#include "TcpServer.h"

#include "Archive/Archive.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedLog/NetLogConnection.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include <map>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <set>
#include <vector>

//-----------------------------------------------------------------------

const unsigned long KEEPALIVE_MS = 1000;

//-----------------------------------------------------------------------

namespace TcpClientNamespace
{
	std::set<TcpClient *> s_pendingConnectionSends;
	std::set<TcpClient *> s_tcpClients;
	bool                  s_installed = false;
}

using namespace TcpClientNamespace;

//---------------------------------------------------------------------

TcpClient::TcpClient (int sockHandle, TcpServer * server) :
m_socket(sockHandle),
m_tcpServer(server),
m_pendingSend(),
m_connection(0),
m_recvBuffer(0),
m_recvBufferLength(0),
m_remoteAddress(),
m_refCount(0),
m_connected (true),
m_lastSendTime(0),
m_bindPort(0),
m_rawTCP( false )
{
	s_tcpClients.insert(this);
	setSockOptions();

	struct sockaddr_in target;
	socklen_t namelen = sizeof(struct sockaddr_in);
	if (getpeername(sockHandle, reinterpret_cast<sockaddr *>(&target), &namelen) == 0)
		m_remoteAddress = target;
	struct sockaddr_in bindAddr;

	if(getsockname(sockHandle, reinterpret_cast<struct sockaddr *>(&bindAddr), &namelen) == 0)
	{
		m_bindPort = ntohs(bindAddr.sin_port);
	}
}

// ----------------------------------------------------------------------

TcpClient::TcpClient (const std::string & a, const unsigned short port) :
m_socket(-1),
m_tcpServer(0),
m_connection(0),
m_recvBuffer(0),
m_recvBufferLength(0),
m_remoteAddress(a, port),
m_refCount(0),
m_connected (false),
m_lastSendTime(0),
m_bindPort(0),
m_rawTCP( false )
{
	FATAL(! s_installed, ("TcpClient is not installed!"));

	s_tcpClients.insert(this);
	setupSocket();
}

//---------------------------------------------------------------------

TcpClient::~TcpClient()
{
	FATAL(m_refCount > 0, ("Attempting to destroy a TcpClient while references still exist!"));
	std::set<TcpClient *>::iterator f = s_tcpClients.find(this);
	if(f != s_tcpClients.end())
		s_tcpClients.erase(f);
	f = s_pendingConnectionSends.find(this);
	if(f != s_pendingConnectionSends.end())
		s_pendingConnectionSends.erase(f);
	
	delete[] m_recvBuffer;
	if(m_socket != -1)
		close (m_socket);
}

//---------------------------------------------------------------------

void TcpClient::addRef()
{
	m_refCount++;
}

//---------------------------------------------------------------------
std::string const &TcpClient::getRemoteAddress() const
{
	return m_remoteAddress.getHostAddress();
}

//---------------------------------------------------------------------

unsigned short TcpClient::getRemotePort() const
{
	return m_remoteAddress.getHostPort();
}

//---------------------------------------------------------------------

void TcpClient::setSockOptions()
{
	int optval = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
	ioctl(m_socket, FIONBIO, &optval);
}

//---------------------------------------------------------------------


void TcpClient::commit()
{
	int sent = 0;
	const unsigned char * ptr = m_pendingSend.getBuffer();
	const unsigned char * const end = ptr + m_pendingSend.getSize();
	static Archive::ByteStream s_localScratchBuffer;
	s_localScratchBuffer.clear();
	int retries = 0;
	static const int maxRetries = ConfigSharedNetwork::getMaxTCPRetries();
	bool sentSomething = false;
	
	do
	{
		// attempt to push pending data to socket
		sent = ::send (m_socket, ptr, end - ptr, MSG_NOSIGNAL);
		if(sent == -1)
		{
			// failed
			switch(errno)
			{
				// because it would block on a non-blocking socket
				case EAGAIN:
				case ENOBUFS:
				case ENOMEM:
				case EINTR:
				{
					retries++;
					// requeue
					break;
				}
				// an unhandled error (connection reset, for example)
				default:
				{
					m_pendingSend.clear();
					perror("send: ");
					if (m_connection)
						m_connection->setDisconnectReason("TcpClient::commit send returned -1, errno=%d", errno);
					onConnectionClosed();
					return;
					break;
				}				
			}
		}
		else if(sent > 0)
		{
			sentSomething=true;
			ptr += sent;
			if(sent < end - ptr)
			{
				// data was passed to the socket, but not all
				// of it could be committed, keep trying!
				retries = 0;
				Os::sleep(1);
			}
		}
		else if(sent == 0)
		{
			// tried to send and got nothing through (should have returned an error, but be thorough)
			retries ++;
			Os::sleep(1);
		}
		
	} while(ptr < end && retries < maxRetries);

	if(ptr == end)
	{
		// all data was committed, clear pending buffer
		m_pendingSend.clear();
	}
	else
	{
		// bailed out before all data was committed, save unsent data for
		// next pass on commit()
		if (sentSomething)
		{
			s_localScratchBuffer.put(ptr, end - ptr);
			m_pendingSend.clear();
			m_pendingSend.put(s_localScratchBuffer.getBuffer(), s_localScratchBuffer.getSize());
		}
		s_pendingConnectionSends.insert(this);
	}

	// clean up local accounting
	s_localScratchBuffer.clear();
}

//---------------------------------------------------------------------

void TcpClient::flush()
{
	if (m_connected && m_pendingSend.getSize() > 0)
	{
		// flush pending buffer
		commit();
	}
}

//-----------------------------------------------------------------------

void TcpClient::flushPendingWrites()
{
	std::set<TcpClient *> sends = s_pendingConnectionSends;
	s_pendingConnectionSends.clear();

	for (std::set<TcpClient *>::const_iterator i = sends.begin(); i != sends.end(); ++i)
	{
		TcpClient * const c = *i;

		if (c->m_connected)
		{
			c->flush();

			if (   c->m_connection
			    && c->m_pendingSend.getSize()
			    && ConfigSharedNetwork::getLogSendingTooMuchData()
			    && !dynamic_cast<NetLogConnection const *>(c->m_connection))
			{
				WARNING(true, ("(%s) Failed to commit %d bytes this frame to %s:%d. This process is sending more data than the remote can receive!", c->m_connection->getConnectionDescription().c_str(), c->m_pendingSend.getSize(), c->getRemoteAddress().c_str(), c->getRemotePort()));
				LOG("Network", ("(%s) Failed to commit %d bytes this frame to %s:%d. This process is sending more data than the remote can receive!", c->m_connection->getConnectionDescription().c_str(), c->m_pendingSend.getSize(), c->getRemoteAddress().c_str(), c->getRemotePort()));
			}
		}
	}
}

//---------------------------------------------------------------------

unsigned short TcpClient::getBindPort() const
{
	return m_bindPort;
}

//---------------------------------------------------------------------

int TcpClient::getSocket() const
{
	return m_socket;
}

//---------------------------------------------------------------------

void TcpClient::install()
{
	FATAL(s_installed, ("TcpClient is already installed!"));
	s_installed = true;
	signal(SIGPIPE, SIG_IGN);
}

//---------------------------------------------------------------------

void TcpClient::onConnectionClosed()
{
	if (m_connection)
		m_connection->setDisconnectReason("TcpClient::onConnectionClosed called");
	m_connected = false;
	shutdown(m_socket, SHUT_RDWR);
	if(m_tcpServer)
	{
		m_tcpServer->removeClient(this);
	}
	close(m_socket);
	m_socket = -1;
	if(m_connection)
	{
		NetworkHandler::onTerminate(m_connection);
	}
}

//---------------------------------------------------------------------

void TcpClient::onConnectionOpened()
{
	// Get the local port this socket was bound to. If
	// a client socket repeatedly attempts to connect to a local
	// service that should be listening on a port that the operating
	// system may assign AND the service isn't actually running and
	// using the port, the client socket may connect to itself.
	// For example, A client connecting to 127.0.0.1:50005, the operating
	// system may assign the client socket port 50005 and the connection
	// succeeds, but it is in fact a connection to itself, meaning the
	// client isn't actually talking to any valid service.
	struct sockaddr_in bindAddr;
	socklen_t addrLen = sizeof(struct sockaddr_in);
	if(getsockname(m_socket, reinterpret_cast<struct sockaddr *>(&bindAddr), &addrLen) == 0)
	{
		m_bindPort = ntohs(bindAddr.sin_port);
	}

	// this *might* be a valid connection to a remote address, but
	// to easily catch all cases of loopback connections, attempt a
	// reconnect if the remote port is identical to the local port
	if(m_remoteAddress.getHostPort() == m_bindPort && ! m_tcpServer)
	{
		shutdown(m_socket, SHUT_RDWR);
		DEBUG_FATAL(m_tcpServer, ("Detected a loopback client connection on a TCP SERVER!!!"));
		close(m_socket);
		m_socket = -1;

		// reconnect the socket
		// address and port info is initialized in the constructor
		setupSocket();
		LOG("Network", ("Detected a loopback, closing socket and reconnecting"));
	}
	else
	{
		m_connected = true;
		if(m_connection)
		{
			m_connection->onConnectionOpened();
		}
		flush();
	}
}

//---------------------------------------------------------------------

void TcpClient::onReceive (const unsigned char * const recvBuf, const int bytes)
{
	if(m_connection)
	{
		m_connection->receive(recvBuf, bytes);
	}
}

//---------------------------------------------------------------------

void TcpClient::queryConnect ()
{
	struct pollfd pfd;
	pfd.fd = m_socket;
	pfd.events = POLLOUT;
	pfd.revents = 0;
	int pollResult = poll(&pfd, 1, 0);
	if(pollResult > 0)
	{
		if (pfd.revents & POLLERR)
		{
			if (m_connection)
				m_connection->setDisconnectReason("TcpClient::queryConnect POLLERR");
			onConnectionClosed();
		}
		else if (pfd.revents & POLLHUP)
		{
			if (m_connection)
				m_connection->setDisconnectReason("TcpClient::queryConnect POLLHUP");
			onConnectionClosed();
		}
		else
		{
			onConnectionOpened();
		}
	}
}

//-----------------------------------------------------------------------

void TcpClient::release()
{
	m_refCount--;
	if(m_refCount == 0)
	{
		if (m_connected)
		{
			if (m_connection)
				m_connection->setDisconnectReason("TcpClient::release called");
			onConnectionClosed();
		}
		delete this;
	}
}

//-----------------------------------------------------------------------

void TcpClient::remove()
{
	FATAL(!s_installed, ("TcpClient already removed!"));
	std::set<TcpClient *>::iterator i;
	for(i = s_tcpClients.begin(); i != s_tcpClients.end(); ++i)
	{
		TcpClient * c = (*i);
		c->release();
	}
	s_installed = false;
}

//---------------------------------------------------------------------

void TcpClient::send(const unsigned char * const buffer, const int bufferLen)
{
	if (bufferLen)
	{
		m_lastSendTime = Clock::getFrameStartTimeMs();
		s_pendingConnectionSends.insert(this);
		if( !m_rawTCP )
			Archive::put(m_pendingSend, bufferLen);
		m_pendingSend.put(buffer, bufferLen);

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

void TcpClient::checkKeepalive()
{
	if (m_connected)
	{
		unsigned long const timeNow = Clock::getFrameStartTimeMs();
		if (timeNow-m_lastSendTime > KEEPALIVE_MS)
		{
			m_lastSendTime = timeNow;
			s_pendingConnectionSends.insert(this);
			Archive::put(m_pendingSend, 0);
		}
	}
}

//---------------------------------------------------------------------

void TcpClient::update()
{
	addRef();
	if(!m_connected)
	{
		queryConnect();
	}

	if(m_connected)
	{
		struct pollfd pfd;
		pfd.fd = m_socket;
		pfd.events = POLLIN|POLLHUP;
		pfd.revents = 0;
		int pollResult = poll (&pfd, 1, 0);
		// POLLERR is mutually exclusive with POLLIN and POLLHUP.
		// POLLIN and POLLHUP are not consistent cross-platform.  Additionally,
		// a POLLHUP doesn't mean that there's not data available.  The best,
		// cross-platform way to check for disconnection is to see if either
		// POLLHUP or POLLIN are set, and then read from the socket.
		// In cases where there's data, we'll just end up checking it next
		// frame anyway.  In cases where there's no data, it means we've
		// disconnected so we can handle cleanup.
		if (pollResult)
		{
			if (m_recvBuffer == nullptr)
			{
				m_recvBufferLength = 1500;
				m_recvBuffer = new unsigned char [m_recvBufferLength];
			}
			int bytesReceived = recv(m_socket, m_recvBuffer, m_recvBufferLength, 0);
			if (bytesReceived > 0)
			{
				onReceive(m_recvBuffer, bytesReceived);
			}
			else if (bytesReceived == -1)
			{
				switch (errno)
				{
					case EAGAIN:
					case EINTR:
						break;
					default:
						if (m_connection)
							m_connection->setDisconnectReason("TcpClient::update recv returned -1, errno=%d", errno);
						onConnectionClosed();
						break;
				}
			}

			else if (bytesReceived == 0)
			{
				// peer closed connection
				


				if (m_connection)
				{
				// Log this, as it's a pretty deep change.
					LOG("Network", ("(%s) Read zero bytes on a successful poll (POLLIN|POLLHUP).  Closing socket since we think it is closed.", m_connection->getConnectionDescription().c_str()));				
					m_connection->setDisconnectReason("TcpClient::update recv returned 0");
				}
				else
				{
					LOG("Network", ("(nullptr connection object) Read zero bytes on a successful poll (POLLIN|POLLHUP).  Closing socket since we think it is closed."));				
				}
				onConnectionClosed();
			}

			if (bytesReceived == m_recvBufferLength)
			{
				// grow the buffer
				delete [] m_recvBuffer;
				m_recvBufferLength = m_recvBufferLength * 2;
				m_recvBuffer = new unsigned char [m_recvBufferLength];
			}
		}
	}
	release();
}

// ----------------------------------------------------------------------

void TcpClient::setupSocket()
{
	FATAL(! s_installed, ("TcpClient is not installed!"));

	protoent * p = getprotobyname ("tcp");
	const int entry = p->p_proto;
	m_socket = socket (AF_INET, SOCK_STREAM, entry);
	if (m_socket != -1)
	{
		setSockOptions();

		int nameLen = sizeof (struct sockaddr_in);
		int result;
		result = connect (m_socket, reinterpret_cast<const struct sockaddr *> (&m_remoteAddress.getSockAddr4 () ), nameLen);
		if(result == -1)
		{
			switch(errno)
			{
				case EINPROGRESS:
				case EALREADY:
					break;
				default:
					perror("connect ");
					if (m_connection)
						m_connection->setDisconnectReason("TcpClient::TcpClient connect returned -1, errno=%d", errno);
					onConnectionClosed();
					break;
			}
		}
		else
		{
			struct sockaddr_in bindAddr;
			socklen_t addrLen = sizeof(struct sockaddr_in);
			if(getsockname(m_socket, reinterpret_cast<struct sockaddr *>(&bindAddr), &addrLen) == 0)
			{
				m_bindPort = ntohs(bindAddr.sin_port);
			}
		}
	}
}

// ----------------------------------------------------------------------
void TcpClient::setRawTCP( bool bNewValue )
{
	m_rawTCP = bNewValue;
}

// ----------------------------------------------------------------------

void TcpClient::clearTcpServer()
{
	m_tcpServer = 0;
}

// ======================================================================

