// ======================================================================
//
// MessageToManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/MessageToManager.h"

#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverNetworkMessages/MessageToMessage.h"
#include "sharedFoundation/ExitChain.h"
#include <vector>

// ======================================================================

MessageToManager *MessageToManager::ms_instance=nullptr;

// ======================================================================

void MessageToManager::install()
{
	DEBUG_FATAL(ms_instance,("Installed MessageToManager twice\n"));
	ms_instance = new MessageToManager;
	
	ExitChain::add(&remove, "MessageToManager::remove");
}

// ----------------------------------------------------------------------

void MessageToManager::remove()
{
	NOT_NULL(ms_instance);
	delete ms_instance;
	ms_instance = nullptr;
}

// ----------------------------------------------------------------------

MessageToManager::MessageToManager()
{
}

// ----------------------------------------------------------------------

MessageToManager::~MessageToManager()
{
	DEBUG_WARNING(!m_messagesByObject.empty(),("Shut down with messages outstanding that were not persisted."));
	for (MessagesByObjectType::iterator i=m_messagesByObject.begin(); i!=m_messagesByObject.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------

/**
 * Called when we receive a message from the game servers.  Save it in the
 * map, in case the object it refers to is loaded before the message is
 * persisted.
 */
void MessageToManager::handleMessageTo(const MessageToPayload &data)
{
	IndexKey theKey(data.getNetworkId(), data.getMessageId());
	
	MessagesByObjectType::iterator i=m_messagesByObject.find(theKey);
	if (i!=m_messagesByObject.end())
	{
		DEBUG_WARNING(true,("Received message %s twice.",data.getMessageId().getValueString().c_str()));
		delete i->second;
		i->second = nullptr;
	}
	
	m_messagesByObject[theKey]=new MessageToPayload(data);
	m_messageToObjectMap[data.getMessageId()]=data.getNetworkId();
}

// ----------------------------------------------------------------------

/**
 * Called when a message is acknowledged by the game servers.  If we were
 * tracking it, we don't need to any more.
 */
void MessageToManager::handleMessageToAck(const MessageToId &messageId)
{
	removeMessage(messageId);
}

// ----------------------------------------------------------------------

/**
 * Called when a message is persisted.  We don't need to track it any more
 * because it will be reloaded from the database.
 */
void MessageToManager::onMessagePersisted(const MessageToId &messageId)
{
	removeMessage(messageId);
}

// ----------------------------------------------------------------------

/**
 * Send any messages still in memory for this object.
 */
void MessageToManager::sendMessagesForObject(const NetworkId &objectId, GameServerConnection &conn) const
{
	for (MessagesByObjectType::const_iterator j=m_messagesByObject.lower_bound(IndexKey(objectId, NetworkId::cms_invalid));
		 (j != m_messagesByObject.end()) && (j->first.m_object == objectId);
		 ++j)
	{
		MessageToMessage const message(*(j->second), DatabaseProcess::getInstance().getProcessId());
		conn.send(message, true);
	}
}

// ----------------------------------------------------------------------

void MessageToManager::removeMessage(const MessageToId &messageId)
{
	MessageToObjectMapType::iterator i=m_messageToObjectMap.find(messageId);
	if (i!=m_messageToObjectMap.end())
	{
		MessagesByObjectType::iterator j=m_messagesByObject.find(IndexKey(i->second, messageId));
		if (j!=m_messagesByObject.end())
		{
			delete j->second;
			j->second=nullptr;
			m_messagesByObject.erase(j);
		}
		m_messageToObjectMap.erase(i);
	}
}

// ----------------------------------------------------------------------

/**
 * Returns true if we have the message in memory.  Used to prevent
 * sending the message twice if we load it from the database and still
 * have it in memory.
 */
bool MessageToManager::hasMessage(const MessageToId &messageId) const
{
	return (m_messageToObjectMap.find(messageId) != m_messageToObjectMap.end());
}

// ======================================================================
