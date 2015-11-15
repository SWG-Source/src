// LoginConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_LoginConnection_H
#define	_INCLUDED_LoginConnection_H

//-----------------------------------------------------------------------

#include "sharedNetwork/Connection.h"

//-----------------------------------------------------------------------

class LoginConnection : public Connection
{
public:
	LoginConnection(const std::string & remoteAddress, const unsigned short port);
	~LoginConnection();

	virtual void   onConnectionClosed      ();
	virtual void   onConnectionOpened      ();
	virtual void   onConnectionOverflowing (const unsigned int bytesPending);
	virtual void   onReceive               (const Archive::ByteStream & message);

private:
	LoginConnection & operator = (const LoginConnection & rhs);
	LoginConnection(const LoginConnection & source);

}; //lint !e1712 

//-----------------------------------------------------------------------

#endif	// _INCLUDED_LoginConnection_H
