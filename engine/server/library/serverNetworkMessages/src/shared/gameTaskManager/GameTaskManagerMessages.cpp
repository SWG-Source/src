// GameTaskManagerMessages.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameTaskManagerMessages.h"

//-----------------------------------------------------------------------

TaskShutdownGameServer::TaskShutdownGameServer() :
GameNetworkMessage("TaskShutdownGameServer")
{
}

//-----------------------------------------------------------------------

TaskShutdownGameServer::TaskShutdownGameServer(Archive::ReadIterator & source) :
GameNetworkMessage("TaskShutdownGameServer")
{
	unpack(source);
}

//-----------------------------------------------------------------------

TaskShutdownGameServer::~TaskShutdownGameServer()
{
}

//-----------------------------------------------------------------------

GameSetClusterName::GameSetClusterName(const std::string & newClusterName, const std::string & newGameServerName) :
GameNetworkMessage("GameSetClusterName"),
clusterName(newClusterName),
gameServerName(newGameServerName)
{
	addVariable(clusterName);
	addVariable(gameServerName);
}

//-----------------------------------------------------------------------

GameSetClusterName::GameSetClusterName(Archive::ReadIterator & source) :
GameNetworkMessage("GameSetClusterName"),
clusterName(),
gameServerName()
{
	addVariable(clusterName);
	addVariable(gameServerName);
	unpack(source);
}

//-----------------------------------------------------------------------

GameSetClusterName::~GameSetClusterName()
{
}

//-----------------------------------------------------------------------

GameSetDbProcess::GameSetDbProcess() :
GameNetworkMessage("GameSetDbProcess")
{

}

//-----------------------------------------------------------------------

GameSetDbProcess::GameSetDbProcess(Archive::ReadIterator & source) :
GameNetworkMessage("GameSetDbProcess")
{
	unpack(source);
}

//-----------------------------------------------------------------------

GameSetDbProcess::~GameSetDbProcess()
{
}

// ======================================================================

ServerIdleMessage::ServerIdleMessage(bool isIdle) :
		GameNetworkMessage("ServerIdleMessage"),
		m_isIdle(isIdle)
{
	addVariable(m_isIdle);
}

// ----------------------------------------------------------------------

ServerIdleMessage::ServerIdleMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ServerIdleMessage"),
		m_isIdle(false)
{
	addVariable(m_isIdle);
	unpack(source);
}

//-----------------------------------------------------------------------

ServerIdleMessage::~ServerIdleMessage()
{
}

//-----------------------------------------------------------------------
