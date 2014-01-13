// ======================================================================
//
// SearchKnowledgeBaseResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_SearchKnowledgeBaseResponseMessage_H
#define	_SearchKnowledgeBaseResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/CustomerServiceSearchResult.h"
#include "sharedNetworkMessages/CustomerServiceSearchResultArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class SearchKnowledgeBaseResponseMessage : public GameNetworkMessage
{
public:
	SearchKnowledgeBaseResponseMessage(int32 result, const std::vector<CustomerServiceSearchResult> &searchResults );
	explicit SearchKnowledgeBaseResponseMessage(Archive::ReadIterator &source);

	~SearchKnowledgeBaseResponseMessage();

public: // methods

	int32                                 getResult()           const;
	const std::vector<CustomerServiceSearchResult> &   getSearchResults()         const;

public: // types

private: 
	Archive::AutoVariable<int32>          m_result;  
	Archive::AutoArray<CustomerServiceSearchResult>    m_searchResults;   
};

// ----------------------------------------------------------------------

inline const std::vector<CustomerServiceSearchResult> & SearchKnowledgeBaseResponseMessage::getSearchResults() const
{
	return m_searchResults.get();
}

// ----------------------------------------------------------------------

inline int32 SearchKnowledgeBaseResponseMessage::getResult() const
{
	return m_result.get();
}

// ----------------------------------------------------------------------

#endif // _SearchKnowledgeBaseResponseMessage_H

