//======================================================================
//
// SuiUpdatePageMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiUpdatePageMessage_H
#define INCLUDED_SuiUpdatePageMessage_H

//======================================================================

#include "sharedGame/SuiPageData.h"
#include "sharedGame/SuiPageDataArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//----------------------------------------------------------------------

class SuiUpdatePageMessage : public GameNetworkMessage
{
public:

	static char const * const MessageType;

public:

	explicit SuiUpdatePageMessage(SuiPageData const & pageData);
	explicit SuiUpdatePageMessage(Archive::ReadIterator & source);

	SuiPageData const & getPageData() const;

private:

	Archive::AutoDeltaVariable<SuiPageData> m_pageData;
};

//======================================================================

#endif
