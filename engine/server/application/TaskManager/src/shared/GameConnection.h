// GameConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_GameConnection_H
#define	_INCLUDED_GameConnection_H

//-----------------------------------------------------------------------

#include "TaskHandler.h"

#include <string>

class TaskConnection;

//-----------------------------------------------------------------------

class GameConnection : public TaskHandler
{
public:
	explicit GameConnection(TaskConnection * connection);
	~GameConnection();

	static void install();
	void receive(const Archive::ByteStream & message);
	static void remove();
	static void update();
	
private:
	GameConnection & operator = (const GameConnection & rhs);
	GameConnection(const GameConnection & source);
	TaskConnection *  connection;
	unsigned long     m_lastKeepalive;
	unsigned long     m_pid;
	std::string       m_commandLine;
	unsigned long     m_timeLastKilled;
	bool              m_loggedKill;
	bool              m_loggedKillForceCore;
}; //lint !e1712

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GameConnection_H
