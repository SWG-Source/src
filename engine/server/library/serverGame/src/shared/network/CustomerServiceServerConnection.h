// CustomerServiceServerConnection.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

#ifndef INCLUDED_CustomerServiceServerConnection_H
#define INCLUDED_CustomerServiceServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class CustomerServiceServerConnection : public ServerConnection
{
public:

	CustomerServiceServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	virtual ~CustomerServiceServerConnection();

	void onConnectionClosed();
	void onConnectionOpened();
	void onReceive(const Archive::ByteStream & bs);
	
private:

	CustomerServiceServerConnection();
	CustomerServiceServerConnection & operator = (const CustomerServiceServerConnection & rhs);
	CustomerServiceServerConnection(const CustomerServiceServerConnection & source);
};

//-----------------------------------------------------------------------

#endif // INCLUDED_CustomerServiceServerConnection_H
