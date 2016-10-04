// GameConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "GameConnection.h"
#include "ConfigTaskManager.h"
#include "Archive/Archive.h"
#include "TaskManager.h"
#include "sharedFoundation/Clock.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "ProcessSpawner.h"

#include <algorithm>

//-----------------------------------------------------------------------

namespace GameConnectionNamespace
{
	std::vector<GameConnection *> s_gameConnections;
}

using namespace GameConnectionNamespace;

//-----------------------------------------------------------------------

GameConnection::GameConnection(TaskConnection * c) :
TaskHandler(),
connection(c),
m_lastKeepalive(0),
m_pid(0),
m_commandLine(),
m_timeLastKilled(0),
m_loggedKill(false),
m_loggedKillForceCore(false)
{
	TaskManager::addToGameConnections(1);
	s_gameConnections.push_back(this);
}

//-----------------------------------------------------------------------

GameConnection::~GameConnection()
{
	std::vector<GameConnection *>::iterator f = std::find(s_gameConnections.begin(), s_gameConnections.end(), this);
	if(f != s_gameConnections.end())
		s_gameConnections.erase(f);
	
	connection = 0;
	TaskManager::addToGameConnections(-1);
}

//-----------------------------------------------------------------------

void GameConnection::install()
{
}

//-----------------------------------------------------------------------

void GameConnection::remove()
{
	s_gameConnections.clear();
}

//-----------------------------------------------------------------------

void GameConnection::receive(const Archive::ByteStream & message)
{
	Archive::ReadIterator r(message);
	GameNetworkMessage m(r);

	r = message.begin();
	if(m.isType("GameServerTaskManagerKeepAlive"))
	{
		if(! m_pid)
		{
			GenericValueTypeMessage<unsigned long> msg(r);
			m_pid = msg.getValue();

#ifndef _WIN32
			// get the command line
			if (m_pid && m_commandLine.empty())
			{
				char filename[30];
				snprintf(filename, sizeof(filename)-1, "/proc/%lu/cmdline", m_pid);

				// format of the cmdline file is a nullptr separates every
				// parameter, so we'll have to replace the NULLs with spaces
				FILE *inFile = fopen(filename,"rb");
				if (inFile)
				{
					char buffer[512];
					int bytesRead = fread(buffer, 1, sizeof(buffer)-1, inFile);
					if ((bytesRead > 0) && (bytesRead < static_cast<int>(sizeof(buffer))))
					{
						buffer[bytesRead] = '\0';

						for (int i = 0; i < bytesRead; ++i)
						{
							if (buffer[i] == '\0')
							{
								buffer[i] = ' ';
							}
						}

						m_commandLine = buffer;
					}

					fclose(inFile);
				}
			}
#endif
		}
		m_lastKeepalive = Clock::timeSeconds();
	}
}

//-----------------------------------------------------------------------

void GameConnection::update()
{
	std::vector<GameConnection *>::iterator i;
	std::vector<GameConnection *> v = s_gameConnections;

	const unsigned long currentTime = Clock::timeSeconds();
	const unsigned long gameServerTimeout = ConfigTaskManager::getGameServerTimeout();
	
	if (gameServerTimeout > 0)
	{
		for (i = v.begin(); i != v.end(); ++i)
		{
			GameConnection * g = (*i);
			if (g->m_lastKeepalive && g->m_pid && ((g->m_lastKeepalive + gameServerTimeout) < currentTime))
			{
				// attempt ProcessSpawner::forceCore() once per minute and if that doesn't
				// work after a period of gameServerTimeout, then do a ProcessSpawner::kill()
				if ((currentTime - g->m_lastKeepalive) > (2 * gameServerTimeout))
				{
					g->m_timeLastKilled = currentTime;
					LOG("ServerHang", ("killing (kill) GameServer %s,%d (%s) because it has not provided a keepalive message in %d seconds", NetworkHandler::getHumanReadableHostName().c_str(), g->m_pid, g->m_commandLine.c_str(), currentTime - g->m_lastKeepalive));
					WARNING(true, ("killing (kill) GameServer %s,%d (%s) because it has not provided a keepalive message in %d seconds", NetworkHandler::getHumanReadableHostName().c_str(), g->m_pid, g->m_commandLine.c_str(), currentTime - g->m_lastKeepalive));

					// one time logging to customer service channel
					if (!g->m_loggedKill)
					{
						g->m_loggedKill = true;
						LOG("TaskManager", ("ServerHang: killing (kill) GameServer %s,%d (%s) because it has not provided a keepalive message in %d seconds", NetworkHandler::getHumanReadableHostName().c_str(), g->m_pid, g->m_commandLine.c_str(), currentTime - g->m_lastKeepalive));
					}

					ProcessSpawner::kill(g->m_pid);
				}
				else if ((g->m_timeLastKilled == 0) || ((currentTime - g->m_timeLastKilled) >= 60))
				{
					g->m_timeLastKilled = currentTime;
					LOG("ServerHang", ("killing (forceCore) GameServer %s,%d (%s) because it has not provided a keepalive message in %d seconds", NetworkHandler::getHumanReadableHostName().c_str(), g->m_pid, g->m_commandLine.c_str(), currentTime - g->m_lastKeepalive));
					WARNING(true, ("killing (forceCore) GameServer %s,%d (%s) because it has not provided a keepalive message in %d seconds", NetworkHandler::getHumanReadableHostName().c_str(), g->m_pid, g->m_commandLine.c_str(), currentTime - g->m_lastKeepalive));

					// one time logging to customer service channel
					if (!g->m_loggedKillForceCore)
					{
						g->m_loggedKillForceCore = true;
						LOG("TaskManager", ("ServerHang: killing (forceCore) GameServer %s,%d (%s) because it has not provided a keepalive message in %d seconds", NetworkHandler::getHumanReadableHostName().c_str(), g->m_pid, g->m_commandLine.c_str(), currentTime - g->m_lastKeepalive));
					}

					ProcessSpawner::forceCore(g->m_pid);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------
