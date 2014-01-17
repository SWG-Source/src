// CentralServerConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_CentralServerConnection_H
#define	_INCLUDED_CentralServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class CentralServerConnection : public ServerConnection
{
public:
	CentralServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~CentralServerConnection();

	void          onConnectionClosed       ();
	void          onConnectionOpened       ();
	void          onReceive                (const Archive::ByteStream & bs);
private:
	CentralServerConnection & operator = (const CentralServerConnection & rhs);
	CentralServerConnection(const CentralServerConnection & source);
};//lint !e1712 default constructor not defined for class

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CentralServerConnection_H
