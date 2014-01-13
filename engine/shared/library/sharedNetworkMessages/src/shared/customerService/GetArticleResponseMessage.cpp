// ======================================================================
//
// GetArticleResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetArticleResponseMessage.h"

// ======================================================================

GetArticleResponseMessage::GetArticleResponseMessage(
	int32 result, 
	const Unicode::String &article
) :
	GameNetworkMessage("GetArticleResponseMessage"),
	m_result(result),
	m_article(article)
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_article);
}

// ======================================================================

GetArticleResponseMessage::GetArticleResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("GetArticleResponseMessage"),
	m_result(0),
	m_article()
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_article);
	unpack(source);
}

// ======================================================================

GetArticleResponseMessage::~GetArticleResponseMessage()
{
}
