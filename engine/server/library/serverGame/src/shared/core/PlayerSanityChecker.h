// ======================================================================
//
// PlayerSanityChecker.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _PlayerSanityChecker_H_
#define _PlayerSanityChecker_H_

// ======================================================================

class NetworkId;
class ServerObject;

// ======================================================================

class PlayerSanityChecker
{
public:
	static void add(NetworkId const &id);
	static void update(float time);
	static void handlePlayerSanityCheck(NetworkId const &id, uint32 fromServerId);
	static void handlePlayerSanityCheckSuccess(NetworkId const &id);
	static void handlePlayerSanityCheckProxy(NetworkId const &id, uint32 fromServerId);
	static void handlePlayerSanityCheckProxyFail(NetworkId const &id, uint32 fromServerId);
	static void enable(bool flag);
};

// ======================================================================

#endif // _PlayerSanityChecker_H_

