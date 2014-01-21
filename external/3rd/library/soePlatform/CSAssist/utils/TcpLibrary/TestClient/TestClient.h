#ifndef TESTCLIENT_H
#define TESTCLIENT_H

#include <TcpLibrary/TcpManager.h>
#include <TcpLibrary/TcpConnection.h>
#include <TcpLibrary/TcpHandlers.h>
#include <string>
#include <time.h>

	enum ConState
	{
		CON_DISCONNECT,
		CON_NEGOTIATE,
		CON_CONNECT
	};

class TestClient : public TcpConnectionHandler
{
public:
    TestClient(const std::string &serverAddress, unsigned short serverPort);
    virtual ~TestClient();

    void process();

    void sendMyMessage(const std::string &msg);


    void OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen);
    void OnTerminated(TcpConnection *con);

    ConState getState(){ return m_conState; }

private:
    ConState m_conState;
    TcpManager *m_manager;
    TcpConnection *m_con;
    std::string m_serverAddress;
    unsigned short m_serverPort;
    time_t m_conTimeout;
};



#endif //TESTCLIENT_H
