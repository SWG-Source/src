// MetricsConnection.h
// copyright 2001 Verant Interactive


#ifndef	_MetricsConnection_H
#define	_MetricsConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class MetricsConnection : public ServerConnection
{
public:
	MetricsConnection(const std::string & remoteAddress, const unsigned short remotePort);
	MetricsConnection(UdpConnectionMT * u, TcpClient * t);
	virtual ~MetricsConnection();

    virtual void  onConnectionClosed();
    virtual void  onConnectionOpened();
    virtual void  onReceive(const Archive::ByteStream & message);

private:
	MetricsConnection(const MetricsConnection&);
	MetricsConnection& operator= (const MetricsConnection&);
}; 

//-----------------------------------------------------------------------

#endif	
