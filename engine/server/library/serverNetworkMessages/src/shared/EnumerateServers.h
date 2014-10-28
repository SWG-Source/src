// EnumerateServers.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_EnumerateServers_H
#define	_INCLUDED_EnumerateServers_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class EnumerateServers : public GameNetworkMessage
{
public:
	enum ServerTypes
	{
		CONNECTION_SERVER,
		GAME_SERVER,
		PLANET_SERVER
	};

	EnumerateServers(const bool add, const std::string &address, const unsigned short port, const int serverType);
	explicit EnumerateServers(Archive::ReadIterator & source);
	~EnumerateServers();

	const bool             getAdd         () const;
	const std::string &    getAddress     () const;
	const unsigned short   getPort        () const;
	const int              getServerType  () const;

private:
	EnumerateServers & operator = (const EnumerateServers & rhs);
	EnumerateServers(const EnumerateServers & source);

	Archive::AutoVariable<bool>            add;
	Archive::AutoVariable<std::string>     address;
	Archive::AutoVariable<unsigned short>  port;
	Archive::AutoVariable<int>             serverType;
};

//-----------------------------------------------------------------------

inline const bool EnumerateServers::getAdd() const
{
	return add.get();
}

//-----------------------------------------------------------------------

inline const std::string & EnumerateServers::getAddress() const
{
	return address.get();
}

//-----------------------------------------------------------------------

inline const unsigned short EnumerateServers::getPort() const
{
	return port.get();
}

//-----------------------------------------------------------------------

inline const int EnumerateServers::getServerType() const
{
	return serverType.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_EnumerateServers_H
