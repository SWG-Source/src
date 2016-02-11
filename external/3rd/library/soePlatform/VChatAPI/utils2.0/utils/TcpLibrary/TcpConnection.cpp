#include "TcpConnection.h"
#include "TcpManager.h"
#include "Clock.h"
#include <errno.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif


//used when want to open new connection with this socket
TcpConnection::TcpConnection(TcpManager *tcpManager, TcpBlockAllocator *sendAlloc, TcpManager::TcpParams &params, const IPAddress &destIP, unsigned short destPort, unsigned timeout)
: m_nextConnection(nullptr),
  m_prevConnection(nullptr),
  m_socket(INVALID_SOCKET),
  m_nextKeepAliveConnection(nullptr),
  m_prevKeepAliveConnection(nullptr),
  m_aliveListId(tcpManager->m_aliveList.m_listID),
  m_nextRecvDataConnection(nullptr),
  m_prevRecvDataConnection(nullptr),
  m_recvDataListId(tcpManager->m_dataList.m_listID),
  m_manager(tcpManager),
  m_status(StatusNegotiating),
  m_handler(nullptr),
  m_destIP(destIP),
  m_destPort(destPort),
  m_refCount(0),
  m_sendAllocator(sendAlloc),
  m_head(nullptr),
  m_tail(nullptr),
  m_bytesRead(0),
  m_bytesNeeded(0),
  m_params(params),
  m_recvBuff(nullptr),
  m_connectTimeout(timeout),
  m_connectTimer(),
  m_wasConRemovedFromMgr(false),
  m_connectionRefused(false)
{
    //start connection timer
    m_connectTimer.start();

    memset(&m_addr, 0, sizeof(m_addr));
    if (m_params.maxRecvMessageSize != 0)
    {
        m_recvBuff = new char[m_params.maxRecvMessageSize];
    }

    m_socket = socket(AF_INET, SOCK_STREAM, 0);


    setOptions();


    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(m_destPort);
    m_addr.sin_addr.s_addr = m_destIP.GetAddress();

    int err = connect(m_socket, (sockaddr *)&m_addr, sizeof(m_addr));

    if(err == SOCKET_ERROR)
    {
#ifdef WIN32
        int sockerr = WSAGetLastError();
        if(sockerr != WSAEWOULDBLOCK)
        {
            //a real error
            m_status = StatusDisconnected;
        }
        else
        {
            m_status = StatusNegotiating;
        }

#else  // UNIX
        if (errno != EINPROGRESS)
        {
            m_status = StatusDisconnected;
        }
        else
        {
            m_status = StatusNegotiating;
        }
#endif
    }
    else
    {
        //we are connected, wow
        m_status = StatusConnected;
    }

}

//used when server mode creates new connection object representing a connect request
TcpConnection::TcpConnection(TcpManager *tcpManager, TcpBlockAllocator *sendAlloc, TcpManager::TcpParams &params, SOCKET socket, const IPAddress &destIP, unsigned short destPort)
: m_nextConnection(nullptr),
  m_prevConnection(nullptr),
  m_socket(socket),
  m_nextKeepAliveConnection(nullptr),
  m_prevKeepAliveConnection(nullptr),
  m_aliveListId(tcpManager->m_aliveList.m_listID),
  m_nextRecvDataConnection(nullptr),
  m_prevRecvDataConnection(nullptr),
  m_recvDataListId(tcpManager->m_dataList.m_listID),
  m_manager(tcpManager),
  m_status(StatusConnected),
  m_handler(nullptr),
  m_destIP(destIP),
  m_destPort(destPort),
  m_refCount(0),
  m_sendAllocator(sendAlloc),
  m_head(nullptr),
  m_tail(nullptr),
  m_bytesRead(0),
  m_bytesNeeded(0),
  m_params(params),
  m_recvBuff(nullptr),
  m_connectTimeout(0),
  m_connectTimer(),
  m_wasConRemovedFromMgr(false)
{
    memset(&m_addr, 0, sizeof(m_addr));
    if (m_params.maxRecvMessageSize != 0)
    {
        m_recvBuff = new char[m_params.maxRecvMessageSize];
    }


    setOptions();
}

void TcpConnection::setOptions()
{
    if (m_socket != INVALID_SOCKET)
    {
#if defined(WIN32)
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
            //bummer, but no need to crash now.... ?
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
            //bummer, but no need to crash now.... ?
        }
#endif
    }

}

int TcpConnection::finishConnect()
{
    AddRef();
    int returnVal = 0;
    m_connectionRefused = false;
    /**< returns < 0 if fatal error and connect will not work, =0 if need more time, >0 if connect completed */
    switch (m_status)
    { 
    case StatusDisconnected:
        {
            //something went wrong
            Disconnect(false);
            returnVal = -1;
        }
        break;
    case StatusNegotiating:
        {
#ifdef WIN32
            //try to finish connection
            fd_set wrSet;
            FD_ZERO(&wrSet);

            FD_SET(m_socket, &wrSet);

            timeval t;
            t.tv_sec = 0;
            t.tv_usec = 0;

            int err = select(m_socket + 1, nullptr, &wrSet, nullptr, &t);

            if (err == 0)
            {
                //needs more time
                returnVal = 0;
            }
            else if (err == SOCKET_ERROR)
            {
                //huhoh, let's hope it needs more time
                int sockerr = WSAGetLastError();
                if (sockerr == WSAEINPROGRESS 
                    || sockerr == WSAEWOULDBLOCK 
                    || sockerr == WSAEALREADY 
                    || sockerr == WSAEINVAL)
                {
                    //yep
                    returnVal = 0;
                }
                else
                {
                    Disconnect(false);
                    returnVal = -1;
                }
            }
            else
            {
                //check if write bit set for socket
                if (FD_ISSET(m_socket, &wrSet))
                {
                    //connection complete
                    m_status = StatusConnected;
                    returnVal = 1;
                }
                else
                {
                    //give it more time??
                    returnVal = 0;
                }
            }

#else  // not WIN32
            int err = connect(m_socket, (sockaddr *)&m_addr, sizeof(m_addr));

            if(err == SOCKET_ERROR)
            {
                m_connectionRefused = (errno == ECONNREFUSED);
                if (errno != EINPROGRESS && errno != EALREADY)
                {
                    /* if (errno == ECONNREFUSED) */ Disconnect(true);
                    /* else Disconnect(false);  -- Don't do this here; bad! */
                    returnVal = -1;//failure
                }
                else
                {
                    returnVal = 0;//need to wait
                }
            }
            else
            {
                m_status = StatusConnected;
                returnVal = 1;//connect success
            }
#endif
        }
        break;
    case StatusConnected:
        {
            //wierd, shouldn't be trying to do this here
            Disconnect(true);
            returnVal = -1;
        }
        break;
    }

    if (returnVal == 0 && m_connectTimeout != 0 && m_connectTimer.isDone(m_connectTimeout))
    {
        Disconnect(true);
        returnVal = -1;
    }
    else if (returnVal ==1/* && m_connectTimeout != 0*/)
    {
        //need to give, onConnect callback
        if (m_handler)
            m_handler->OnConnectRequest(this);
    }

    Release();
    return returnVal;

}


TcpConnection::~TcpConnection()
{
    delete [] m_recvBuff;

    while(m_head != nullptr)
    {
        data_block *tmp = m_head;
        m_head = m_head->m_next;
        m_sendAllocator->returnBlock(tmp);
    }

    //TODO:  need to notify app if are currently connected
}

void TcpConnection::Send(const char *data, unsigned int dataLen)
{
//add msg to buf
    int totalLen = dataLen + sizeof(int);

    if(m_status == StatusDisconnected)
    {
        return;
    }

    if (m_params.keepAliveDelay > 0 && m_aliveListId == m_manager->m_aliveList.m_listID)
    {
        m_aliveListId = m_manager->m_keepAliveList.m_listID;

        if (m_prevKeepAliveConnection != nullptr)
            m_prevKeepAliveConnection->m_nextKeepAliveConnection = m_nextKeepAliveConnection;
        if (m_nextKeepAliveConnection != nullptr)
            m_nextKeepAliveConnection->m_prevKeepAliveConnection = m_prevKeepAliveConnection;
        if (m_manager->m_keepAliveList.m_beginList == this)
            m_manager->m_keepAliveList.m_beginList = m_nextKeepAliveConnection;

        m_nextKeepAliveConnection = m_manager->m_aliveList.m_beginList;
        m_prevKeepAliveConnection = nullptr;
        if (m_manager->m_aliveList.m_beginList != nullptr)
            m_manager->m_aliveList.m_beginList->m_prevKeepAliveConnection = this;
        m_manager->m_aliveList.m_beginList = this;
    }
        

    data_block *work = nullptr;    

    // this connection has no send buffer. Get a block
    if(!m_tail)
    {
        m_head = m_sendAllocator->getBlock();
        m_tail = m_head;
    }
    work = m_tail;

    //send message len first
    unsigned nLen = htonl(totalLen);
    unsigned lenLength = sizeof(int);
    unsigned lenIndex = 0;
    while(lenIndex < lenLength)
    {
        if ((lenLength - lenIndex) <= (work->m_totalSize - work->m_usedSize))
        {
            //size will fit in this block
            memcpy(work->m_data + work->m_usedSize, (char *)(&nLen) + lenIndex, lenLength - lenIndex);
            work->m_usedSize += (lenLength - lenIndex);
            lenIndex += (lenLength - lenIndex);
        }
        else
        {
            //size will not fit in this block
            memcpy(work->m_data + work->m_usedSize, (char *)(&nLen) + lenIndex, work->m_totalSize - work->m_usedSize);
            lenIndex += work->m_totalSize - work->m_usedSize;
            work->m_usedSize += work->m_totalSize - work->m_usedSize;    
            work->m_next = m_sendAllocator->getBlock();
            work = work->m_next;
            m_tail = work;
        }
    }

    //now send message payload
    unsigned messageIndex = 0;
    while(messageIndex < dataLen)
    {
        if((dataLen - messageIndex) <= (work->m_totalSize - work->m_usedSize))
        {
            // data will fit in this block
            memcpy(work->m_data + work->m_usedSize, data + messageIndex, (dataLen - messageIndex));
            work->m_usedSize += (dataLen - messageIndex);
            messageIndex += (dataLen - messageIndex);
        }
        else
        {
            // data will not fit in this block. Fill this block and get another block
            memcpy(work->m_data + work->m_usedSize, data + messageIndex, work->m_totalSize - work->m_usedSize);
            messageIndex += work->m_totalSize - work->m_usedSize;
            work->m_usedSize += work->m_totalSize - work->m_usedSize;    
            work->m_next = m_sendAllocator->getBlock();
            work = work->m_next;
            m_tail = work;
        }
    }


    return;
}


void TcpConnection::Disconnect(bool notifyApplication)
{
    AddRef();
    m_status = StatusDisconnected;
    if (!m_wasConRemovedFromMgr)
    {
        m_manager->removeConnection(this);
        m_wasConRemovedFromMgr = true;
    }


    if(m_socket != INVALID_SOCKET)
    {
#if defined(WIN32)
        closesocket(m_socket);
#else
        close(m_socket);
#endif
        m_socket = INVALID_SOCKET;
    }


    if (notifyApplication && m_handler)
        m_handler->OnTerminated(this);

    Release();
}


void TcpConnection::AddRef()
{
    m_refCount++;
}


void TcpConnection::Release()
{
    if (--m_refCount == 0)
    {
        //make sure manager knows I'm gone
        if (m_status != StatusDisconnected)
		{
			m_refCount = 1;
            Disconnect(false);
			m_refCount = 0;
		}
        delete this;
    }
}

int TcpConnection::processIncoming()
{
/**< returns < 0 if fatal error and socket has been closed, 
    =0 if read anything (full or partial message), 
    >0 if nothing to read now, or would block so shouldn't try again immediately. */

    if (m_status != StatusConnected)
    {
        //wait until connect succeeds
        return 1;
    }

    if (m_params.noDataTimeout > 0 && m_recvDataListId == m_manager->m_dataList.m_listID)
    {
        m_recvDataListId = m_manager->m_noDataList.m_listID;

        if (m_prevRecvDataConnection != nullptr)
            m_prevRecvDataConnection->m_nextRecvDataConnection = m_nextRecvDataConnection;
        if (m_nextRecvDataConnection != nullptr)
            m_nextRecvDataConnection->m_prevRecvDataConnection = m_prevRecvDataConnection;
        if (m_manager->m_noDataList.m_beginList == this)
            m_manager->m_noDataList.m_beginList = m_nextRecvDataConnection;

        m_nextRecvDataConnection = m_manager->m_dataList.m_beginList;
        m_prevRecvDataConnection = nullptr;
        if (m_manager->m_dataList.m_beginList != nullptr)
            m_manager->m_dataList.m_beginList->m_prevRecvDataConnection = this;
        m_manager->m_dataList.m_beginList = this;
    }


        int newMsg = 0;


        if (m_bytesRead < sizeof(int))
        {
            //new msg    
            newMsg = 1;
            //printf("socket: %d\n", m_socket);
            int ret = recv(m_socket, ((char *)(&m_bytesNeeded) + m_bytesRead), 
                                                            4 - m_bytesRead, 0);
            //fprintf(stderr, "READ: %d\n", ret);    
            if (ret == 0)
            {
                //We did a select, so there should be data.  Socket was closed.
                Disconnect();
                return -1;
            }
            else if (ret == -1)
            {
                if (translateRecvSocketEror())
                {
                    //fatal error
                    return -1;
                }
                else
                {
                    //need to wait
                    return 1;
                }
            }
            else
            {
                m_bytesRead += ret; 
                if (m_bytesRead < 4)
                {
                    return 1;//need to wait
                }
                else
                {

                    m_bytesNeeded = ntohl(m_bytesNeeded);

                    //printf("m_bytesNeeded = %i\n", m_bytesNeeded);
                    if (m_bytesNeeded == sizeof(int))
                    {
                        //keepalive, ignore
                        m_bytesRead = 0;
                        m_bytesNeeded = 0;
                        return 0;
                    }
                    else if (m_bytesNeeded < sizeof(int))
                    {
                        //major protocol violation
                        Disconnect();
                        return -1;
                    }
                    else if (m_params.maxRecvMessageSize == 0)
                    {
                        delete [] m_recvBuff;
                        m_recvBuff = new char[m_bytesNeeded-4];
                    }
                    else if (m_params.maxRecvMessageSize != 0 && (m_bytesNeeded-4) > m_params.maxRecvMessageSize)
                    {
                        //error, maxRecvMeessageSize exceeded, Disconnect
                        Disconnect();
                        return -1;
                    }
                }
            }
        }

        int msgBytesRead = m_bytesRead - 4;
        int msgBytesNeeded = m_bytesNeeded - 4;

        int ret = recv(m_socket, (char *)(m_recvBuff + msgBytesRead),
                                    msgBytesNeeded - msgBytesRead, 0);
        if (ret == 0 && !newMsg)
        {
            //We did a select, so there should be data.  Socket was closed.
            Disconnect();
            return -1;
        }
        if (ret == -1)
        {
            if (translateRecvSocketEror())
            {
                //fatal error
                return -1;
            }
            else
            {
                //need to wait
                return 1;
            }
        } else
        {
            m_bytesRead += ret; 
        }

        if (m_bytesRead == m_bytesNeeded)
        {
            m_bytesRead = 0;
            m_bytesNeeded = 0;
            if (m_handler)
            {
                AddRef();//could get deleted during this callback
                m_handler->OnRoutePacket(this, (unsigned char *)m_recvBuff, msgBytesNeeded);

                if (m_status == StatusDisconnected)
                {
                    Release();
                    return -1;
                }
                Release();
            }

            //entire message received
            return 0;
        }
        else
        {
            return 1;//couldn't get entire msg
        }
}

int TcpConnection::processOutgoing()
{
/**< returns < 0 if fatal error and socket has been closed, 
    =0 if sent data, call again immediately if want to, 
    >0 may have sent data, but calling again would do no good because there is either no more data to send, or would block. */
    if (m_status != StatusConnected)
    {
        //wait until connect succeeds
        return 0;
    }



    int sendError = 1;

    // If m_head is not nullptr, then this connection has something to send


    if(m_head)
    {


        int amt = ::send(m_socket, m_head->m_data + m_head->m_sentSize, m_head->m_usedSize - m_head->m_sentSize, 0);
        if(amt < 0)
        {
#ifdef WIN32
            switch(WSAGetLastError())
            {
            case WSAEWOULDBLOCK:
            case WSAEINTR:
            case WSAEINPROGRESS:
            case WSAEALREADY:
            case WSA_IO_PENDING:
            case WSA_NOT_ENOUGH_MEMORY:
            case WSATRY_AGAIN:
                //try again
                sendError = 1;
                break;
            default:
                //assume broken, Disconnect
                Disconnect();
                sendError = -1;
                break;
            }
#else //not WIN32

            // error condition, EAGAIN is recoverable, otherwise raise an error condition. Break from loop 
            switch(errno)
            {
            case EAGAIN:
                //try again
                sendError = 1;
                break;
            default:
                //assume broken, Disconnect
                Disconnect();
                sendError = -1;
                break;
            }
#endif
        }
        else if(static_cast<unsigned>(amt) < (m_head->m_usedSize - m_head->m_sentSize))
        {
            // partial send: trying to do anything more now would be a waste of time. Break from loop
            m_head->m_sentSize += amt;
            sendError = 1;
        }
        else if(amt == 0)
        {
            Disconnect();
            sendError = -1;
            // client closed connection
        }
        else
        {
            // everything was sent from this block. Return it to the pool, advance m_head. Attempt to continue
            // sending
            data_block *tmp = m_head;
            if(m_tail == m_head)
            {
                m_tail = m_tail->m_next;
                m_head = m_head->m_next;
            }
            else
            {
                m_head = m_head->m_next;
            }
            m_sendAllocator->returnBlock(tmp);

            sendError = 0;
        }
    }

    return sendError;
}


bool TcpConnection::translateRecvSocketEror()
{
/**< returns  false if fatal error and socket has been closed, 
    true if should try again. */
    bool fatalError=false;
#ifdef WIN32

    int lastErr = WSAGetLastError();
    switch(lastErr)
    {
        case WSAENOBUFS:
        case WSAEINPROGRESS:
        case WSAEINTR:
        case WSAEWOULDBLOCK:
        case WSABASEERR:
            fatalError=false;
            break;

        case WSANOTINITIALISED:
        case WSAENETDOWN:
        case WSAEFAULT:
        case WSAENOTCONN:
        case WSAENETRESET:
        case WSAENOTSOCK:
        case WSAEOPNOTSUPP:
        case WSAESHUTDOWN:
        case WSAEMSGSIZE:
        case WSAEINVAL:
        case WSAECONNABORTED:
        case WSAETIMEDOUT:
        case WSAECONNRESET:
        default:
            //fatal
            fatalError=true;
            Disconnect();
            break;
    }

#else //not WIN32
    
    switch(errno)
    {
        case EWOULDBLOCK:
        case EINTR:
        case ETIMEDOUT:
        case ENOBUFS:
            //try later
            fatalError=false;
            break;

        case EBADF:
        case ECONNRESET:
        case EFAULT:
        case EINVAL:
        case ENOTCONN:
        case ENOTSOCK:
        case EOPNOTSUPP:
        case EPIPE:
        case EIO:
        case ENOMEM:
        case ENOSR:
        default:
            //fatal
            fatalError=true;
            Disconnect();
            break;
    }
#endif


    return fatalError;
}

#ifdef EXTERNAL_DISTRO
};
#endif


