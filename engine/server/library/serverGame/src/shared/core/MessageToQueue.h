// ======================================================================
//
// MessageToQueue.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageToQueue_H
#define INCLUDED_MessageToQueue_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"

class MessageToPayload;
class ServerObject;

// ======================================================================

/**
 * Singleton that handles sending MessageTos
 */
class MessageToQueue : public MessageDispatch::Receiver
{
public:
	static std::string ms_nullString;
	
public:
	static unsigned long getEffectiveMessageToTime();
	
	static void sendMessageToC                     (const NetworkId &objectId, const std::string &method, const std::string &data, int delay, bool guaranteed, NetworkId const & undeliveredCallbackObject=NetworkId::cms_invalid, std::string const & undeliveredCallbackMethod=ms_nullString);
	static void sendMessageToJava                  (const NetworkId &objectId, const std::string &method, const std::vector<int8> &packedData, int delay, bool guaranteed, NetworkId const & undeliveredCallbackObject=NetworkId::cms_invalid, std::string const & undeliveredCallbackMethod=ms_nullString);
	static void sendRecurringMessageToC            (const NetworkId &objectId, const std::string &method, const std::string & data, int delay);
	static void sendRecurringMessageToJava         (const NetworkId &objectId, const std::string &method, const std::vector<int8> &packedData, int delay);
	static void cancelRecurringMessageTo           (const NetworkId &objectId, const std::string &method);
	static void update                             (float frameTime);
	static void addToScheduler                     (ServerObject const & object, unsigned long callTime);
	static void onGameServerDisconnect             (uint32 serverId);
	static std::string debugGetLastKnownLocations  ();
	static std::string debugGetSchedulerData       ();
	static int getTotalMessages                    ();
	static int getBackloggedObjectCount            ();
	static void incrementBackloggedObjectCount     ();
	virtual void receiveMessage                    (MessageDispatch::Emitter const &, MessageDispatch::MessageBase const &);

	static MessageToQueue & getInstance();
	static bool isInstalled            ();
	static void remove                 ();
	static void install                ();

private:
	MessageToQueue();
	~MessageToQueue();
};

// ======================================================================

#endif
