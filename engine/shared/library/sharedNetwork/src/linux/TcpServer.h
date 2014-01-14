// TcpServer.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TcpServer_H
#define	_INCLUDED_TcpServer_H

//-----------------------------------------------------------------------

#include "Address.h"
#include <string>
#include <vector>
#include <map>
#include <set>

//-----------------------------------------------------------------------

class Service;
class TcpClient;

//-----------------------------------------------------------------------

class TcpServer
{
public:
	TcpServer(Service * service, const std::string & bindAddress, const unsigned short bindPort);
	~TcpServer();

	const std::string &   getBindAddress      () const;
	const unsigned short  getBindPort         () const;
	void                  onConnectionClosed  (TcpClient *);
	void                  removeClient        (TcpClient *);
	void                  update              ();

private:
	TcpServer & operator = (const TcpServer & rhs);
	TcpServer(const TcpServer & source);

private:
	Address                     m_bindAddress;
	int                         m_handle;
	Service *                   m_service;
	std::map<int, TcpClient *>  m_connections;
	std::vector<struct pollfd>  m_connectionSockets;
	unsigned char *             m_inputBuffer;
	int                         m_inputBufferSize;
	std::set<TcpClient *>       m_pendingDestroys;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TcpServer_H
