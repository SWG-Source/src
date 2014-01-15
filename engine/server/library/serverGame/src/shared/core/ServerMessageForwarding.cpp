// ======================================================================
//
// ServerMessageForwarding.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerMessageForwarding.h"

#include "serverGame/GameServer.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#ifdef _DEBUG
#include "serverUtility/ServerConnection.h"
#endif

// ======================================================================

namespace ServerMessageForwardingNamespace
{
	std::vector<int> s_forwardCounts;
	std::vector<std::vector<uint32> > s_forwardDestinationServerPids;
}
using namespace ServerMessageForwardingNamespace;

// ======================================================================

void ServerMessageForwarding::begin(uint32 destinationServerPid)
{
	std::vector<uint32> serverList;
	serverList.push_back(destinationServerPid);
	begin(serverList);
}

// ----------------------------------------------------------------------

void ServerMessageForwarding::begin(std::vector<uint32> const &destinationServerPids)
{
	if (s_forwardDestinationServerPids.empty() || s_forwardDestinationServerPids.back() != destinationServerPids)
	{
		s_forwardCounts.push_back(1);
		s_forwardDestinationServerPids.push_back(destinationServerPids);

		GenericValueTypeMessage<std::vector<uint32> > const beginForwardMessage("BeginForward", destinationServerPids);
		GameServer::getInstance().sendToPlanetServer(beginForwardMessage);
	}
	else
		++s_forwardCounts.back();
}

// ----------------------------------------------------------------------

void ServerMessageForwarding::beginBroadcast()
{
	begin(GameServer::getInstance().getAllGameServerPids());
}

// ----------------------------------------------------------------------

void ServerMessageForwarding::end()
{
	FATAL(s_forwardCounts.empty(), ("ServerMessageForwarding::end without a begin?"));

	if (--s_forwardCounts.back() == 0)
	{
		GameNetworkMessage const endForwardMessage("EndForward");
		GameServer::getInstance().sendToPlanetServer(endForwardMessage);

		s_forwardCounts.pop_back();
		s_forwardDestinationServerPids.pop_back();
	}
}

// ----------------------------------------------------------------------

void ServerMessageForwarding::send(GameNetworkMessage const &msg)
{
	DEBUG_FATAL(!ServerConnection::isMessageForwardable(msg.getType()), ("Tried to forward message of type %lu which is not in the forwardable message table.", msg.getType()));

	GameServer::getInstance().sendToPlanetServer(msg);
}

// ======================================================================

