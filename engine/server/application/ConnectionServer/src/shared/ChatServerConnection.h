// ChatServerConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatServerConnection_H
#define	_INCLUDED_ChatServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

class Client;

//-----------------------------------------------------------------------

class ChatServerConnection : public ServerConnection
{
public:
	explicit ChatServerConnection(UdpConnectionMT *, TcpClient *);
	virtual ~ChatServerConnection();
	void  addClient(Client *);
	void  onConnectionClosed  ();
	void  onConnectionOpened  ();
	void  onReceive           (const Archive::ByteStream &);
	void  removeClient(Client *);
	const std::set<Client *> & getClients() const;
private:
	ChatServerConnection();
	ChatServerConnection & operator = (const ChatServerConnection & rhs);
	ChatServerConnection(const ChatServerConnection & source);
	std::set<Client *> clients;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatServerConnection_H
