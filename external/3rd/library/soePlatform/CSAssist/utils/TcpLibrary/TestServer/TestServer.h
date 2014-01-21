#ifndef TESTSERVER_H
#define TESTSERVER_H

#include <TcpLibrary/TcpHandlers.h>
#include <TcpLibrary/TcpConnection.h>
#include <TcpLibrary/TcpManager.h>
#include <list>
#include <string>


class Connection;

class TestServer : public TcpManagerHandler
{
public:
    TestServer(unsigned short port);
    virtual ~TestServer();

    void process();
    
    void OnConnectRequest(TcpConnection *con);

    void removeConnection(Connection *con);

private:
    unsigned short m_port;
    TcpManager *m_manager;
    std::list<Connection *> m_connections;

};

class Connection : public TcpConnectionHandler
{
public:
    Connection(TestServer *manager, TcpConnection *con);
    virtual ~Connection();

    void OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen);
    void OnTerminated(TcpConnection *con);

    void send(const std::string &msg);

private:
    TestServer *m_manager;
    TcpConnection *m_con;
};

#endif //TESTSERVER_H
