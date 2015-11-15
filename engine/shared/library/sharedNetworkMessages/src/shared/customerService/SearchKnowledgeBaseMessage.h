// ======================================================================
//
// SearchKnowledgeBaseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_SearchKnowledgeBaseMessage_H
#define	_SearchKnowledgeBaseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

#include <string>

//-----------------------------------------------------------------------

class SearchKnowledgeBaseMessage : public GameNetworkMessage
{
public:

	SearchKnowledgeBaseMessage(const Unicode::String &searchString, const std::string &language);
	explicit SearchKnowledgeBaseMessage(Archive::ReadIterator &source);

	~SearchKnowledgeBaseMessage();

public: 

	const Unicode::String &                 getSearchString() const;
	const std::string     &                 getLanguage()     const;

private: 
	Archive::AutoVariable<Unicode::String>    m_searchString;  
	Archive::AutoVariable<std::string>        m_language;  
};

// ----------------------------------------------------------------------

inline const Unicode::String & SearchKnowledgeBaseMessage::getSearchString() const
{
	return m_searchString.get();
}

// ----------------------------------------------------------------------

inline const std::string & SearchKnowledgeBaseMessage::getLanguage() const
{
	return m_language.get();
}

// ----------------------------------------------------------------------

#endif // _SearchKnowledgeBaseMessage_H

