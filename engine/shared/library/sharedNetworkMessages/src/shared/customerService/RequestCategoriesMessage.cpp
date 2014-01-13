// ======================================================================
//
// RequestCategoriesMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/RequestCategoriesMessage.h"

// ----------------------------------------------------------------------
RequestCategoriesMessage::RequestCategoriesMessage(std::string const & language)
 : GameNetworkMessage("RequestCategoriesMessage")
 , m_language(language)
{
	AutoByteStream::addVariable(m_language);
}

// ----------------------------------------------------------------------
RequestCategoriesMessage::RequestCategoriesMessage(Archive::ReadIterator & source)
 : GameNetworkMessage("RequestCategoriesMessage")
 , m_language()
{
	AutoByteStream::addVariable(m_language);
	unpack(source);
}

// ----------------------------------------------------------------------
RequestCategoriesMessage::~RequestCategoriesMessage()
{
}

// ----------------------------------------------------------------------
std::string RequestCategoriesMessage::getLanguage() const
{
	return m_language.get();
}

// ======================================================================
