// ======================================================================
//
// AuthTransferTracker.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _AuthTransferTracker_H_
#define _AuthTransferTracker_H_

// ======================================================================

#include "serverGame/ProxyList.h"

// ======================================================================

class NetworkId;

// ======================================================================

class AuthTransferTracker
{
public:
	static void install();
	static void remove();
	static void update();

	static void beginAuthTransfer(NetworkId const &networkId, uint32 newAuthProcess, ProxyList const &proxyList);
	static void sendConfirmAuthTransfer(NetworkId const &networkId, uint32 fromServer);
	static void handleConfirmAuthTransfer(NetworkId const &networkId, uint32 whichServer);
	static uint32 getAuthTransferDest(NetworkId const &networkId);
	static void handleGameServerDisconnect(uint32 processId);
};

// ======================================================================

#endif // _AuthTransferTracker_H_

