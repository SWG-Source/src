#include "TcpManager.h"
#include <assert.h>
#include "IPAddress.h"
#include "TcpConnection.h"
#include <time.h>

#ifndef WIN32
    #include <sys/poll.h>
#endif


#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

const time_t DNS_TIMEOUT = 60 * 5;

TcpManager::TcpParams::TcpParams()
: port(0),
/*
  maxConnections(1000),
  incomingBufferSize(6*1024),
  outgoingBufferSize(64*1024),
  allocatorBlockSize(8*1024),
  allocatorBlockCount(16),
  maxRecvMessageSize(0),
  keepAliveDelay(0),
  noDataTimeout(0)
  */
  maxConnections(1000),
  incomingBufferSize(512*1024),
  outgoingBufferSize(512*1024),
  allocatorBlockSize(8*1024),
  allocatorBlockCount(1024),
  maxRecvMessageSize(2048*1024),
  keepAliveDelay(0),
  noDataTimeout(0)

{
    memset(bindAddress, 0, sizeof(bindAddress));
}

TcpManager::TcpParams::TcpParams(const TcpParams &cpy)
: port(cpy.port),
  maxConnections(cpy.maxConnections),
  incomingBufferSize(cpy.incomingBufferSize),
  outgoingBufferSize(cpy.outgoingBufferSize),
  allocatorBlockSize(cpy.allocatorBlockSize),
  allocatorBlockCount(cpy.allocatorBlockCount),
  maxRecvMessageSize(cpy.maxRecvMessageSize),
  keepAliveDelay(cpy.keepAliveDelay),
  noDataTimeout(cpy.noDataTimeout)
{
    memset(bindAddress, 0, sizeof(bindAddress));
    strncpy(bindAddress, cpy.bindAddress, sizeof(cpy.bindAddress));
}

TcpManager::TcpManager(const TcpParams &params)
: m_handler(nullptr),
  m_keepAliveList(nullptr, 1),
  m_aliveList(nullptr, 2),
  m_noDataList(nullptr, 1),
  m_dataList(nullptr, 2),
  m_params(params),
  m_refCount(1),
  m_connectionList(nullptr),
  m_connectionListCount(0),
  m_socket(INVALID_SOCKET),
  m_boundAsServer(false),
  m_allocator(params.allocatorBlockSize, params.allocatorBlockCount),
  m_keepAliveTimer(),
  m_noDataTimer(),
  m_dnsMap()
{
    if (params.keepAliveDelay > 0)
        m_keepAliveTimer.start();

    if (params.noDataTimeout > 0)
        m_noDataTimer.start();

#if defined(WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(1,1), &wsaData);

    FD_ZERO(&m_permfds);//select only used on win32
#endif

}


TcpManager::~TcpManager()
{
#if defined(WIN32)
    WSACleanup();
#endif

    if (m_boundAsServer)
    {
#if defined(WIN32)
        closesocket(m_socket);
#else
        close(m_socket);
#endif
    }
    while (m_connectionList != nullptr)
    {
        TcpConnection *con = m_connectionList;
        con->AddRef();
        removeConnection(con);
        con->Release();
    }
}


bool TcpManager::BindAsServer()
{

    m_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_socket != INVALID_SOCKET)
    {
#if defined(WIN32)
        FD_SET(m_socket, &m_permfds);//the socket this server is listening on

        unsigned long isNonBlocking = 1;
        int outBufSize = m_params.outgoingBufferSize;
        int inBufSize = m_params.incomingBufferSize;
        int keepAlive = 1;
        int reuseAddr = 1;
        struct linger ld;
        ld.l_onoff = 0;
        ld.l_linger = 0;

        if (ioctlsocket(m_socket, FIONBIO, &isNonBlocking) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char *)&outBufSize, sizeof(outBufSize)) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char *)&inBufSize, sizeof(inBufSize)) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepAlive, sizeof(keepAlive)) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseAddr, sizeof(reuseAddr)) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof(ld)) != 0 )
        {
            return false;
        }
#else    // linux is to remain the default compile mode
        unsigned long isNonBlocking = 1;
        unsigned long keepAlive = 1;
        unsigned long outBufSize = m_params.outgoingBufferSize;
        unsigned long inBufSize = m_params.incomingBufferSize;
        unsigned long reuseAddr = 1;
        struct linger ld;
        ld.l_onoff = 0;
        ld.l_linger = 0;

        if (ioctl(m_socket, FIONBIO, &isNonBlocking) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &outBufSize, sizeof(outBufSize)) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &inBufSize, sizeof(inBufSize)) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive)) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) != 0
            || setsockopt(m_socket, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) != 0)
        {
            return false;
        }
#endif
    }
    else
    {
        return false;
    }


    struct sockaddr_in addr_loc;
    addr_loc.sin_family = AF_INET;
    addr_loc.sin_port = htons(m_params.port);
    addr_loc.sin_addr.s_addr = htonl(INADDR_ANY);
    if (m_params.bindAddress[0] != 0)
    {
        unsigned long address = inet_addr(m_params.bindAddress);
        if (address == INADDR_NONE)
        {
            struct hostent * lphp;
            lphp = gethostbyname(m_params.bindAddress);
            if (lphp != nullptr)
                addr_loc.sin_addr.s_addr = ((struct in_addr *)(lphp->h_addr))->s_addr;
        }
        else
        {
            addr_loc.sin_addr.s_addr = address;
        }
    }

    if (bind(m_socket, (struct sockaddr *)&addr_loc, sizeof(addr_loc)) != 0)
    {
        return false;
    }

    if (listen(m_socket, 1000) != 0)
    {
        return false;
    }

    m_boundAsServer = true;
    return true;
}

TcpConnection *TcpManager::acceptClient()
{
    TcpConnection *newConn = nullptr;
    
    if (m_boundAsServer && m_connectionListCount < m_params.maxConnections)
    {

        sockaddr_in addr;
        int addrLength = sizeof(addr);
        SOCKET sock = ::accept(m_socket, (sockaddr *) &addr, (socklen_t *) &addrLength);


        if (sock != INVALID_SOCKET)
        {
            newConn = new TcpConnection(this, &m_allocator, m_params, sock, IPAddress(addr.sin_addr.s_addr), ntohs(addr.sin_port));
            addNewConnection(newConn);
            if (m_handler != nullptr)
            {
                m_handler->OnConnectRequest(newConn);
            }
        }
    }

    return newConn;
}

void TcpManager::SetHandler(TcpManagerHandler *handler)
{
    m_handler = handler;
}

SOCKET TcpManager::getMaxFD()
{
#ifdef WIN32
    return 0;//this param is not used on win32 for select, only on unix
#else
    SOCKET maxfd = 0;
    
    if (m_boundAsServer)
        maxfd = m_socket+1;
    
    TcpConnection *next = nullptr;
    for (TcpConnection *con = m_connectionList ; con != nullptr ; con = next)
    {
        next = con->m_nextConnection;
        if (con->GetStatus() != TcpConnection::StatusDisconnected && con->m_socket > maxfd)
        {
            maxfd = con->m_socket + 1;
        }
    }
    return maxfd;
#endif
}

TcpConnection *TcpManager::getConnection(SOCKET fd)
{
    TcpConnection *next = nullptr;
    for (TcpConnection *con = m_connectionList ; con != nullptr ; con = next)
    {
        next = con->m_nextConnection;
        if (con->m_socket == fd)
        {
            return con;
        }
    }
    //if get here ,couldn't find it
    return nullptr;
}

bool TcpManager::GiveTime(unsigned maxTimeAcceptingConnections,unsigned maxSendTimePerConnection, unsigned maxRecvTimePerConnection)
{
    bool processedIncoming = false;

    if (maxTimeAcceptingConnections == 0 && maxSendTimePerConnection==0 && maxRecvTimePerConnection==0)
    {
        //they don't want to do anything now
        return processedIncoming;
    }

    AddRef(); //keep a reference to ourself in case we callback to the application and the application releases us.



    //first process outgoing on each connection, and finish establishing connections, if params say to
    if (m_connectionListCount != 0 && maxSendTimePerConnection != 0)
    {

        // Send output from last heartbeat
        TcpConnection *next = nullptr;
        for (TcpConnection *con = m_connectionList ; con != nullptr ; con = next)
        {
			con->AddRef();
			if (next) next->Release();
            next = con->m_nextConnection;
			if (next) next->AddRef();
            if(con->GetStatus() == TcpConnection::StatusConnected)
            {
                Clock timer;
                timer.start();
                while(!timer.isDone(maxSendTimePerConnection))
                {
                    int err = con->processOutgoing();

                    if (err > 0)
                    {
                        //couldn't finish processing last request, don't try more
                        break;
                    }
                    else if (err < 0)
                    {
                        break;
                    }
                }
				con->Release();
            }
            else if (con->GetStatus() == TcpConnection::StatusNegotiating)
            {
                if (con->finishConnect() < 0)
                {
					con->Release();
                    continue;
                }
				con->Release();
            }
            else //inactive client in client list????
            {
                removeConnection(con);
				con->Release();
                continue;
            }
        }

    }

    //process incoming messages (including connect requests)
    if ((
            m_boundAsServer  //if in server mode and want to spend time accepting clients
            &&  maxTimeAcceptingConnections != 0
        )
        || 
        (
            m_connectionListCount != 0  //if there are connections and want to spend time receiving on them
            && maxRecvTimePerConnection != 0
        )
       )
    {
#ifdef WIN32 
        SOCKET maxfd = getMaxFD(); //re-calc maxfd every time select on WIN32

        //select on all fd's
        struct timeval timeout;

        fd_set tmpfds;
        tmpfds = m_permfds;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        int cnt = select(maxfd, &tmpfds, nullptr, nullptr, &timeout);    // blocks for timeout


        if (cnt > 0) 
        {
            if (m_boundAsServer && maxTimeAcceptingConnections != 0)
            {//activity on our socket means connect requests
                
                //see if are new incoming clients
                if (FD_ISSET(m_socket, &tmpfds))
                {
                    //yep
                    Clock timer;
                    timer.start();
                    while (acceptClient() && !timer.isDone(maxTimeAcceptingConnections))
                    {
                        //loop
                    }
                }
            }



            //process incoming client messages
            if (maxRecvTimePerConnection != 0)
            {
                TcpConnection *next = nullptr;
                for (TcpConnection *con = m_connectionList ; con != nullptr ; con = next)
                {
					con->AddRef();
					if (next) next->Release();
                    next = con->m_nextConnection;
					if (next) next->AddRef();

                    SOCKET fd = con->m_socket;
                    if (fd == INVALID_SOCKET)
                    {
                        //invalid socket in list?, check if is connecting, otherwise, Disconnect and discard
                        if (con->GetStatus() != TcpConnection::StatusNegotiating)
                        {
                            removeConnection(con);
							con->Release();
							continue;
                        }
                    }

                    if (FD_ISSET(fd, &tmpfds)) 
                    {
                        Clock timer;
                        timer.start();
                        while(!timer.isDone(maxRecvTimePerConnection) && con->GetStatus() == TcpConnection::StatusConnected)
                        {
                            int err = con->processIncoming();
                            if (err >= 0)
                            {
                                processedIncoming = true;
                            }

                            if (err > 0)
                            {
                                //couldn't finish processing last request, don't try more
                                break;
                            }
                            else if (err < 0)
                            {
                                break;
                            }

                        }//while(!timer...)
                    }//if (FD_ISSET...)
					con->Release();
                }//for (...)
            } //maxRecvTimePerConnection != 0
        }//cnt > 0
#else //on UNIX use poll

        int numfds = m_connectionListCount;
        int idx = 0;
        if (m_boundAsServer)
        {
            numfds++;
            idx++;
        }

        struct pollfd pollfds[numfds];

        if (m_boundAsServer)
        {
            pollfds[0].fd = m_socket;
            pollfds[0].events |= POLLIN;
        }

        TcpConnection *next = nullptr;
        for (TcpConnection *con = m_connectionList ; con != nullptr ; con = next, idx++)
        {
            next = con->m_nextConnection;
            pollfds[idx].fd = con->m_socket;
            pollfds[idx].events |= POLLIN;
            pollfds[idx].events |= POLLHUP;
        }
    

        int cnt = poll(pollfds, numfds, 1);

        if(cnt == SOCKET_ERROR)
        {
            //poll not working?
            //TODO:  need to notify client somehow, don't think we can assume a fatal error here
        }
        else if (cnt > 0) 
        {
            for (idx = 0; idx < numfds; idx++)
            {
                //find corresponding TcpConnection
                //TODO:  optimize, seriously, this is takes linear time, every time
                TcpConnection *con = getConnection(pollfds[idx].fd);

                if (pollfds[idx].revents & POLLIN)
                {
                    if (m_boundAsServer && maxTimeAcceptingConnections != 0 && pollfds[idx].fd == m_socket)
                    {
                        //new incoming clients
                        Clock timer;
                        timer.start();
                        while (acceptClient() && !timer.isDone(maxTimeAcceptingConnections))
                        {
                            //loop
                        }

                        continue;//don't try to readmsgs from listening fd
                    }
                    
                    //process regular msg(s)
                    if (con == nullptr)
                    {
                        close(pollfds[idx].fd);
                        continue;
                    }

                    Clock timer;
                    timer.start();
                    con->AddRef();//so it can't get deleted while we are checking it's status
                    while(!timer.isDone(maxRecvTimePerConnection) && con->GetStatus() == TcpConnection::StatusConnected)
                    {
                        int err = con->processIncoming();
                        if (err >= 0)
                        {
                            processedIncoming = true;
                        }

                        if (err > 0)
                        {
                            //couldn't finish processing last request, don't try more
                            break;
                        }
                        else if (err < 0)
                        {
                            break;
                        }

                    }//while(!timer....)
                    con->Release();
                }//if(pollfds[...
                else if (pollfds[idx].revents & POLLHUP)
                {
                    if (con == nullptr)
                    {
                        close(pollfds[idx].fd);
                        continue;
                    }

                    //Disconnect client
                    con->Disconnect();
                }
            }//for (idx=0....
        }//else if (cnt > 0)

#endif
    }//wanted to process incoming messages or connect requests

    //now process any keepalives, if time to do that
    if (m_params.keepAliveDelay > 0 && m_keepAliveTimer.isDone(m_params.keepAliveDelay))
    {
        TcpConnection *next = nullptr;
        for (TcpConnection *con = m_keepAliveList.m_beginList ; con != nullptr ; con = next)
        {
			con->AddRef();
			if (next) next->Release();
            next = con->m_nextKeepAliveConnection;
			if (next) next->AddRef();
            
            con->Send(nullptr, 0); //note:  this request will move the connection from the keepAliveList to the aliveList
			con->Release();
        }

        //now move the complete alive list over to the keepalive list to reset those timers
        m_keepAliveList.m_beginList = m_aliveList.m_beginList;
        m_aliveList.m_beginList = nullptr;

        //switch id's for those connections that were in the alive list last go - around
        int tmpID = m_aliveList.m_listID;
        m_aliveList.m_listID = m_keepAliveList.m_listID;
        m_keepAliveList.m_listID = tmpID;

        m_keepAliveTimer.reset();
        m_keepAliveTimer.start();
    }

    //now process any noDataCons, if time to do that
    if (m_params.noDataTimeout > 0 && m_noDataTimer.isDone(m_params.noDataTimeout))
    {
        TcpConnection *next = nullptr;
        for (TcpConnection *con = m_noDataList.m_beginList ; con != nullptr ; con = next)
        {
			con->AddRef();
			if (next) next->Release();
            next = con->m_nextRecvDataConnection;
			if (next) next->AddRef();

            //time to disconnect this guy
            con->Disconnect();
			con->Release();
        }

        //now move the complete data list over to the nodata list to reset those timers
        m_noDataList.m_beginList = m_dataList.m_beginList;
        m_dataList.m_beginList = nullptr;

        //switch id's for those connections that were in the data list last go - around
        int tmpID = m_dataList.m_listID;
        m_dataList.m_listID = m_noDataList.m_listID;
        m_noDataList.m_listID = tmpID;

        m_noDataTimer.reset();
        m_noDataTimer.start();
    }

    Release();

    return processedIncoming;
}

TcpConnection *TcpManager::EstablishConnection(const char *serverAddress, unsigned short serverPort, unsigned timeout)
{
    if (m_boundAsServer)
    {
        //can't open outgoing connections when in server mode
        // use a different TcpManager to do that
        return nullptr;
    }

    if (m_connectionListCount >= m_params.maxConnections)
        return(nullptr);

        // get server address
    unsigned long address = inet_addr(serverAddress);
    if (address == INADDR_NONE)
    {
		if (m_dnsMap[serverAddress].timeout >= time(nullptr))
 		{
 			address = m_dnsMap[serverAddress].addr;
 		}
 		else
 		{
 			struct hostent * lphp;
 			lphp = gethostbyname(serverAddress);
 			if (lphp == nullptr)
 				return(nullptr);
 			address = ((struct in_addr *)(lphp->h_addr))->s_addr;
 			
 			m_dnsMap[serverAddress].addr = address;
 			m_dnsMap[serverAddress].timeout = time(nullptr)+DNS_TIMEOUT;
		}
    }
    IPAddress destIP(address);

    TcpConnection *con = new TcpConnection(this, &m_allocator, m_params, destIP, serverPort, timeout);
	con->AddRef();//for the client - to conform to UdpLibrary method
    addNewConnection(con);

    return con;
}

void TcpManager::addNewConnection(TcpConnection *con)
{
    con->AddRef();
#ifdef WIN32 //uses select
    if (con->m_socket != INVALID_SOCKET)
        FD_SET(con->m_socket, &m_permfds);
#endif
    con->m_nextConnection = m_connectionList;
    con->m_prevConnection = nullptr;
    if (m_connectionList != nullptr)
        m_connectionList->m_prevConnection = con;
    m_connectionList = con;
    m_connectionListCount++;

    con->m_nextKeepAliveConnection = m_aliveList.m_beginList;
    con->m_prevKeepAliveConnection = nullptr;
    if (m_aliveList.m_beginList != nullptr)
        m_aliveList.m_beginList->m_prevKeepAliveConnection = con;
    m_aliveList.m_beginList = con;
    con->m_aliveListId = m_keepAliveList.m_listID;//start it out thinking it's already in the alive list, since it is

    con->m_nextRecvDataConnection = m_dataList.m_beginList;
    con->m_prevRecvDataConnection = nullptr;
    if (m_dataList.m_beginList != nullptr)
        m_dataList.m_beginList->m_prevRecvDataConnection = con;
    m_dataList.m_beginList = con;
    con->m_recvDataListId = m_noDataList.m_listID;//start it out thinking it's already in the data list, since it is
}

void TcpManager::removeConnection(TcpConnection *con)
{
    if (!con->wasRemovedFromMgr())
    {
        con->setRemovedFromMgr();
        m_connectionListCount--;
    #ifdef WIN32  //select only used on win32
        if (con->m_socket != INVALID_SOCKET)
        {
            FD_CLR(con->m_socket, &m_permfds);
        }
    #endif
        if (con->m_prevConnection != nullptr)
            con->m_prevConnection->m_nextConnection = con->m_nextConnection;
        if (con->m_nextConnection != nullptr)
            con->m_nextConnection->m_prevConnection = con->m_prevConnection;
        if (m_connectionList == con)
            m_connectionList = con->m_nextConnection;
        con->m_nextConnection = nullptr;
        con->m_prevConnection = nullptr;

        if (con->m_prevKeepAliveConnection != nullptr)
            con->m_prevKeepAliveConnection->m_nextKeepAliveConnection = con->m_nextKeepAliveConnection;
        if (con->m_nextKeepAliveConnection != nullptr)
            con->m_nextKeepAliveConnection->m_prevKeepAliveConnection = con->m_prevKeepAliveConnection;

        if (m_aliveList.m_beginList == con)
            m_aliveList.m_beginList = con->m_nextKeepAliveConnection;
        else if (m_keepAliveList.m_beginList == con)
            m_keepAliveList.m_beginList = con->m_nextKeepAliveConnection;
        con->m_nextKeepAliveConnection = nullptr;
        con->m_prevKeepAliveConnection = nullptr;



        if (con->m_prevRecvDataConnection != nullptr)
            con->m_prevRecvDataConnection->m_nextRecvDataConnection = con->m_nextRecvDataConnection;
        if (con->m_nextRecvDataConnection != nullptr)
            con->m_nextRecvDataConnection->m_prevRecvDataConnection = con->m_prevRecvDataConnection;

        if (m_dataList.m_beginList == con)
            m_dataList.m_beginList = con->m_nextRecvDataConnection;
        else if (m_noDataList.m_beginList == con)
            m_noDataList.m_beginList = con->m_nextRecvDataConnection;
        con->m_nextRecvDataConnection = nullptr;
        con->m_prevRecvDataConnection = nullptr;



        con->Release();
    }
}

void TcpManager::AddRef()
{
    m_refCount++;
}

void TcpManager::Release()
{
    if (--m_refCount == 0)
        delete this;
}

IPAddress TcpManager::GetLocalIp() const
{
	struct sockaddr_in addr_self;
	memset(&addr_self, 0, sizeof(addr_self));
	socklen_t len = sizeof(addr_self);
	getsockname(m_socket, (struct sockaddr *)&addr_self, &len);
	return(IPAddress(addr_self.sin_addr.s_addr));

}

unsigned int TcpManager::GetLocalPort() const
{
	struct sockaddr_in addr_self;
	memset(&addr_self, 0, sizeof(addr_self));
	socklen_t len = sizeof(addr_self);
	getsockname(m_socket, (struct sockaddr *)&addr_self, &len);
	return(ntohs(addr_self.sin_port));
}


#ifdef EXTERNAL_DISTRO
};
#endif


