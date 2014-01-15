// ======================================================================
//
// ClusterWideDataManagerList.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClusterWideDataManagerList_H
#define INCLUDED_ClusterWideDataManagerList_H

// ======================================================================

namespace MessageDispatch
{
	class Emitter;
	class MessageBase;
	class Receiver;
}

// ======================================================================

class ClusterWideDataManagerList
{
public:

	static void install();
	static void remove();
	static void update(float time);

	static void registerMessage(MessageDispatch::Receiver & messageReceiver);
	static bool handleMessage(MessageDispatch::Emitter const & source, MessageDispatch::MessageBase const & message);
	static void onGameServerDisconnect(unsigned long processId);
	static void setLockTimeoutValue(int timeout);
	static int  getNumberOfQueuedRequests();
};

// ======================================================================

#endif
