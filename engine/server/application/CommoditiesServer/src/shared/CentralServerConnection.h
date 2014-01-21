// CentralServerConnection.h
// copyright 2004 Verant Interactive
// Author: Justin Randall

#ifndef	_CentralServerConnection_H
#define	_CentralServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class CentralServerConnection : public ServerConnection
{
public:
	             CentralServerConnection   (const std::string & address, const unsigned short port);
	             CentralServerConnection(UdpConnectionMT *, TcpClient *);
	             ~CentralServerConnection();
	void         onConnectionClosed();
	void         onConnectionOpened();
	void         onReceive(const Archive::ByteStream & message);
	
private:
	CentralServerConnection();
	CentralServerConnection(const CentralServerConnection&);
	CentralServerConnection& operator=(const CentralServerConnection&);

};

//-----------------------------------------------------------------------

#endif	// _CentralServerConnection_H
