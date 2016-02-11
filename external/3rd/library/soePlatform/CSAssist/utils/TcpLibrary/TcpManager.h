#ifndef TCPMANAGER_H
#define TCPMANAGER_H

#include "TcpHandlers.h"

#include "TcpBlockAllocator.h"
#include "IPAddress.h"
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
	const unsigned long int INVALID_SOCKET = 0xFFFFFFFF;
	const unsigned long int SOCKET_ERROR   = 0xFFFFFFFF;
    typedef int SOCKET;
#endif

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

class TcpConnection;

struct ConnectionList
{
    ConnectionList(TcpConnection *con, int id) : m_beginList(con), m_listID(id) {}

    TcpConnection *m_beginList;
    int m_listID;
};

/** 
 * @brief The purpose of the TcpManager is to manage a set of connections that are coming in on a particular port.
 *
 */
class TcpManager
{
public:

    /** @brief Parameters for the TcpManager. */
    struct TcpParams
    {
        /** @brief Simple constructor sets default values for members. */
        TcpParams();

        /** @brief Simple copy constructor. */
        TcpParams(const TcpParams &cpy);

        /**    
         * @brief Connection port number.
         *
         * this is the port number that this manager will use for all incoming and outgoing data. On the client side
         * this is typically set to 0, which causes the manager object to randomly pick an available port.  On the server
         * side, this port should be set to a specific value as it will represent the port number that clients will use
         * to connect to the server (ie. the listening port).  It's generally a good idea to give the user on the client
         * side the option of fixing this port number at a specific value as well as it is often necessary for them to
         * do so in order to navigate company firewalls which may have specific port numbers open to them for this purpose.
         * default = 0
         */
        unsigned short port;

        /**
         * @ brief Server bind ip.
         * 
         */
        char bindAddress[64];


        /**    
         * @brief Maximum number of connections that can be established by this manager.
         *
         * this is the maximum number of connections that can be established by this manager, any incoming/outgoing connections
         * over this limit will be refused.  On the client side, this typically only needs to be set to 1, though there
         * is little harm in setting this number larger.
         * default = 10
         */
        unsigned maxConnections;

        /**    
         * @brief The size of the incoming socket buffer.  
         *
         * The client will want to set this fairly small (32k or so), but the server
         * will want to set this fairly large (512k)
         * default = 64k
         */
        unsigned incomingBufferSize;

        /**    
         * @brief The size of the outgoing socket buffer.
         *
         * The client will want to set this fairly small (32k or so), but the server
         * will want to set this fairly large (512k)
         * default = 64k
         */
        unsigned outgoingBufferSize;

        /**
         * @brief The block size of a single outgoing buffer memory allocator block.
         *
         * This param should allways be set at least as high as the maximum message size you 
         * expect to send (performance will suffer otherwise).
         * default = 8K
         */
        unsigned allocatorBlockSize;

        /**
         * @brief The number of block memory allocator 'blocks' created at a time.
         * 
         * This is the number of blocks created for the buffer allocator for each 
         * TcpConnection opened by this manager.  Since the block size should be
         * the max size of an outgoing message, the recommended setting is: greater 
         * than the number of concurrent connections you expect to normally have open.
         * default = 1024
         */
        unsigned allocatorBlockCount;

        /**
         * @brief The maximum size that a recvd message is allowed to be.
         *
         * Really only here for protection, not required.  If you set this, you can safeguard
         * your client/server from receiving stray oversized messages.  If a message on the socket
         * specifies it's length at larger than this value, then the message is not read, and the connection
         * is terminated.  If the value is set to 0, then there is no max message size checking
         * on incoming messages (this will also cary a performance hit, since every new message
         * recieved will have to have a new buffer created if you don't specify a value here). Be careful
         * not to set this too small, if you have messages that could exceed the value you set here
         * they will be discarded, and the connection will be terminated without warning.
         * default = 0
         */
        unsigned maxRecvMessageSize;

        unsigned keepAliveDelay;

        unsigned noDataTimeout;
    };


    /** 
     * @brief 
     */
    TcpManager(const TcpParams &params);

    /**    
     * @brief Use to specify a handler object to receive callbacks.
     *
     * To have the TcpManager call your object directly when connection requests come in, you
     * simply need to derive your class (multiply if necessary) from TcpManagerHandler, then you can use
     * this method to set the object the TcpManager will call as appropriate.  The TcpConnection object
     * also has a handler mechanism that replaces the other callback functions below, see TcpConnection::SetHandler
     * default = nullptr (no callbacks made)
     *
     * @param handler  The object which will be called for manager related notifications.
     */
    void SetHandler(TcpManagerHandler *handler);

    /** 
     * @brief This function MUST be called on a regular basis in order to give the manager object time to service the socket and give time to various connection objects that may need processing time, etc.
     *
     * @param maxTimeAcceptingConnections  The max amount of time in milliseconds to spend accepting new client connections.
     *          This parameter is only used if this manager has been bound as a server (bindAsServer).
     * If you set this param to 0, it will not attempt to accept any new connections.
     * 
     * @param giveConnectionsTime  
     *          True if every connection opened on this manager is given time in this call, false if 
     * no connections are given time.
     * 
     * @param maxSendTimePerConnection  Max amount of time in milliseconds to spend on each client processing outgoing messages.
     *          A max of the specified amount of time will be spent on each and every individual connection.  If you set
     * this parametrer to 0, it will not process any outgoing messages on any clients. Note also that when attempting
     * to establish new connections (via the EstablishConnection method), this parameter must be > 0 in order to
     * complete the connection process for any connections that were still negotiating.
     *
     * @param maxRecvTimePerConnection  Max amount of time in milliseconds to spend on each client processing incoming messages.
     *          A max of the specified amount of time will be spent on each and every individual connection.  If you set
     * this param to 0, it will not process any incoming messages on any clients.
     * This is a good way to give the manager processing time for outgoing packets in situations
     * where the application does not want to have to worry about processing incoming packets.
     *
     * @return true if any incoming packets were processed during this time slice, otherwise returns false
     */
    bool GiveTime(unsigned maxTimeAcceptingConnections = 5, unsigned maxSendTimePerConnection = 5, unsigned maxRecvTimePerConnection = 5);

    /**    
     * @brief Used to establish a connection to a server that is listening at the specified address and port.
     * 
     * The serverAddress will do a DNS lookup as appropriate.  This call will block long enough to resolve
     * the DNS lookup, but then will return a TcpConnection object that will be in a StatusNegotiating
     * state until the connection is actually established.  The application must give the manager
     * object time after calling EstablishConnection or else the negotiation process to establish the
     * connection will never have time to actually occur.  Typically the client establishing the connection
     * will call EstablishConnection, then sit in a loop calling TcpManager::GiveTime and checking to see
     * if the status of the returned TcpConnection object is changed from StatusNegotiating.  This allows
     * the application to look for the ESC key or timeout an attempted connection.
     *
     * @param serverAddress  The address of the server to open a connection to.
     * 
     * @param serverPort  The port of the server to open a connection to.
     *
     * @param timeout  How long to attempt connecting to the server (in milliseconds).
     *          Setting the timeout value to something greater than 0 will cause the TcpConnection object to change
     * from a StatusNegotiating state to a StatusDisconnected state after the timeout has expired.  It will also cause
     * the connect-complete callback to be called if the connection is succesfull.
     *
     * @return A pointer to a TcpConnection object.
     *          nullptr if the manager object has exceeded its maximum number of connections
     * or if the serverAddress cannot be resolved to an IP address.
     */
    TcpConnection *EstablishConnection(const char *serverAddress, unsigned short serverPort, unsigned timeout = 0);

    /**
     * @brief  Binds this manager as a server which will listen for and accept incoming connections.
     *
     * @return 'true' if manager is able to bind succesfully, false otherwise.
     */
    bool BindAsServer();

    /**
     * @brief Standard AddRef/Release scheme
     */
    void AddRef();

    /**
     * @brief Standard AddRef/Release scheme
     */
    void Release();

    /**
     * @brief Returns the ip address of this machine.  If the machine is multi-homed, this value may be blank.
     */
	IPAddress GetLocalIp() const;


	/**
     * @brief Returns the port the manager is actually using.  This value will be the same as is specified in 
	 *        Params::port (or if Params::port was set to 0, this will be the dynamically assigned port number)
 	 */
	unsigned int GetLocalPort() const;

protected:
    friend class TcpConnection;
    void removeConnection(TcpConnection *con);
    TcpManagerHandler *m_handler;

    ConnectionList m_keepAliveList;
    ConnectionList m_aliveList;

    ConnectionList m_noDataList;
    ConnectionList m_dataList;

private:
    ~TcpManager();
    TcpParams m_params;
    int m_refCount;
    TcpConnection *m_connectionList;
    unsigned m_connectionListCount;
#ifdef WIN32
    fd_set m_permfds; /**< Used for select on WIN32 if we are in server mode. Keeps track of all clients connected to us. */
#endif //WIN32
    SOCKET m_socket;
    bool m_boundAsServer;
    TcpBlockAllocator m_allocator;
    Clock m_keepAliveTimer;
    Clock m_noDataTimer;

    void addNewConnection(TcpConnection *con);
    SOCKET getMaxFD();
    TcpConnection *getConnection(SOCKET fd);
    TcpConnection *acceptClient();
};

#ifdef EXTERNAL_DISTRO
};
#endif

#endif //TCPMANAGER_H



