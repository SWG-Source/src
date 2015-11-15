//======================================================================
//
// SuiUpdatePageMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/SuiUpdatePageMessage.h"

//======================================================================

char const * const SuiUpdatePageMessage::MessageType = "SuiUpdatePageMessage";

//----------------------------------------------------------------------

SuiUpdatePageMessage::SuiUpdatePageMessage(SuiPageData const & pageData) :
GameNetworkMessage(MessageType),
m_pageData(pageData)
{
	addVariable(m_pageData);
}

//----------------------------------------------------------------------

SuiUpdatePageMessage::SuiUpdatePageMessage(Archive::ReadIterator & source) :
GameNetworkMessage(MessageType),
m_pageData()
{
	addVariable(m_pageData);
	AutoByteStream::unpack(source);
}

//----------------------------------------------------------------------

SuiPageData const & SuiUpdatePageMessage::getPageData() const
{
	return m_pageData.get();
}

//----------------------------------------------------------------------
