// ======================================================================
//
// AppendCommentMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AppendCommentMessage.h"

// ======================================================================

AppendCommentMessage::AppendCommentMessage(
	unsigned int ticketId, 
	const std::string &characterName, 
	const Unicode::String &comment 
) :
GameNetworkMessage("AppendCommentMessage"),
m_ticketId(ticketId),
m_characterName(characterName),
m_comment(comment),
m_stationId(0)
{
	addVariable(m_ticketId);
	addVariable(m_characterName);
	addVariable(m_comment);
	addVariable(m_stationId);
}

// ======================================================================

AppendCommentMessage::AppendCommentMessage(Archive::ReadIterator &source) :
GameNetworkMessage("AppendCommentMessage"),
m_ticketId(0),
m_characterName(),
m_comment(),
m_stationId(0)
{
	addVariable(m_ticketId);
	addVariable(m_characterName);
	addVariable(m_comment);
	addVariable(m_stationId);
	unpack(source);
}

// ======================================================================

AppendCommentMessage::~AppendCommentMessage()
{
}
