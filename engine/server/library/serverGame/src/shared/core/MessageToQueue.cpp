// ======================================================================
//
// MessageToQueue.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/MessageToQueue.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "serverNetworkMessages/MessageToAckMessage.h"
#include "serverNetworkMessages/MessageToMessage.h"
#include "serverUtility/ServerClock.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Timer.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/NetworkIdManager.h"

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

std::string MessageToQueue::ms_nullString;

// ======================================================================

namespace MessageToQueueNamespace
{
	/**
	 * A class to keep track of objects we are trying to find on other
	 * servers.
	 */
	class ObjectLocator
	{
	public:
		explicit ObjectLocator(NetworkId const & object);
		~ObjectLocator();
		void addMessage(MessageToPayload const & message);
		bool foundObject(uint32 serverId, bool authoritative);
		bool objectNotOnServer(uint32 serverId);

	private:
		bool removeServer(uint32 serverId);
		void sendMessagesToServer(uint32 serverId);
			
	private:
		NetworkId m_object;
		std::set<uint32> m_waitingForServers;
		std::vector<MessageToPayload> m_messages;
		uint32 m_possibleNonAuthServer;

	private:
		ObjectLocator(); //disable
	};

	/**
	 * A class to keep track of objects who have scheduled messages.
	 */
	class SchedulerItem
	{
	public:
		SchedulerItem(); //required to put these in a tree, but should never actually be used
		SchedulerItem (ServerObject const & object, unsigned long deliveryTime);
		unsigned long getDeliveryTime() const;
		ServerObject * getObject() const;
		CachedNetworkId const & getNetworkId() const;

	public:
		class IsSoonerThan
		{
		public:
			bool operator() (SchedulerItem const & left, SchedulerItem const & right) const;
		};

	private:
		CachedNetworkId m_networkId;
		unsigned long m_deliveryTime;
		//SchedulerItem & operator= (SchedulerItem const &); //disable
	};
	
	typedef std::map<NetworkId, uint32> LastKnownLocationsType;
	typedef std::map<NetworkId, ObjectLocator *> ObjectLocatorsType;
	typedef std::set<SchedulerItem, SchedulerItem::IsSoonerThan> SchedulerItemsType;
	typedef std::vector<MessageToPayload> FrameMessagesType;
	
	MessageToQueue * ms_instance=nullptr;
	LastKnownLocationsType ms_lastKnownLocations;
	ObjectLocatorsType ms_objectLocators;
	SchedulerItemsType ms_schedulerItems;
	FrameMessagesType ms_frameMessages;
	unsigned long ms_effectiveMessageToTime=0;
	int ms_backloggedObjectCount=0;

	void internalSendMessageTo   (MessageToPayload & messageData);
	void sendNetworkMessageTo    (MessageToPayload & data);
	void handleObjectOnServer    (uint32 server, NetworkId const & object, bool authoritative);
	void handleObjectNotOnServer (uint32 server, NetworkId const & object);
	void handleMessageFromNetwork(MessageToPayload & data, uint32 sourceServer);
	void dropMessage             (MessageToPayload & message);
}
using namespace MessageToQueueNamespace;

// ======================================================================

MessageToQueue & MessageToQueue::getInstance()
{
	return *(NON_NULL(ms_instance));
}

// ----------------------------------------------------------------------

bool MessageToQueue::isInstalled ()
{
	if (ms_instance)
		return true;
	return false;
}

// ----------------------------------------------------------------------

void MessageToQueue::install()
{
	ExitChain::add(remove,"MessageToQueue::remove");
	ms_instance = new MessageToQueue;
}

// ----------------------------------------------------------------------

void MessageToQueue::remove()
{
	delete ms_instance;
	ms_instance = nullptr;
}

// ----------------------------------------------------------------------

MessageToQueue::MessageToQueue() :
		MessageDispatch::Receiver()
{
	connectToMessage("MessageToMessage");
	connectToMessage("ObjectNotOnServerMessage");
	connectToMessage("ObjectOnServerMessage");
	connectToMessage("WhoHasMessage");
}

// ----------------------------------------------------------------------

MessageToQueue::~MessageToQueue()
{
	ms_lastKnownLocations.clear();
	for (ObjectLocatorsType::iterator i=ms_objectLocators.begin(); i!=ms_objectLocators.end(); ++i)
		delete i->second;
	ms_objectLocators.clear();
	ms_schedulerItems.clear();
	ms_frameMessages.clear();
}

// ----------------------------------------------------------------------

unsigned long MessageToQueue::getEffectiveMessageToTime() 
{
	return ms_effectiveMessageToTime;
}

// ----------------------------------------------------------------------

void MessageToQueue::update(float frameTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("MessageToQueue::update");
	if (!ServerClock::getInstance().isSet())
		return;  // Do no processing until the serverclock is ready
		
	static Timer resetLastKnownLocationsTimer(static_cast<float>(ConfigServerGame::getLastKnownLocationsResetTimeSec()));
	if (resetLastKnownLocationsTimer.updateZero(frameTime))
	{
		ms_lastKnownLocations.clear();
	}

	ms_effectiveMessageToTime = ServerClock::getInstance().getGameTimeSeconds();
	ms_backloggedObjectCount=0; // Count of backlogged objects resets at the start of the frame.  Backlogged objects will increment the count during the frame

	{
		PROFILER_AUTO_BLOCK_DEFINE("MessageToQueue::update - frame messages");
		FrameMessagesType copyFrameMessages(ms_frameMessages);
		ms_frameMessages.clear();
		
		// Deliver all messages sent during the last frame.  The objects will either handle them right away if appropriate, or will
		// enqueue them for later
		for (FrameMessagesType::iterator i=copyFrameMessages.begin(); i!=copyFrameMessages.end(); ++i)
		{
			ServerObject * const object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(i->getNetworkId()));
			if (object)
				object->deliverMessageTo(*i);
			else
				sendNetworkMessageTo(*i);
		}
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE("MessageToQueue::update - queued messages");
		// Handle all objects who have queued messages whos time has come
		std::vector<SchedulerItem> newItems;
		while (!ms_schedulerItems.empty() && ms_schedulerItems.begin()->getDeliveryTime() < ms_effectiveMessageToTime)
		{
			SchedulerItem const & schedulerItem = *ms_schedulerItems.begin();
			ServerObject * const object = schedulerItem.getObject();  
			if (object)
			{
				unsigned long nextMessageTime = object->processQueuedMessageTos(ms_effectiveMessageToTime);
				if (nextMessageTime != 0)
					newItems.push_back(SchedulerItem(*object, nextMessageTime));
			}
			ms_schedulerItems.erase(ms_schedulerItems.begin());
		}

		for (std::vector<SchedulerItem>::iterator i=newItems.begin(); i!=newItems.end(); ++i)
		{
			// This item might duplicate one already in the scheduler, in which case the duplicate
			// will be ignored
			IGNORE_RETURN(ms_schedulerItems.insert(*i));
		}
	}
}

// ----------------------------------------------------------------------

void MessageToQueue::receiveMessage(MessageDispatch::Emitter const &, MessageDispatch::MessageBase const &message)
{
	static Archive::ReadIterator ri;
	ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin(); //lint !e1774 // could use dynamic_cast

	const uint32 messageType = message.getType();
	
	switch(messageType) {	
		case constcrc("MessageToMessage") :
		{
			ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
			MessageToMessage const msg(ri);

			// const_cast avoids having to make another copy of the data.  Since we expect to receive lots of these mesages,
			// avoiding making extra copies matters.  In this particular case, we know it is safe since the message is about
			// to go out of scope anyway.
			handleMessageFromNetwork(const_cast<MessageToPayload&>(msg.getData()), msg.getSourceServerPid());
			
			break;
		}
		case constcrc("WhoHasMessage") :
		{
			// Attempt to find the object, tell the source server if we have it or not
			GenericValueTypeMessage<std::pair<uint32, NetworkId> > const whoHasMessage(ri);
			uint32 sourceServer = whoHasMessage.getValue().first;
			NetworkId const & networkId = whoHasMessage.getValue().second;
			
			ServerMessageForwarding::begin(sourceServer);
			ServerObject const * const object = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(networkId));
			if (object)
			{
				GenericValueTypeMessage<std::pair<uint32, std::pair<NetworkId, bool> > > reply(
					"ObjectOnServerMessage",
					std::make_pair(GameServer::getInstance().getProcessId(),
								   std::make_pair(networkId,
												  object->isAuthoritative())));
				ServerMessageForwarding::send(reply);
			}
			else
			{
				GenericValueTypeMessage<std::pair<uint32, NetworkId> > reply(
					"ObjectNotOnServerMessage",
					std::make_pair(GameServer::getInstance().getProcessId(),
								   networkId));
				ServerMessageForwarding::send(reply);
			}
			ServerMessageForwarding::end();
			
			break;
		}
		case constcrc("ObjectNotOnServerMessage") :
		{
			GenericValueTypeMessage<std::pair<uint32, NetworkId> > const msg(ri);
			handleObjectNotOnServer(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("ObjectOnServerMessage") :
		{
			GenericValueTypeMessage<std::pair<uint32, std::pair<NetworkId, bool> > > const msg(ri);
			handleObjectOnServer(msg.getValue().first, msg.getValue().second.first, msg.getValue().second.second);
			break;
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Send a message from one object to another, to be handled in C++.
 * The object does not need to be loaded.
 */
void MessageToQueue::sendMessageToC (const NetworkId &objectId, const std::string &method, const std::string &data, int delay, bool guaranteed, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod)
{
	WARNING_DEBUG_FATAL(delay < 0, ("Sent messageTo with negative delay.  Target %s, method %s, delay %f",objectId.getValueString().c_str(), method.c_str(), delay));
	if (delay < 0)
		delay = 0;

	MessageToPayload messageData(objectId, ObjectIdManager::getNewObjectId(), method, data, ServerClock::getInstance().getGameTimeSeconds() + static_cast<unsigned long>(delay), guaranteed, MessageToPayload::DT_c, undeliveredCallbackObject, undeliveredCallbackMethod, 0);
	internalSendMessageTo(messageData);
}

// ----------------------------------------------------------------------

/**
 * Send a message from an object to another, to be handled by a Java
 * script.
 */
void MessageToQueue::sendMessageToJava (const NetworkId &objectId, const std::string &method, const std::vector<int8> &packedData, int delay, bool guaranteed, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod)
{
	WARNING_DEBUG_FATAL(delay < 0, ("Sent messageTo with negative delay.  Target %s, method %s, delay %f",objectId.getValueString().c_str(), method.c_str(), delay));
	if (delay < 0)
		delay = 0;

	MessageToPayload messageData(objectId, ObjectIdManager::getNewObjectId(), method, packedData, ServerClock::getInstance().getGameTimeSeconds() + static_cast<unsigned long>(delay), guaranteed, MessageToPayload::DT_java, undeliveredCallbackObject, undeliveredCallbackMethod, 0);
	internalSendMessageTo(messageData);
}

// ----------------------------------------------------------------------

void MessageToQueue::sendRecurringMessageToC  (const NetworkId &objectId, const std::string &method, const std::string & data, int delay)
{
	WARNING_DEBUG_FATAL(delay < 0, ("Sent messageTo with negative delay.  Target %s, method %s, delay %f",objectId.getValueString().c_str(), method.c_str(), delay));
	if (delay < 0)
		delay = 0;

	MessageToPayload messageData(objectId, ObjectIdManager::getNewObjectId(), method, data, ServerClock::getInstance().getGameTimeSeconds() + static_cast<unsigned long>(delay), false, MessageToPayload::DT_c, NetworkId::cms_invalid, ms_nullString, delay);
	internalSendMessageTo(messageData);
}

// ----------------------------------------------------------------------

void MessageToQueue::sendRecurringMessageToJava(const NetworkId &objectId, const std::string &method, const std::vector<int8> &packedData, int delay)
{
	WARNING_DEBUG_FATAL(delay < 0, ("Sent messageTo with negative delay.  Target %s, method %s, delay %f",objectId.getValueString().c_str(), method.c_str(), delay));
	if (delay < 0)
		delay = 0;

	MessageToPayload messageData(objectId, ObjectIdManager::getNewObjectId(), method, packedData, ServerClock::getInstance().getGameTimeSeconds() + static_cast<unsigned long>(delay), false, MessageToPayload::DT_java, NetworkId::cms_invalid, ms_nullString, delay);
	internalSendMessageTo(messageData);
}

// ----------------------------------------------------------------------

void MessageToQueue::cancelRecurringMessageTo(const NetworkId &objectId, const std::string &method)
{
	sendMessageToC(objectId, "CancelRecurringMessageTo", method, 0, false);	
}

// ----------------------------------------------------------------------

/**
 * Helper function to send a messsageTo.  Queues it on the target object or sends it on the network.
 */
void MessageToQueueNamespace::internalSendMessageTo (MessageToPayload & data)
{
	WARNING_DEBUG_FATAL(data.getGuaranteed() && !data.getUndeliveredCallbackMethod().empty(), ("Programmer bug:  attemented to send message %s to object %s, with both \"guaranteed\" and \"undeliveredCallbackMethod\" set.  Persisted messages can't have an undelivered callback.", data.getMethod().c_str(), data.getNetworkId().getValueString().c_str()));
	if (data.getNetworkId() == NetworkId::cms_invalid)
	{
		WARNING(true, ("MessageToQueue::sendMessageTo trying to send message %s to id 0!", data.getMethod().c_str()));
		return;
	}

	// non-persisted MessageTo can have an unlimited sized payload
	if (data.getGuaranteed())
	{
		std::vector<int8> const &packedData = data.getPackedDataVector();
		if (packedData.size() > 1000)
		{
			LOG("messageToFailure", ("Discarding persisted messageTo that exceeds 1000 bytes.  Persisted message %s sent to object %s had %i bytes of data.  Persisted message will not be delivered.", data.getMethod().c_str(), data.getNetworkId().getValueString().c_str(), packedData.size()));
			return;
		}
		WARNING(packedData.size() > 900 ,("Delivering persisted messageTo that exceeds 900 bytes (dangerously large, probably indicates a bug).  Persisted message %s sent to object %s had %i bytes of data.\n", data.getMethod().c_str(), data.getNetworkId().getValueString().c_str(), packedData.size()));
	}

	ServerObject * const object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(data.getNetworkId()));

	if (object)
		ms_frameMessages.push_back(data);
	else
		sendNetworkMessageTo(data);		
}

// ----------------------------------------------------------------------

/**
 * Send a MessageTo to the server for the target object, if known.  If
 * not known, find the server.
 */
void MessageToQueueNamespace::sendNetworkMessageTo(MessageToPayload & data)
{
	LastKnownLocationsType::const_iterator i=ms_lastKnownLocations.find(data.getNetworkId());
	if (i!=ms_lastKnownLocations.end())
	{
		data.addBounceServer(GameServer::getInstance().getProcessId());
		ServerMessageForwarding::begin(i->second);
		ServerMessageForwarding::send(MessageToMessage(data, GameServer::getInstance().getProcessId()));
		ServerMessageForwarding::end();
	}
	else
	{
		// If not looking for the object already, start looking for it.  Remember the
		// message, to be sent once the object is found
		if (GameServer::getInstance().hasConnectionsToOtherGameServers())
		{
			ObjectLocatorsType::iterator j=ms_objectLocators.find(data.getNetworkId());
			if (j==ms_objectLocators.end())
			{
				ObjectLocator * newLocator = new ObjectLocator(data.getNetworkId());
				j=ms_objectLocators.insert(std::make_pair(data.getNetworkId(), newLocator)).first;
			}
			data.addBroadcastCount();
			j->second->addMessage(data);
		}
		else
			dropMessage(data);
	}
}

// ----------------------------------------------------------------------

/**
 * Handle a MessageTo received on the network.  It is now this server's
 * responsibility to deal with the MessgeTo, even if it doesn't own the
 * target object.  So, it may end up re-sending the message on the network.
 */
void MessageToQueueNamespace::handleMessageFromNetwork(MessageToPayload & data, uint32 sourceServer)
{
	ServerObject * const object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(data.getNetworkId()));
	if (object && object->isAuthoritative())
	{
		object->deliverMessageTo(data);
	}
	else
	{
		if (!object)
		{
			// Hint to source server that this isn't a good place to try to find this object
			ServerMessageForwarding::begin(sourceServer);
			GenericValueTypeMessage<std::pair<NetworkId, uint32> > message("ObjectNotOnServerMessage", std::make_pair(data.getNetworkId(), GameServer::getInstance().getProcessId()));
			ServerMessageForwarding::send(message);
			ServerMessageForwarding::end();
		}

		// if this message is coming back around to this game server for the second time,
		// then it means there's a circular forwarding chain, so if we haven't broadcasted
		// the message yet to try to locate its recipient, do it now; otherwise, drop the
		// message, as the recipient won't be found
		if (data.hasBounceServer(GameServer::getInstance().getProcessId()))
		{
			// if we received the message the second time, it means the last
			// valid location we have for the recipient is no longer valid
			IGNORE_RETURN(ms_lastKnownLocations.erase(data.getNetworkId()));

			if (ConfigServerGame::getMaxMessageToBounces() != 0 && data.getBounceCount() >= ConfigServerGame::getMaxMessageToBounces())
			{
				WARNING(true,("Dropping message %s to object %s, because it bounced between too many servers (bounce=%d, broadcast=%d)", data.getMethod().c_str(), data.getNetworkId().getValueString().c_str(), data.getBounceCount(), data.getBroadcastCount()));
				dropMessage(data);
			}
			// allow the message to be broadcasted twice, just to make absolutely sure the object cannot be found
			else if (data.getBroadcastCount() < 2)
			{
				data.clearBounceServers();

				// Send the message to a (hopefully) better server to handle it
				sendNetworkMessageTo(data);
			}
			else
			{
				// drop the message
				WARNING(true,("Dropping message %s to object %s, because it bounced back to this server (bounce=%d, broadcast=%d)", data.getMethod().c_str(), data.getNetworkId().getValueString().c_str(), data.getBounceCount(), data.getBroadcastCount()));
				dropMessage(data);
			}
		}
		// object is not authoritative, but this is the first time we have received the message
		// so let the object handle it (by forwarding the message to the authoritative game server)
		else if (object) 
		{
			object->deliverMessageTo(data);
		}
		// first time we've seen the message and the object isn't here
		else
		{
			// Send the message to a (hopefully) better server to handle it
			sendNetworkMessageTo(data);
		}
	}
}

// ----------------------------------------------------------------------

void MessageToQueueNamespace::handleObjectOnServer(uint32 server, NetworkId const & object, bool authoritative)
{
	ObjectLocatorsType::iterator i=ms_objectLocators.find(object);
	if (i!=ms_objectLocators.end())
	{
		bool doneLocating = i->second->foundObject(server, authoritative);
		if (doneLocating)
		{
			delete i->second;
			ms_objectLocators.erase(i);
		}
	}
}

// ----------------------------------------------------------------------

void MessageToQueueNamespace::handleObjectNotOnServer(uint32 server, NetworkId const & object)
{
	IGNORE_RETURN(ms_lastKnownLocations.erase(object));
	
	ObjectLocatorsType::iterator i=ms_objectLocators.find(object);
	if (i!=ms_objectLocators.end())
	{
		bool doneLocating = i->second->objectNotOnServer(server);
		if (doneLocating)
		{
			delete i->second;
			ms_objectLocators.erase(i);
		}
	}
}

// ----------------------------------------------------------------------

void MessageToQueue::onGameServerDisconnect (uint32 serverId)
{
	for(LastKnownLocationsType::iterator i=ms_lastKnownLocations.begin(); i!=ms_lastKnownLocations.end();)
	{
		if (i->second == serverId)
		{
			LastKnownLocationsType::iterator next=i;
			++next;
			ms_lastKnownLocations.erase(i);
			i=next;
		}
		else
			++i;
	}

	for (ObjectLocatorsType::iterator j=ms_objectLocators.begin(); j!=ms_objectLocators.end();)
	{
		// If a server has crashed, then any object we are looking for isn't on it
		bool doneLocating = j->second->objectNotOnServer(serverId);
		if (doneLocating)
		{
			ObjectLocatorsType::iterator next=j;
			++next;
			delete j->second;
			ms_objectLocators.erase(j);
			j=next;
		}
		else
			++j;
	}
}

// ----------------------------------------------------------------------

std::string MessageToQueue::debugGetLastKnownLocations() 
{
	std::string result;
	for (LastKnownLocationsType::const_iterator i=ms_lastKnownLocations.begin(); i!=ms_lastKnownLocations.end(); ++i)
	{
		char temp[255];
		snprintf(temp,sizeof(temp)-1,"%s: %lu\n",i->first.getValueString().c_str(), i->second);
		temp[sizeof(temp)-1]='\0';
		result += temp;
	}
	return result;
}

// ======================================================================

ObjectLocator::ObjectLocator(NetworkId const & object) :
		m_object(object),
		m_waitingForServers(GameServer::getInstance().getAllGameServerPids().begin(),GameServer::getInstance().getAllGameServerPids().end()),
		m_messages(),
		m_possibleNonAuthServer(0)
{
	ServerMessageForwarding::beginBroadcast();
	
	GenericValueTypeMessage<std::pair<uint32, NetworkId> > const whoHasMessage(
		"WhoHasMessage",
		std::make_pair(GameServer::getInstance().getProcessId(),object));
	ServerMessageForwarding::send(whoHasMessage);

	ServerMessageForwarding::end();
}

// ----------------------------------------------------------------------

ObjectLocator::~ObjectLocator()
{
	m_messages.clear();
}

// ----------------------------------------------------------------------

void ObjectLocator::addMessage(MessageToPayload const & message)
{
	m_messages.push_back(message);
}

// ----------------------------------------------------------------------

bool ObjectLocator::foundObject(uint32 serverId, bool authoritative)
{
	if (authoritative)
	{
		MessageToQueueNamespace::ms_lastKnownLocations[m_object] = serverId;
		m_waitingForServers.clear();
		sendMessagesToServer(serverId);
		return true;
	}
	else
	{
		// Remember the non-auth server, use it if we can't find an auth server
		m_possibleNonAuthServer = serverId;
		return removeServer(serverId);
	}
}

// ----------------------------------------------------------------------

bool ObjectLocator::objectNotOnServer(uint32 serverId)
{
	return removeServer(serverId);
}
	
// ----------------------------------------------------------------------

void ObjectLocator::sendMessagesToServer(uint32 serverId)
{
	ServerMessageForwarding::begin(serverId);
	for (std::vector<MessageToPayload>::iterator i=m_messages.begin(); i!=m_messages.end(); ++i)
	{
		i->addBounceServer(GameServer::getInstance().getProcessId());
		ServerMessageForwarding::send(MessageToMessage(*i, GameServer::getInstance().getProcessId()));
	}
	ServerMessageForwarding::end();

	m_messages.clear();
}

// ----------------------------------------------------------------------

bool ObjectLocator::removeServer(uint32 serverId)
{
	IGNORE_RETURN(m_waitingForServers.erase(serverId));
	if (m_waitingForServers.empty())
	{
		// Didn't find the authoritative server
		if (m_possibleNonAuthServer != 0)
		{
			MessageToQueueNamespace::ms_lastKnownLocations[m_object] = m_possibleNonAuthServer;
			sendMessagesToServer(m_possibleNonAuthServer);
		}
		else
		{
			// didn't find object at all.  Messages will be dropped
			for (std::vector<MessageToPayload>::iterator message=m_messages.begin(); message!=m_messages.end(); ++message)
				dropMessage(*message);
			m_messages.clear();
		}
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------

/**
 * The message is about to be dropped.  Persist it if it is guaranteed,
 * send the undelivered callback message if one is specified.
 */
void MessageToQueueNamespace::dropMessage(MessageToPayload & message)
{
	if (message.getGuaranteed() && !message.getPersisted())
	{
		message.setPersisted(true);
		MessageToMessage const msg(message, GameServer::getInstance().getProcessId());
		GameServer::getInstance().sendToDatabaseServer(msg);			
	}
	
	if (message.getUndeliveredCallbackObject() != NetworkId::cms_invalid && !message.getUndeliveredCallbackMethod().empty())
	{
		MessageToPayload nackMessage(message.getUndeliveredCallbackObject(), ObjectIdManager::getNewObjectId(), message.getUndeliveredCallbackMethod(), message.getPackedDataVector(), ServerClock::getInstance().getGameTimeSeconds(), message.getGuaranteed(), message.getDeliveryType(), NetworkId::cms_invalid, std::string(), 0);
		internalSendMessageTo(nackMessage);
	}
}

// ----------------------------------------------------------------------

void MessageToQueue::addToScheduler(ServerObject const & object, unsigned long callTime)
{
	IGNORE_RETURN(ms_schedulerItems.insert(SchedulerItem(object,callTime)));
}

// ----------------------------------------------------------------------

std::string MessageToQueue::debugGetSchedulerData() 
{
	std::string result;
	unsigned int lastDeliveryTime=0;
	for (SchedulerItemsType::const_iterator i=ms_schedulerItems.begin(); i!=ms_schedulerItems.end(); ++i)
	{
		char temp[256];
		if (i->getDeliveryTime() != lastDeliveryTime)
		{
			if (lastDeliveryTime != 0)
				result += '\n';
			snprintf(temp,sizeof(temp),"%lu: ",i->getDeliveryTime());
			result += temp;
			lastDeliveryTime = i->getDeliveryTime();
		}
		ServerObject const * const object = i->getObject();
		if (object)
			result += " " + object->getNetworkId().getValueString();
		else
			result += " [invalidated]";
	}
	result += '\n';
	return result;
}

// ----------------------------------------------------------------------

int MessageToQueue::getTotalMessages()
{
	return MessageToPayload::getInstanceCount();
}

// ----------------------------------------------------------------------

int MessageToQueue::getBackloggedObjectCount()
{
	return ms_backloggedObjectCount;
}

// ----------------------------------------------------------------------

void MessageToQueue::incrementBackloggedObjectCount()
{
	++ms_backloggedObjectCount;
}

// ======================================================================

SchedulerItem::SchedulerItem (ServerObject const & object, unsigned long deliveryTime) :
		m_networkId(object),
		m_deliveryTime(deliveryTime)
{
}

// ----------------------------------------------------------------------

SchedulerItem::SchedulerItem() :
		m_networkId(CachedNetworkId::cms_cachedInvalid),
		m_deliveryTime(0)
{
	FATAL(true,("Programmer bug:  default-constructed a MessageToQueue::SchedulerItem.  These should always be created using the constructor that takes explicit values"));
}

// ----------------------------------------------------------------------

unsigned long SchedulerItem::getDeliveryTime() const
{
	return m_deliveryTime;
}

// ----------------------------------------------------------------------

ServerObject * SchedulerItem::getObject() const
{
	return safe_cast<ServerObject*>(m_networkId.getObject());
}

// ----------------------------------------------------------------------

CachedNetworkId const & SchedulerItem::getNetworkId() const
{
	return m_networkId;
}

// ----------------------------------------------------------------------

bool SchedulerItem::IsSoonerThan::operator() (SchedulerItem const & left, SchedulerItem const & right) const
{
	return (left.getDeliveryTime() < right.getDeliveryTime() ||
			(left.getDeliveryTime() == right.getDeliveryTime() && left.getNetworkId() < right.getNetworkId()));
}

// ======================================================================
