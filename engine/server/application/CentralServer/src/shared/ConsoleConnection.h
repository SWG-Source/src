// ConsoleConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConsoleConnection_H
#define	_INCLUDED_ConsoleConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

class ConsoleCommandParser;

//-----------------------------------------------------------------------

class ConsoleConnection : public ServerConnection
{
public:
	ConsoleConnection();
	ConsoleConnection(UdpConnectionMT *, TcpClient *);
	~ConsoleConnection();

	static void  onCommandComplete  (const std::string & result, const int trackId);
	void         onReceive          (const Archive::ByteStream & message);
private:
	ConsoleConnection & operator = (const ConsoleConnection & rhs);
	ConsoleConnection(const ConsoleConnection & source);
	void         parseCommand       (const std::string & cmd);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConsoleConnection_H
