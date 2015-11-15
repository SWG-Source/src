// ======================================================================
//
// MessageBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageBuffer_H
#define INCLUDED_MessageBuffer_H

// ======================================================================

#include "serverDatabase/AbstractTableBuffer.h"
#include "serverNetworkMessages/MessageToPayload.h"

#include <map>
#include <vector>
#include <set>

// ======================================================================

namespace DB
{
	class Session;
}

class GameServerConnection;

// ======================================================================

/**
 * A buffer to hold messageTo commands.
 */
class MessageBuffer : public AbstractTableBuffer
{
  public:
	void handleMessageTo       (const MessageToPayload &message);
	void handleMessageToAck    (const MessageToId &messageId);
	void sendMessages          (GameServerConnection &conn) const;
	void removeSavedMessagesFromMemory () const;

	virtual bool save(DB::Session *session);
	virtual bool load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual void removeObject(const NetworkId &object);
	
  public:
	virtual ~MessageBuffer();
		
  private:
	typedef std::map<MessageToId,MessageToPayload> MessageMap;
	typedef std::vector<MessageToId> AckedMessagesType;
	typedef std::set<NetworkId> OIDList;
	
  private:
	MessageMap m_data;
	AckedMessagesType m_ackedMessages;
	OIDList m_cancelledObjects;
};

// ======================================================================

#endif
