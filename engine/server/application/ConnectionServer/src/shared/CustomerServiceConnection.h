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
	virtual ~CustomerServiceConnection();
	void  addClient(Client *);
	void  onConnectionClosed  ();
	void  onConnectionOpened  ();
	void  onReceive           (const Archive::ByteStream &);
	void  removeClient(Client *);
	const std::set<Client *> & getClients() const;
private:
	CustomerServiceConnection();
	CustomerServiceConnection & operator = (const CustomerServiceConnection & rhs);
	CustomerServiceConnection(const CustomerServiceConnection & source);
	std::set<Client *> clients;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CustomerServiceConnection_H
