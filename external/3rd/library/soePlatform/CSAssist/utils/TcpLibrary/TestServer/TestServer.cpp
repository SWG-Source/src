#include "TestServer.h"


TestServer::TestServer(unsigned short port)
: m_port(port)
{
    TcpManager::TcpParams params;
    params.port = port;
    params.maxConnections = 1000000;
    params.incomingBufferSize = 512*1024;
    params.outgoingBufferSize = 512*1024;
    params.allocatorBlockSize = 8*1024;
    params.allocatorBlockCount = 1024;
    params.maxRecvMessageSize = 8*1024;

    m_manager = new TcpManager(params);

    m_manager->SetHandler(this);
    m_manager->BindAsServer();
}


TestServer::~TestServer()
{
    m_manager->Release();
    //clean up m_connections
}

void TestServer::process()
{
    m_manager->GiveTime();
}

void TestServer::OnConnectRequest(TcpConnection *con)
{
    printf("recvd connect: client (%d)\n", con->GetDestinationPort());
    Connection *mycon = new Connection(this, con);
	m_connections.insert(m_connections.begin(), mycon);
}

void TestServer::removeConnection(Connection *con)
{
    m_connections.remove(con);
}




Connection::Connection(TestServer *manager, TcpConnection *con)
: m_manager(manager),
  m_con(con)
{
    m_con->SetHandler(this);
}

Connection::~Connection()
{
    m_manager->removeConnection(this);
    m_con->Release();
}

void Connection::OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen)
{
    std::string msg((const char *)data, dataLen);
    printf("recvd message: client (%d) message(%s)\n", con->GetDestinationPort(), msg.c_str());
    //send a reply back immediately
    send(std::string("right back attya..."));
}


void Connection::OnTerminated(TcpConnection *con)
{
    printf("recvd disconnect: client (%d)\n", con->GetDestinationPort());
    m_con->Release();
    delete this;
}

void Connection::send(const std::string &msg)
{
    m_con->Send(msg.c_str(), msg.length());
}


