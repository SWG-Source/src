// ======================================================================
//
// SearchKnowledgeBaseResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/SearchKnowledgeBaseResponseMessage.h"

// ======================================================================

SearchKnowledgeBaseResponseMessage::SearchKnowledgeBaseResponseMessage(
	int32 result, 
	const std::vector<CustomerServiceSearchResult> &searchResults
) :
	GameNetworkMessage("SearchKnowledgeBaseResponseMessage"),
	m_result(result),
	m_searchResults()
{
	m_searchResults.set(searchResults);
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_searchResults);
}

// ======================================================================

SearchKnowledgeBaseResponseMessage::SearchKnowledgeBaseResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("SearchKnowledgeBaseResponseMessage"),
	m_result(0),
	m_searchResults()
{
	AutoByteStream::addVariable(m_result);
	AutoByteStream::addVariable(m_searchResults);
	unpack(source);
}

// ======================================================================

SearchKnowledgeBaseResponseMessage::~SearchKnowledgeBaseResponseMessage()
{
}
