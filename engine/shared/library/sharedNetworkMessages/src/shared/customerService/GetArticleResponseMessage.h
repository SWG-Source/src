// ======================================================================
//
// GetArticleResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetArticleResponseMessage_H
#define	_GetArticleResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class GetArticleResponseMessage : public GameNetworkMessage
{
public:
	GetArticleResponseMessage(int32 result, const Unicode::String &article);
	explicit GetArticleResponseMessage(Archive::ReadIterator &source);

	~GetArticleResponseMessage();

public: // methods

	int32                            getResult() const;
	const Unicode::String &          getArticle() const;

public: // types

private: 
	Archive::AutoVariable<int32>           m_result;  
	Archive::AutoVariable<Unicode::String> m_article;  
};

// ----------------------------------------------------------------------

inline int32 GetArticleResponseMessage::getResult() const
{
	return m_result.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & GetArticleResponseMessage::getArticle() const
{
	return m_article.get();
}

// ----------------------------------------------------------------------

#endif // _GetArticleResponseMessage_H

