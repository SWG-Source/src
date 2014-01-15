// ======================================================================
//
// DatabaseProcessConnection.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DatabaseProcessConnection_H
#define INCLUDED_DatabaseProcessConnection_H

// ======================================================================

#include "serverUtility/ServerConnection.h"

// ======================================================================

class DatabaseProcessConnection: public ServerConnection
{
public:
	DatabaseProcessConnection(std::string const &remoteAddress, unsigned short remotePort, uint32 pid);
	~DatabaseProcessConnection();

	void onConnectionClosed();
	void onConnectionOpened();
	void onReceive(Archive::ByteStream const &message);

	uint32 getPid() const;

private:
	DatabaseProcessConnection(DatabaseProcessConnection const &);
	DatabaseProcessConnection &operator=(DatabaseProcessConnection const &);

	uint32 m_pid;
};

// ======================================================================

#endif // INCLUDED_DatabaseProcessConnection_H

