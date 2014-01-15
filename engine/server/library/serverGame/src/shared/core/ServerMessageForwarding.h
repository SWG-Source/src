// ======================================================================
//
// ServerMessageForwarding.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerMessageForwarding_H
#define INCLUDED_ServerMessageForwarding_H

// ======================================================================

class GameNetworkMessage;

// ======================================================================

class ServerMessageForwarding
{
public:
	static void begin(uint32 destinationServerPid);
	static void begin(std::vector<uint32> const &destinationServerPids);
	static void beginBroadcast();
	static void end();
	static void send(GameNetworkMessage const &msg);
};

// ======================================================================

#endif // INCLUDED_ServerMessageForwarding_H

