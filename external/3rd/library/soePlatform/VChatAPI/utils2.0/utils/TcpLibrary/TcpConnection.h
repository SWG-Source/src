#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H


#include "TcpHandlers.h"
#include "TcpManager.h"
#include "IPAddress.h"
#include "TcpBlockAllocator.h"
#include "Clock.h"

#if defined(WIN32)
    #include <winsock2.h>
    typedef int socklen_t;
#else        // for non-windows platforms (linux)
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif


/**
 * @brief Manages a single connection.
 */
class TcpConnection
{
public:

    /**
     * @brief The connection status.
     */
    enum Status { 
        StatusNegotiating, /**< Currently attempting to connect.  */
        StatusConnected,   /**< Currently connected. */
        StatusDisconnected /**< Currently disconnected. */
    };

    /**
     * @brief Sets the handler object which will receive callback methods.
     *
     * To have the TcpConnection call your object directly when packets are received, and when the 
     * connection is disconnected, you simply need to derive your class
     * (multiply if necessary) from TcpConnectionHandler, then you can use
     * this method to set the object the TcpConnection will call as appropriate.
     * default = nullptr (no callbacks made)
     *
     * @param handler  The object which will be called for notifications.
     */
    void SetHandler(TcpConnectionHandler *handler){ m_handler = handler; }

    /**
     * @brief Returns the handler associated with this object.
     */
    TcpConnectionHandler *GetHandler(){ return m_handler; }

    /**
     * @brief Returns the current status of this connection.
     */
    Status GetStatus(){ return m_status; }

    /**
     * @brief Queues a message to be sent on this connection.
     */
    void Send(const char *data, unsigned dataLen);

    /**
     * @brief Disconnects and recycles the socket.
     * 
     * @param notifyApplication primarily used internally, but when set to 'true', it will cause the application
     *          to be called back via the onTerminated handler due to this call (the callback will not occur if the connection was
     * already disconnected)
     */
    void Disconnect(bool notifyApplication=true);

    /**
     * @brief Returns the ip on the other side of this connection.
     */
    IPAddress GetDestinationIp(){ return m_destIP; }

    /**
     * @brief Returns the port on the other side of this conection.
     */
    unsigned short GetDestinationPort(){ return m_destPort; }

    /**
     * @brief Standard AddRef/Release scheme
     */
    void AddRef();

    /**
     * @brief Standard AddRef/Release scheme
     */
    void Release();

    bool wasRemovedFromMgr() { return m_wasConRemovedFromMgr; }
    void setRemovedFromMgr() { m_wasConRemovedFromMgr = true; }

    bool isConnectionRefused() const { return m_connectionRefused; }

protected:
    friend class TcpManager;
    TcpConnection(TcpManager *tcpManager, TcpBlockAllocator *sendAlloc, TcpManager::TcpParams &params, const IPAddress &destIP, unsigned short destPort, unsigned timeout);
    int finishConnect();/**< returns < 0 if fatal error and connect will not work, =0 if need more time, >0 if connect completed */
    TcpConnection(TcpManager *tcpManager, TcpBlockAllocator *sendAlloc, TcpManager::TcpParams &params, SOCKET socket, const IPAddress &destIP, unsigned short destPort);
    TcpConnection *m_nextConnection; /**< Double linked list imp. */
    TcpConnection *m_prevConnection; /**< Double linked list imp. */
    SOCKET m_socket;
    int processOutgoing();/**< returns < 0 if fatal error and socket has been closed, =0 if sent data, call again immediately if want to, >0 may have sent data, but calling again would do no good because there is either no more data to send, or would block. */
    int processIncoming();/**< returns < 0 if fatal error and socket has been closed, =0 if read anything (full or partial message), >0 if nothing to read now, or would block so shouldn't try again immediately. */
    

    TcpConnection *m_nextKeepAliveConnection; /**< Double linked list imp. */
    TcpConnection *m_prevKeepAliveConnection; /**< Double linked list imp. */
    int m_aliveListId;

    TcpConnection *m_nextRecvDataConnection;
    TcpConnection *m_prevRecvDataConnection;
    int m_recvDataListId;

private:
    ~TcpConnection();
    void setOptions();
    TcpManager *m_manager;
    bool translateRecvSocketEror();
    Status m_status;
    TcpConnectionHandler *m_handler;
    IPAddress m_destIP;
    unsigned short m_destPort;
    unsigned m_refCount;
    TcpBlockAllocator *m_sendAllocator;
    data_block *m_head;
    data_block *m_tail;
    unsigned m_bytesRead;
    unsigned m_bytesNeeded;
    TcpManager::TcpParams m_params;
    char *m_recvBuff;
    sockaddr_in m_addr;
    unsigned m_connectTimeout;
    Clock m_connectTimer;
    bool m_connectionRefused;

    bool m_wasConRemovedFromMgr;
};

#ifdef EXTERNAL_DISTRO
};
#endif

#endif //TCPCONNECTION_H



