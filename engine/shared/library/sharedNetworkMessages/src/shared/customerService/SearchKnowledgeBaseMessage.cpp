// ======================================================================
//
// SearchKnowledgeBaseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/SearchKnowledgeBaseMessage.h"

// ======================================================================

SearchKnowledgeBaseMessage::SearchKnowledgeBaseMessage(
	const Unicode::String &searchString,
	const std::string &language
) :
GameNetworkMessage("SearchKnowledgeBaseMessage"),
m_searchString(searchString),
m_language(language)
{
	addVariable(m_searchString);
	addVariable(m_language);
}

// ======================================================================

SearchKnowledgeBaseMessage::SearchKnowledgeBaseMessage(Archive::ReadIterator &source) :
GameNetworkMessage("SearchKnowledgeBaseMessage"),
m_searchString(),
m_language()
{
	addVariable(m_searchString);
	addVariable(m_language);
	unpack(source);
}

// ======================================================================

SearchKnowledgeBaseMessage::~SearchKnowledgeBaseMessage()
{
}
