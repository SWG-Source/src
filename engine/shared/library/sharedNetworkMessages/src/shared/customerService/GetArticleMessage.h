// ======================================================================
//
// GetArticleMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetArticleMessage_H
#define	_GetArticleMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

#include <string>

//-----------------------------------------------------------------------

class GetArticleMessage : public GameNetworkMessage
{
public:

	GetArticleMessage(const std::string &id, const std::string &language);
	explicit GetArticleMessage(Archive::ReadIterator &source);

	~GetArticleMessage();

public: 

	const std::string     &                 getId()           const;
	const std::string     &                 getLanguage()     const;

private: 
	Archive::AutoVariable<std::string>        m_id;  
	Archive::AutoVariable<std::string>        m_language;  
};

// ----------------------------------------------------------------------

inline const std::string & GetArticleMessage::getId() const
{
	return m_id.get();
}

// ----------------------------------------------------------------------

inline const std::string & GetArticleMessage::getLanguage() const
{
	return m_language.get();
}

// ----------------------------------------------------------------------

#endif // _GetArticleMessage_H

