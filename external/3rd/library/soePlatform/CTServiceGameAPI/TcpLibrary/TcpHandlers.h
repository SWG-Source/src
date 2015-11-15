#ifndef TCPHANDLERS_H
#define TCPHANDLERS_H

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

class TcpConnection;

/**
 * @brief Interface used by TcpManager class for notification to application of connection state/etc.
 *
 * Note: these callbacks will only be made when during a call to TcpManager::giveTime.
 */
class TcpManagerHandler
{
  public:
    /**
     * @brief Callback made when a new connection has been established by the manager.
     */
    virtual void OnConnectRequest(TcpConnection *con)=0;

};

/**
 * @brief Interface used by TcpConnection class for notification to application of connection state/etc.
 *
 * Note: these callbacks will only be made when during a call to TcpManager::giveTime.
 */
class TcpConnectionHandler
{
  public:
    /**
     * @brief Callback made when a new message has been received on the specified connection.
     */
    virtual void OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen)=0;

    /**
     * @brief Callback made when the specified connection has closed, or been closed.
     */
    virtual void OnTerminated(TcpConnection *con)=0;
};


#ifdef EXTERNAL_DISTRO
};
#endif
#endif //TCPHANDLERS_H

