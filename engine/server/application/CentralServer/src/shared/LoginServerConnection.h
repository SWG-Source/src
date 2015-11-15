// LoginServerConnection.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_LoginServerConnection_H
#define	_LoginServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

class LoginServerCommandChannel;

//-----------------------------------------------------------------------

class LoginServerConnection : public ServerConnection
{
public:
	LoginServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	LoginServerConnection(UdpConnectionMT *, TcpClient *);
	~LoginServerConnection();

	void onConnectionClosed();
	void onConnectionOpened();
	void onReceive(const Archive::ByteStream & message);

private:
	LoginServerConnection(const LoginServerConnection&);
	LoginServerConnection& operator=(const LoginServerConnection&);
}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

#endif	// _LoginServerConnection_H

