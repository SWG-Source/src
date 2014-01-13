//======================================================================
//
// SuiCreatePageMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/SuiCreatePageMessage.h"

//======================================================================

char const * const SuiCreatePageMessage::MessageType = "SuiCreatePageMessage";

//----------------------------------------------------------------------

SuiCreatePageMessage::SuiCreatePageMessage(SuiPageData const & pageData) :
GameNetworkMessage(MessageType),
m_pageData(pageData)
{
	addVariable(m_pageData);
}

//----------------------------------------------------------------------

SuiCreatePageMessage::SuiCreatePageMessage(Archive::ReadIterator & source) :
GameNetworkMessage(MessageType),
m_pageData()
{
	addVariable(m_pageData);
	AutoByteStream::unpack(source);
}

//----------------------------------------------------------------------

SuiPageData const & SuiCreatePageMessage::getPageData() const
{
	return m_pageData.get();
}

//----------------------------------------------------------------------
