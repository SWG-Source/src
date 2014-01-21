// DatabaseServerConnection.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_DatabaseServerConnection_H
#define	_DatabaseServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class DatabaseServerConnection : public ServerConnection
{
public:
	DatabaseServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~DatabaseServerConnection();

	void  onConnectionClosed();
	void  onConnectionOpened();
	void  onReceive(const Archive::ByteStream & message);

private:
	DatabaseServerConnection(const DatabaseServerConnection&);
	DatabaseServerConnection& operator= (const DatabaseServerConnection&);

}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

#endif	// _DatabaseServerConnection_H


