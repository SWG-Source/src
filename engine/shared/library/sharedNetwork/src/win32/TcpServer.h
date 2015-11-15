// TcpServer.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TcpServer_H
#define	_INCLUDED_TcpServer_H

//-----------------------------------------------------------------------

#include <winsock2.h>
#include <string>
#include <vector>

//-----------------------------------------------------------------------

class Service;
class TcpClient;

//-----------------------------------------------------------------------

class TcpServer
{
public:
	TcpServer(Service * service, const std::string & bindAddress, const unsigned short bindPort);
	~TcpServer();

	TcpClient *           accept              ();
	const std::string &   getBindAddress      () const;
	const unsigned short  getBindPort         () const;
	void                  onConnectionClosed  (TcpClient *);
	void                  update              ();

private:
	TcpServer & operator = (const TcpServer & rhs);
	TcpServer(const TcpServer & source);

	void  queueAccept  ();

private:
	SOCKET                    m_handle;
	HANDLE                    m_localIOCP;
	std::vector<TcpClient *>  m_pendingConnections;
	std::string               m_bindAddress;
	unsigned short            m_bindPort;
	Service *                 m_service;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TcpServer_H
