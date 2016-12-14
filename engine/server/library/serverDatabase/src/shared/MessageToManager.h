// ======================================================================
//
// MessageToManager.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageToManager_H
#define INCLUDED_MessageToManager_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include <map>

class GameServerConnection;

// ======================================================================

/**
 * Unlike all the other types of data, messages can be sent to an object
 * that is offline.  This class stores messages until they are saved, so
 * that if an object is loaded while messages are pending, it will get
 * those messages.
 */
class MessageToManager
{
  public:
	typedef std::vector<MessageToPayload> MessageVector;

  public:
	static void install();
	static void remove();
	static MessageToManager &getInstance();
	
  public:
	void handleMessageTo(const MessageToPayload &data);
	void handleMessageToAck(const MessageToId &messageId);
	void onMessagePersisted(const MessageToId &messageId);
	void sendMessagesForObject(const NetworkId &objectId, GameServerConnection &conn) const;
	bool hasMessage(const MessageToId &messageId) const;

  private:
	MessageToManager();
	~MessageToManager();

  private:
	void removeMessage(const MessageToId &messageId);
		
  private:
	struct IndexKey
	{
		IndexKey(const NetworkId &object, const MessageToId &message);
		
		NetworkId m_object;
		MessageToId m_message;
		bool operator< (const IndexKey &rhs) const;
	};
	typedef std::map<IndexKey, MessageToPayload*> MessagesByObjectType;
	typedef	std::map<MessageToId, NetworkId> MessageToObjectMapType;

  private:
	MessagesByObjectType m_messagesByObject;
	MessageToObjectMapType m_messageToObjectMap;

  private:
	static MessageToManager *ms_instance;
};

// ======================================================================

inline MessageToManager & MessageToManager::getInstance()
{
	NOT_NULL(ms_instance);
	return *ms_instance;
}

// ======================================================================

inline MessageToManager::IndexKey::IndexKey(const NetworkId &object, const MessageToId &message) :
		m_object(object),
		m_message(message)
{
}

// ----------------------------------------------------------------------

inline bool MessageToManager::IndexKey::operator< (const IndexKey &rhs) const
{
	if (m_object < rhs.m_object)
		return true;
	else if ((m_object == rhs.m_object) && (m_message < rhs.m_message))
		return true;
	else
		return false;
}

// ======================================================================

#endif
