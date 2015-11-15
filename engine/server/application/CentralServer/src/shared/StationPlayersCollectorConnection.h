// StationPlayersCollectorConnection.h
// copyright 2005 Sony Online Entertainment
// Author: Doug Mellencamp

#ifndef	_StationPlayersCollectorConnection_H
#define	_StationPlayersCollectorConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class StationPlayersCollectorConnection : public ServerConnection
{
public:
	StationPlayersCollectorConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~StationPlayersCollectorConnection();

	void  onConnectionClosed();
	void  onConnectionOpened();
	void  onReceive(const Archive::ByteStream & message);

private:
	StationPlayersCollectorConnection(const StationPlayersCollectorConnection&);
	StationPlayersCollectorConnection& operator= (const StationPlayersCollectorConnection&);

}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

#endif	// _StationPlayersCollectorConnection_H


