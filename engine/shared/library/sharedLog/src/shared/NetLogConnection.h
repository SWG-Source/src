// ======================================================================
//
// NetLogConnection.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NetLogConnection_H
#define INCLUDED_NetLogConnection_H

// ======================================================================

#include "sharedNetwork/Connection.h"

// ======================================================================

class NetLogObserver;

// ======================================================================

class NetLogConnection: public Connection
{
public:
	explicit NetLogConnection(NetLogObserver *owner);
	virtual ~NetLogConnection();

	virtual void onConnectionClosed();
	virtual void onConnectionOpened();
	virtual void onReceive(Archive::ByteStream const &);

	void clearOwner();

protected:
	virtual bool isNetLogConnection() const;

private:
	NetLogConnection();
	NetLogConnection(NetLogConnection const &);
	NetLogConnection &operator=(NetLogConnection const &);

	NetLogObserver *m_owner;
};

// ======================================================================

#endif

