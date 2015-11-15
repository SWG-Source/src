// ======================================================================
//
// PlanetServerConnection.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetServerConnection_H
#define INCLUDED_PlanetServerConnection_H

// ======================================================================

#include "serverUtility/ServerConnection.h"

// ======================================================================

class PlanetServerConnection : public ServerConnection
{
public:
	PlanetServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~PlanetServerConnection();
	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();

private:
	PlanetServerConnection (const PlanetServerConnection&);
	PlanetServerConnection& operator= (const PlanetServerConnection&);
};

// ======================================================================

#endif
