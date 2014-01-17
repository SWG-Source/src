// CustomerServiceConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_CustomerServiceConnection_H
#define	_INCLUDED_CustomerServiceConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class CustomerServiceConnection : public ServerConnection
{
public:
	CustomerServiceConnection(UdpConnectionMT *, TcpClient *);
	~CustomerServiceConnection();
	const unsigned short  getGameServicePort  () const;
	void                  onConnectionClosed  ();
	void                  onConnectionOpened  ();
	void                  onReceive           (const Archive::ByteStream & message);
	void                  setGameServicePort  (const unsigned short gameServicePort);

private:
	CustomerServiceConnection & operator = (const CustomerServiceConnection & rhs);
	CustomerServiceConnection(const CustomerServiceConnection & source);
	unsigned short  gameServicePort;

}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CustomerServiceConnection_H
