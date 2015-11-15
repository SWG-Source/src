// ======================================================================
//
// GetArticleMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetArticleMessage.h"

// ======================================================================

GetArticleMessage::GetArticleMessage(
	const std::string &id,
	const std::string &language
) :
GameNetworkMessage("GetArticleMessage"),
m_id(id),
m_language(language)
{
	addVariable(m_id);
	addVariable(m_language);
}

// ======================================================================

GetArticleMessage::GetArticleMessage(Archive::ReadIterator &source) :
GameNetworkMessage("GetArticleMessage"),
m_id(),
m_language()
{
	addVariable(m_id);
	addVariable(m_language);
	unpack(source);
}

// ======================================================================

GetArticleMessage::~GetArticleMessage()
{
}
