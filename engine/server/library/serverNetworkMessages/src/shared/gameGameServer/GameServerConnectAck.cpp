//========================================================================
//
// GameServerConnectAck.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameServerConnectAck.h"

//-----------------------------------------------------------------------

GameServerConnectAck::GameServerConnectAck() :
		GameNetworkMessage("GameServerConnectAck")
{
}

//-----------------------------------------------------------------------

GameServerConnectAck::GameServerConnectAck(Archive::ReadIterator & source) :
			GameNetworkMessage("GameServerConnectAck")
{
	unpack(source);
}

//-----------------------------------------------------------------------

GameServerConnectAck::~GameServerConnectAck()
{
}	

//-----------------------------------------------------------------------
