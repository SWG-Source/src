// TransferServerConnection.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_TransferServerConnection_H
#define	_TransferServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class TransferServerConnection : public ServerConnection
{
public:
	TransferServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~TransferServerConnection();

	void  onConnectionClosed();
	void  onConnectionOpened();
	void  onReceive(const Archive::ByteStream & message);

private:
	TransferServerConnection(const TransferServerConnection&);
	TransferServerConnection& operator= (const TransferServerConnection&);

}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

#endif	// _TransferServerConnection_H


