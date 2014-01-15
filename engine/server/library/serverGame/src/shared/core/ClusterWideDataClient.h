// ======================================================================
//
// ClusterWideDataClient.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClusterWideDataClient_H
#define INCLUDED_ClusterWideDataClient_H

// ======================================================================

namespace MessageDispatch
{
	class Emitter;
	class MessageBase;
	class Receiver;
}

class NetworkId;
class ValueDictionary;

// ======================================================================

class ClusterWideDataClient
{
public:

	static void          install();
	static void          remove();

	// operations
	static unsigned long getClusterWideData(std::string const & managerName, std::string const & elementNameRegex, bool lockElements, NetworkId const & callbackObjectId);
	static void          releaseClusterWideDataLock(std::string const & managerName, unsigned long lockKey);
	static void          removeClusterWideData(std::string const & managerName, std::string const & elementNameRegex, unsigned long lockKey);
	static void          updateClusterWideData(std::string const & managerName, std::string const & elementNameRegex, ValueDictionary const & dictionary, unsigned long lockKey);
	static void          replaceClusterWideData(std::string const & managerName, std::string const & elementNameRegex, ValueDictionary const & dictionary, bool autoRemove, unsigned long lockKey);

	// message handling
	static void          registerMessage(MessageDispatch::Receiver & messageReceiver);
	static bool          handleMessage(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const & message);
};

// ======================================================================

#endif
