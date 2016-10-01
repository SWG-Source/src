// ChatServerConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatServerConnection_H
#define	_INCLUDED_ChatServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class ChatServerConnection : public ServerConnection
{
public:
	ChatServerConnection(const std::string & address, const unsigned short port);
	~ChatServerConnection();

	void onConnectionClosed();
	void onReceive(const Archive::ByteStream &message);

private:
	ChatServerConnection & operator = (const ChatServerConnection & rhs);
	ChatServerConnection(const ChatServerConnection & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatServerConnection_H
