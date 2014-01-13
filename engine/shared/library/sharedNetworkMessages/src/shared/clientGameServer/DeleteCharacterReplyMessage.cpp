// ======================================================================
//
// DeleteCharacterReplyMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/DeleteCharacterReplyMessage.h"

// ======================================================================

const char * const DeleteCharacterReplyMessage::MessageType = "DeleteCharacterReplyMessage";

//----------------------------------------------------------------------

DeleteCharacterReplyMessage::DeleteCharacterReplyMessage(ResultCode resultCode) :
GameNetworkMessage(MessageType),
m_resultCode(static_cast<int>(resultCode))
{
	addVariable(m_resultCode);
}

//-----------------------------------------------------------------------

DeleteCharacterReplyMessage::DeleteCharacterReplyMessage(Archive::ReadIterator & source) :
GameNetworkMessage(MessageType),
m_resultCode()
{
	addVariable(m_resultCode);
	unpack(source);
}

// ----------------------------------------------------------------------

DeleteCharacterReplyMessage::~DeleteCharacterReplyMessage()
{
}

// ======================================================================
