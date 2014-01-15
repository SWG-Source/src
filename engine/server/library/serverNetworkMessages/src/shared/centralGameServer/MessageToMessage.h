// ======================================================================
//
// MessageToMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageToMessage_H
#define INCLUDED_MessageToMessage_H

// ======================================================================

#include "serverNetworkMessages/MessageToPayload.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class MessageToMessage: public GameNetworkMessage
{
public:
	MessageToMessage(MessageToPayload const &data, uint32 sourceServerPid);
	MessageToMessage(Archive::ReadIterator &source);
	~MessageToMessage();

	MessageToPayload const &getData() const;
	uint32 getSourceServerPid() const;

private:
	Archive::AutoVariable<MessageToPayload> m_data;
	Archive::AutoVariable<uint32> m_sourceServerPid;

	MessageToMessage();
	MessageToMessage(MessageToMessage const &);
	MessageToMessage &operator=(MessageToMessage const &);
};

// ----------------------------------------------------------------------

inline MessageToPayload const &MessageToMessage::getData() const
{
	return m_data.get();
}

// ----------------------------------------------------------------------

inline uint32 MessageToMessage::getSourceServerPid() const
{
	return m_sourceServerPid.get();
}

// ======================================================================

#endif

