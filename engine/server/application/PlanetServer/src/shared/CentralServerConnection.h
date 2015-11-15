// ======================================================================
//
// CentralServerConnection.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CentralServerConnection_H
#define INCLUDED_CentralServerConnection_H

// ======================================================================

#include "serverUtility/ServerConnection.h"

// ======================================================================

class CentralServerConnection : public ServerConnection
{
public:
	CentralServerConnection   (const std::string & remoteAddress, const unsigned short port);
	~CentralServerConnection  ();

	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	const std::string &	          getClusterName          () const;
	virtual void                  onReceive               (const Archive::ByteStream & message);
private:
	CentralServerConnection (const CentralServerConnection&);
	CentralServerConnection& operator= (const CentralServerConnection&);
	CentralServerConnection();
};

// ======================================================================

#endif
