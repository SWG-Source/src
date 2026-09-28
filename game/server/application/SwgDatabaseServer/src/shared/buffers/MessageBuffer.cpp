// ======================================================================
//
// MessageBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/MessageBuffer.h"

#include "SwgDatabaseServer/MessageQuery.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/Loader.h"
#include "serverDatabase/MessageToManager.h"
#include "serverNetworkMessages/MessageToMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

// ======================================================================

MessageBuffer::~MessageBuffer()
{
}

// ----------------------------------------------------------------------

void MessageBuffer::handleMessageTo(const MessageToPayload &message)
{
	DEBUG_FATAL(m_data.find(message.getMessageId())!=m_data.end(),("Received duplicate messageTo, id %s.\n",message.getMessageId().getValueString().c_str()));

	if (message.getNetworkId().getValue() <= static_cast<NetworkId::NetworkIdType>(0))
		DEBUG_WARNING(true,("Ignored persisted messageTo sent to invalid object id %s, method %s",message.getNetworkId().getValueString().c_str(), message.getMethod().c_str()));
	else	
		m_data[message.getMessageId()]=message;
}

// ----------------------------------------------------------------------

void MessageBuffer::handleMessageToAck(const MessageToId &messageId)
{
	MessageMap::iterator i=m_data.find(messageId);
	if (i!=m_data.end())
	{
		// we have the message in the buffer, so we can just delete it without involving the database
		m_data.erase(i);
	}
	else
	{
		// we don't have the message, so it must have already been persisted to the database
		// set a flag to remove it from the DB
		m_ackedMessages.push_back(messageId);
	}
}

// ----------------------------------------------------------------------

bool MessageBuffer::save(DB::Session *session)
{
	int actualSaves=0;
	DBQuery::SaveMessageQuery qry;
	if (!qry.setupData(session))
		return false;

	for (MessageMap::const_iterator i=m_data.begin(); i!=m_data.end(); ++i)
	{
		if (m_cancelledObjects.find(i->second.getNetworkId())==m_cancelledObjects.end())
		{
			++actualSaves;
			if (!qry.addData((*i).second))
				return false;

			if (qry.getNumItems() == ConfigServerDatabase::getDefaultMessageBulkBindSize())
			{	
				if (! (session->exec(&qry)))
					return false;
				qry.clearData();
			}
		}
	}
	if (qry.getNumItems() != 0)
		if (! (session->exec(&qry)))
			return false;

	qry.done();
	qry.freeData();

	DBQuery::AckMessageQuery ackqry;
	if (!ackqry.setupData(session))
		return false;

	for (AckedMessagesType::const_iterator j=m_ackedMessages.begin(); j!=m_ackedMessages.end(); ++j)
	{
		if (!ackqry.addData(*j))
			return false;
		
		if (ackqry.getNumItems() == ConfigServerDatabase::getDefaultMessageBulkBindSize())
		{	
			if (! (session->exec(&ackqry)))
				return false;
			ackqry.clearData();
		}
	}
	if (ackqry.getNumItems() != 0)
		if (! (session->exec(&ackqry)))
			return false;;

	ackqry.done();
	ackqry.freeData();


	LOG("SaveCounts",("Messages:  %i out of %zu saved in db, %zu acks sent to db",actualSaves,m_data.size(),m_ackedMessages.size()));
	return true;
}

// ----------------------------------------------------------------------

bool MessageBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool)
{
	int rowsFetched;
	UNREF(tags); // any object can have messages

	DBQuery::LoadMessagesQuery qry(schema);

	if (! (session->exec(&qry)))
		return false;
	while ((rowsFetched = qry.fetch()) > 0)
	{
		size_t numRows = qry.getNumRowsFetched();
		size_t count = 0;

		const std::vector<DBQuery::LoadMessagesQuery::MessageRow> &data =qry.getData();
		
		for (std::vector<DBQuery::LoadMessagesQuery::MessageRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
		{
			if (++count > numRows)
				break;
		
			// call_time is the message's uint32 call time, stored in its signed
			// 32-bit form. A NULL call time is absent, not bad: it loads as a
			// new message's default, 0. On the 32-bit server a NULL read left
			// the buffer unchanged (a stale value or a sentinel); this default
			// is a new, deliberate compatibility policy.
			uint32_t callTime = 0;
			if (i->call_time.isNull())
				WARNING(true, ("DatabaseWarning: MessageBuffer::load: message %s has a NULL messages.call_time; loading the new-message default 0",
					i->message_id.getValue().getValueString().c_str()));
			else
				callTime = i->call_time.getValue();

			std::string utf8data;
			i->packed_data.getValue(utf8data);
			std::vector<int8> packedDataVector;
			qry.convertUTF8ToMessage(utf8data, packedDataVector);

			MessageToPayload messageData(i->object_id.getValue(),
															  i->message_id.getValue(),
															  Unicode::wideToNarrow(i->method.getValue()),
															  packedDataVector,
															  callTime,
															  i->guaranteed.getValue(),
															  static_cast<MessageToPayload::DeliveryType>(i->delivery_type.getValue()),
															  NetworkId::cms_invalid,
															  std::string(),
															  0);
			messageData.setPersisted(true);
			m_data[i->message_id.getValue()]=messageData;
		}
	}
	qry.done();
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

/**
 * Send all the messages in the buffer to the GameServer, which will enqueue
 * them and handle them when their timers expire.
 * Note:  This requires that all the objects loaded have already been
 * sent to the game server.
 */
void MessageBuffer::sendMessages(GameServerConnection &conn) const
{
	for (MessageMap::const_iterator i=m_data.begin(); i!=m_data.end(); ++i)
	{
		if (!MessageToManager::getInstance().hasMessage(i->second.getMessageId()))
		{
			MessageToMessage const msg((*i).second, DatabaseProcess::getInstance().getProcessId());
			conn.send(msg, true);
		}
	}
}

// ----------------------------------------------------------------------

void MessageBuffer::removeObject(const NetworkId &object)
{
	// Too slow to hunt down all the messages & acks for this object, so easier to make
	// a set of objects and skip any new messages for these objects
	m_cancelledObjects.insert(object);
}

// ----------------------------------------------------------------------

/**
 * Called by the main thread after persistence is complete.
 * Remove all the saved messages from the MessageToManager, since
 * they will be reloaded from the DB if needed.
 */
void MessageBuffer::removeSavedMessagesFromMemory () const
{
	for (MessageMap::const_iterator i=m_data.begin(); i!=m_data.end(); ++i)
		MessageToManager::getInstance().onMessagePersisted(i->second.getMessageId());
}

// ======================================================================
