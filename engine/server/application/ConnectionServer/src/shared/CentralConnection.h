// CentralConnection.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_CentralConnection_H
#define	_CentralConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class CentralConnection : public ServerConnection
{
public:
	             CentralConnection   (const std::string & address, const unsigned short port);
	             CentralConnection(UdpConnectionMT *, TcpClient *);
	             ~CentralConnection();
	void         onConnectionClosed();
	void         onConnectionOpened();
	void         onReceive(const Archive::ByteStream & message);
	
private:
	CentralConnection();
	CentralConnection(const CentralConnection&);
	CentralConnection& operator=(const CentralConnection&);
};

//-----------------------------------------------------------------------

#endif	// _CentralConnection_H
