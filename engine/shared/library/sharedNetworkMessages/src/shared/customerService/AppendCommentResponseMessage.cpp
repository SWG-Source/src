// ======================================================================
//
// AppendCommentResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AppendCommentResponseMessage.h"

// ======================================================================

AppendCommentResponseMessage::AppendCommentResponseMessage(int32 result, unsigned int ticketId) :
	GameNetworkMessage("AppendCommentResponseMessage"),
	m_result(result),
	m_ticketId(ticketId)
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_ticketId);
}

// ======================================================================

AppendCommentResponseMessage::AppendCommentResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("AppendCommentResponseMessage"),
	m_result(0),
	m_ticketId(0)
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_ticketId);
	unpack(source);
}

// ======================================================================

AppendCommentResponseMessage::~AppendCommentResponseMessage()
{
}
