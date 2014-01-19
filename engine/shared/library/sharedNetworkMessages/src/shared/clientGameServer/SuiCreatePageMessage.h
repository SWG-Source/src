//======================================================================
//
// SuiCreatePageMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiCreatePageMessage_H
#define INCLUDED_SuiCreatePageMessage_H

//======================================================================

#include "Archive/AutoDeltaVector.h"
#include "sharedGame/SuiPageData.h"
#include "sharedGame/SuiPageDataArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//----------------------------------------------------------------------

class SuiCreatePageMessage : public GameNetworkMessage
{
public:

	static char const * const MessageType;

public:

	explicit SuiCreatePageMessage(SuiPageData const & pageData);
	explicit SuiCreatePageMessage(Archive::ReadIterator & source);

	SuiPageData const & getPageData() const;

private:

	Archive::AutoDeltaVariable<SuiPageData> m_pageData;
};

//======================================================================

#endif
