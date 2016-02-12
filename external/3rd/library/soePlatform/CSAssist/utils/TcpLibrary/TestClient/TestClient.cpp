#include "TestClient.h"

extern bool g_callbackRecieved;
extern unsigned g_numSent;
extern unsigned g_numRecvd;


TestClient::TestClient(const std::string &serverAddress, unsigned short serverPort)
: m_conState(CON_DISCONNECT),
  m_serverAddress(serverAddress),
  m_serverPort(serverPort)
{
    TcpManager::TcpParams params;
    params.port = 0;
    params.maxConnections = 1;
    params.incomingBufferSize = 32*1024;
    params.outgoingBufferSize = 32*1024;
    params.allocatorBlockSize = 8*1024;
    params.allocatorBlockCount = 1;
    params.maxRecvMessageSize = 8*1024;

    m_manager = new TcpManager(params);
}

TestClient::~TestClient()
{
    if (m_con)
        m_con->Release();

    if (m_manager)
        m_manager->Release();
}


void TestClient::process()
{
    switch(m_conState)
	{
	case CON_DISCONNECT:
		m_con = m_manager->EstablishConnection(m_serverAddress.c_str(), m_serverPort);
		if(m_con)
		{
			m_con->SetHandler(this);
			m_conState = CON_NEGOTIATE;
			m_conTimeout = time(nullptr) + 20;//m_reconnectTimeout;
		}
		break;
	case CON_NEGOTIATE:
		if(m_con->GetStatus() == TcpConnection::StatusConnected)
		{
			m_conState = CON_CONNECT;
			printf("callback here.... connected\n");
		}
		else if(time(nullptr) > 20)
		{
			m_con->Release();
			m_con = nullptr;
			m_conState = CON_DISCONNECT;
		}
		break;
	case CON_CONNECT:
		break;
	default:
		m_conState = CON_DISCONNECT;
		m_con->Release();
		m_con = nullptr;
	}
	m_manager->GiveTime();
}


void TestClient::sendMyMessage(const std::string &msg)
{
    
    if (m_con && m_con->GetStatus()== TcpConnection::StatusConnected)
    {
        g_numSent++;
        printf("sending msg (%s)\n", msg.c_str());
        m_con->Send(msg.c_str(), msg.length());
    }
}


void TestClient::OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen)
{
    g_callbackRecieved = true;
    g_numRecvd++;
    std::string t((const char *)data, dataLen);
    printf("received message: '%s' Length: %d\n", t.c_str(), t.length());
}

void TestClient::OnTerminated(TcpConnection *con)
{
    printf("callback here, disconnected...\n");
    if (m_con)
    {
        m_con->Release();
        m_con = nullptr;
    }
    m_conState = CON_DISCONNECT;
}

