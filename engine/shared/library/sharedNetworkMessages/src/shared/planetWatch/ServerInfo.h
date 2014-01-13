// ServerInfo.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ServerInfo_H
#define	_INCLUDED_ServerInfo_H

//-----------------------------------------------------------------------

struct ServerInfo
{
	std::string    ipAddress;
	unsigned long  serverId;
	unsigned long  systemPid;
	std::string    sceneId;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ServerInfo_H
