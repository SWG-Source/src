// GameTaskManagerMessages.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_GameTaskManagerMessages_H
#define	_INCLUDED_GameTaskManagerMessages_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

namespace GameTaskManagerMessageConstants
{
	const unsigned char        ShutdownGameServer = 1;
	const unsigned char        GameSetClusterName = 2;
	const unsigned char        GameSetDbProcess   = 3;
}

//-----------------------------------------------------------------------

class TaskShutdownGameServer : public GameNetworkMessage
{
public:
	TaskShutdownGameServer();
	TaskShutdownGameServer(Archive::ReadIterator & source);
	virtual ~TaskShutdownGameServer();
private:
	TaskShutdownGameServer(const TaskShutdownGameServer & source);
	TaskShutdownGameServer & operator = (const TaskShutdownGameServer & rhs);
};

//-----------------------------------------------------------------------

class GameSetClusterName : public GameNetworkMessage
{
public:
	GameSetClusterName(const std::string & newClusterName, const std::string & newGameServerName);
	GameSetClusterName(Archive::ReadIterator & source);
	virtual ~GameSetClusterName();

	const std::string & getClusterName    (void) const;
	const std::string & getGameServerName (void) const;

private:
	Archive::AutoVariable<std::string> clusterName;
	Archive::AutoVariable<std::string> gameServerName;

	GameSetClusterName();
	GameSetClusterName(const GameSetClusterName&);
	GameSetClusterName& operator= (const GameSetClusterName&);
};

//-----------------------------------------------------------------------

inline const std::string & GameSetClusterName::getClusterName(void) const
{
	return clusterName.get();
}

//-----------------------------------------------------------------------

inline const std::string & GameSetClusterName::getGameServerName(void) const
{
	return gameServerName.get();
}

//-----------------------------------------------------------------------
class GameSetDbProcess : public GameNetworkMessage
{
public:
	GameSetDbProcess();
	GameSetDbProcess(Archive::ReadIterator & source);
	virtual ~GameSetDbProcess();

private:
	GameSetDbProcess(const GameSetDbProcess&);
	GameSetDbProcess& operator= (const GameSetDbProcess&);
};

//-----------------------------------------------------------------------

/**
 * Sent from:  various processes (only DatabaseServer yet, as of when this comment was written)
 * Sent to:    task manager
 * Action:     Notify the task manager that the process isn't doing anything
 *             significant, e.g. has no data to save or load
 */
class ServerIdleMessage : public GameNetworkMessage
{
  public:
	ServerIdleMessage(bool isIdle);
	ServerIdleMessage(Archive::ReadIterator & source);
	virtual ~ServerIdleMessage();

	bool getIsIdle();
	
  private:
	Archive::AutoVariable<bool> m_isIdle;
	
  private:
	ServerIdleMessage(const ServerIdleMessage&);
	ServerIdleMessage& operator= (const ServerIdleMessage&);
};

// ----------------------------------------------------------------------

inline bool ServerIdleMessage::getIsIdle()
{
	return m_isIdle.get();
}

// ----------------------------------------------------------------------

#endif	// _INCLUDED_GameTaskManagerMessages_H
