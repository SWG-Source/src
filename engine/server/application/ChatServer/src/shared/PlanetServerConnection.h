// PlanetServerConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_PlanetServerConnection_H
#define	_INCLUDED_PlanetServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class PlanetServerConnection : public ServerConnection
{
public:
	PlanetServerConnection(UdpConnectionMT *, TcpClient *);
	virtual ~PlanetServerConnection();

	void          onConnectionClosed       ();
	void          onConnectionOpened       ();
	void          onReceive                (const Archive::ByteStream & bs);
	
private:
	PlanetServerConnection();
	PlanetServerConnection & operator = (const PlanetServerConnection & rhs);
	PlanetServerConnection(const PlanetServerConnection & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_PlanetServerConnection_H
