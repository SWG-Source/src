// ======================================================================
//
// MessageToAckMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageToAckMessage_H
#define INCLUDED_MessageToAckMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

/**
 * Case 1:
 * Sent from:  GameServer
 * Sent to:  Central
 * Action:  Acknowldege that a MessageTo has been handled.
 *
 * Case 2:
 * Sent from:  Central
 * Sent to:  DBProcess
 * Action:  Remove a MessageTo from the database.
 */
class MessageToAckMessage : public GameNetworkMessage
{
  public:
	MessageToAckMessage  (const NetworkId &messageId);
	MessageToAckMessage  (Archive::ReadIterator & source);
	~MessageToAckMessage ();

	const NetworkId & getMessageId() const;

  private:
	Archive::AutoVariable<NetworkId> m_messageId;

	MessageToAckMessage();
	MessageToAckMessage(const MessageToAckMessage&);
	MessageToAckMessage& operator= (const MessageToAckMessage&);
};

// ----------------------------------------------------------------------

inline const NetworkId & MessageToAckMessage::getMessageId() const
{
	return m_messageId.get();
}

// ======================================================================

#endif
