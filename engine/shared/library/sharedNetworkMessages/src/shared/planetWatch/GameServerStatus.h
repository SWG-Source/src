// GameServerStatus.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_GameServerStatus_H
#define	_INCLUDED_GameServerStatus_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ServerInfo.h"
#include "sharedNetworkMessages/ServerInfoArchive.h"

//-----------------------------------------------------------------------

class GameServerStatus : public GameNetworkMessage
{
public:
	GameServerStatus(const bool isOnline, const ServerInfo & info);
	explicit GameServerStatus(Archive::ReadIterator & source);
	~GameServerStatus();

	const bool          isOnline       () const;
	const ServerInfo &  getServerInfo  () const;

private:
	GameServerStatus & operator = (const GameServerStatus & rhs);
	GameServerStatus(const GameServerStatus & source);
	
	Archive::AutoVariable<bool>        m_online;
	Archive::AutoVariable<ServerInfo>  m_serverInfo;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GameServerStatus_H
