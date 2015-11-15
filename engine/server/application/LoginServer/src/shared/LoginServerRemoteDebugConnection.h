// ======================================================================
//
// LoginServerRemoteDebugConnection.h
// copyright 2001, Sony Online Entertainment
//
// ======================================================================

#ifndef _LoginServerRemoteDebugConnection_H
#define _LoginServerRemoteDebugConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class LoginServerRemoteDebugConnection : public ServerConnection
{
public:
	LoginServerRemoteDebugConnection(UdpConnectionMT *, TcpClient *);
	virtual ~LoginServerRemoteDebugConnection();
	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	void	                      onReceive               (const Archive::ByteStream & message);

private:
	LoginServerRemoteDebugConnection(const LoginServerRemoteDebugConnection & source);
	LoginServerRemoteDebugConnection & operator = (const LoginServerRemoteDebugConnection & rhs);
}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

#endif	// _ServerConnectionHandler_H
