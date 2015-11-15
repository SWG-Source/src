// ======================================================================
//
// GetCommentsResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetCommentsResponseMessage.h"

// ======================================================================

GetCommentsResponseMessage::GetCommentsResponseMessage(
	int32 result, 
	const std::vector<CustomerServiceComment> &comments
) :
	GameNetworkMessage("GetCommentsResponseMessage"),
	m_result(result),
	m_comments()
{
	m_comments.set(comments);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_comments);
}

// ======================================================================

GetCommentsResponseMessage::GetCommentsResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("GetCommentsResponseMessage"),
	m_result(0),
	m_comments()
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_comments);
	unpack(source);
}

// ======================================================================

GetCommentsResponseMessage::~GetCommentsResponseMessage()
{
}
