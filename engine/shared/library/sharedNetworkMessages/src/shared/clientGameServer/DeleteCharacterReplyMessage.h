// ======================================================================
//
// DeleteCharacterReplyMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeleteCharacterReplyMessage_H
#define INCLUDED_DeleteCharacterReplyMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class DeleteCharacterReplyMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	enum ResultCode {rc_OK, rc_ALREADY_IN_PROGRESS, rc_CLUSTER_DOWN};
	
public:
	DeleteCharacterReplyMessage(ResultCode resultCode);
	explicit DeleteCharacterReplyMessage(Archive::ReadIterator & source);
	virtual ~DeleteCharacterReplyMessage();
	
public:
	ResultCode getResultCode() const;
	
private:
	Archive::AutoVariable<int> m_resultCode;
	
	DeleteCharacterReplyMessage();
	DeleteCharacterReplyMessage(const DeleteCharacterReplyMessage&);
	DeleteCharacterReplyMessage& operator= (const DeleteCharacterReplyMessage&);
};

// ----------------------------------------------------------------------

inline DeleteCharacterReplyMessage::ResultCode DeleteCharacterReplyMessage::getResultCode() const
{
	return static_cast<ResultCode>(m_resultCode.get());
}

// ======================================================================

#endif	// _INCLUDED_ClientCentralMessages_H
