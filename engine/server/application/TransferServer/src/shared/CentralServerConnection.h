// CentralServerConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

#ifndef	_INCLUDED_CentralServerConnection_H
#define	_INCLUDED_CentralServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"
#include <map>
#include <string>

class CentralServerConnection;

//-----------------------------------------------------------------------

class CentralServerConnection : public ServerConnection
{
public:
	CentralServerConnection(UdpConnectionMT *, TcpClient * t);
	~CentralServerConnection();

	virtual void  onConnectionClosed  ();
	virtual void  onConnectionOpened  ();
	virtual void  onReceive           (const Archive::ByteStream & message);

	static CentralServerConnection *  getCentralServerConnectionForGalaxy(const std::string & galaxyName);
	void          setGalaxyName       (const std::string & galaxyName);

	static const std::map<std::string, CentralServerConnection *> &  getGalaxies();
	
private:
	CentralServerConnection & operator = (const CentralServerConnection & rhs);
	CentralServerConnection(const CentralServerConnection & source);

	std::string  m_galaxyName;
}; //lint !e1712 default constructor not defined for class 'CentralServerConnection'

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CentralServerConnection_H
